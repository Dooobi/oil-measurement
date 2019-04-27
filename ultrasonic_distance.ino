#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <SD.h>

#define ESP8266_BAUD_RATE 115200

// WIFI config
#define WIFI_SSID "KabelBox-2230"
#define WIFI_PW "schlauebiene"
#define GATEWAY 192,168,0,1
#define SUBNET 255,255,255,0

// Time config
// Number of seconds between re-sync
// 604800 seconds = 1 week
//#define TIME_SYNC_INTERVAL 604800
#define TIME_SYNC_INTERVAL 604800

// IFTTT config
#define IFTTT_ID "dRRxfSKktUHiO1Erkm59a_"
#define IFTTT_EVENT "oil"

// App server config
// URL to call for adding a measurement (use {timestamp}, {distance} and {volume} as variables
#define ADD_DATA_URL "http://192.168.0.11:3000/addData?timestamp={timestamp}&distance={distance}&volume={volume}"

// Measurements
#define NUMBER_OF_MEASUREMENTS_TO_AVERAGE 4

// Distances of oil surface to ultrasonic sensor
#define MIN_DISTANCE 10
#define MAX_DISTANCE 180

// Volumes of oil at distances
#define VOLUME_AT_MIN_DISTANCE 10000
#define VOLUME_AT_MAX_DISTANCE 0

// GPIOs for ultrasonic sensor
#define TRIGGER 2
#define ECHO 0

// CS pin for SD card
#define CHIP_SELECT D4
#define MAX_FILE_SIZE_SETUP 150

IPAddress gateway(GATEWAY);
IPAddress subnet(SUBNET);

int timeSyncCounter = 0;
bool timeSyncSuccessful = false;

time_t prevTime = 0;

void setup() {
  Serial.begin(ESP8266_BAUD_RATE);

  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);

  setSyncProvider(timeProvider);
  setSyncInterval(TIME_SYNC_INTERVAL);

  WiFi.disconnect();
}

void loop() {
  int duration = 0;
  int distance = 0;
  int volume = 0;
  char timestamp[20];

  delay(500);
  Serial.println();

  if (timeStatus() == timeNotSet) {
    Serial.println("Waiting for time sync");
    delay(3600000); // 1 hour
    return;
  }

  int currTime = now();
  if (prevTime != 0) {
    int prevMin = day(prevTime);
    int currMin = day(currTime);

    if (prevMin != currMin) {
      distance = makeAverageMeasurement(NUMBER_OF_MEASUREMENTS_TO_AVERAGE, 500);
  
      if (distance >= 500 || distance <= 0) {
        Serial.println("Kein Messwert");
        return;
      }
      
      volume = map(distance, MIN_DISTANCE, MAX_DISTANCE, VOLUME_AT_MIN_DISTANCE, VOLUME_AT_MAX_DISTANCE);
      getTimestamp(timestamp);
    
      Serial.print(timestamp);
      Serial.print(" - ");
      Serial.print(distance);
      Serial.print(" cm");
      Serial.print(" - ");
      Serial.print(volume);
      Serial.print(" l");
      Serial.println();
    
      writeToSdCard(timestamp, distance, volume);
    
      connectWifi();
      sendIftttEvent(timestamp, distance, volume);
      sendDataToAppServer(timestamp, distance, volume);
      
      WiFi.disconnect();
    }
  }

  prevTime = now();
  
  delay(10000);
}

int makeAverageMeasurement(int numberOfMeasurements, int delayBetweenMeasurements) {
  Serial.print("Making measurements (averaging over ");
  Serial.print(numberOfMeasurements);
  Serial.println(" measurements)");

  int validMeasurements = 0;
  int sumDistances = 0;
  for (int i = 0; i < numberOfMeasurements; i++) {
    int distance = makeSingleMeasurement();
    Serial.print(" ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(distance);
    Serial.print(" cm");
    if (distance >= 500 || distance <= 0) {
      // Invalid measurement
      Serial.print(" (ignored)");
    } else {
      // Valid measurement
      validMeasurements++;
      sumDistances += distance;
    }
    Serial.println();
    delay(delayBetweenMeasurements);
  }

  Serial.print("Average: ");
  Serial.print(sumDistances / validMeasurements);
  Serial.println(" cm");
  return sumDistances / validMeasurements;
}

int makeSingleMeasurement() {
  int duration, distance;

  digitalWrite(TRIGGER, LOW);
  delay(5);
  digitalWrite(TRIGGER, HIGH);
  delay(10);
  digitalWrite(TRIGGER, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration / 2) / 29.1;

  return distance;
}

void writeToSdCard(char* timestamp, int distance, int volume) {
  if (!SD.begin(CHIP_SELECT)) {
    Serial.println("Can't write to SD card (not initialized).");
    return;
  }
  
  File dataFile = SD.open("data.csv", FILE_WRITE);
  if (!dataFile) {
    Serial.println("Can't write to SD card (file not opened).");
  }

  Serial.print("Writing to SD card...");
  dataFile.print(timestamp);
  dataFile.print(";");
  dataFile.print(distance);
  dataFile.print(";");
  dataFile.println(volume);

  dataFile.close();
  Serial.println("done!");
}

void connectWifi() {
  int i = 0;

  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PW);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i++ >= 20) {
      Serial.println("failed!");
      return;
    }
  }
  Serial.println("connected!");
}

void sendIftttEvent(char* timestamp, int distance, int volume) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Can't send IFTTT event (WiFi not connected).");
    return;
  }

  HTTPClient https;
  BearSSL::WiFiClientSecure client;
  client.setInsecure();

  char url[150];
  snprintf(url, 150, "https://maker.ifttt.com/trigger/%s/with/key/%s?value1=%s&value2=%d&value3=%d", IFTTT_EVENT, IFTTT_ID, timestamp, distance, volume);

  // Replace ' ' (in timestamp) with 'T'
  for (int i = 0; i < 150; i++) {
    if (url[i] == ' ') {
      url[i] = 'T';
    }
  }

  https.begin(client, url);

  int responseCode = https.GET();
  String response = https.getString();

  Serial.println("Sent ifttt event to:");
  Serial.print(" ");
  Serial.println(url);
  Serial.println("Response:");
  Serial.print(" ");
  Serial.println(response);

  https.end();

  if (responseCode < 0) {
    Serial.print("Failed to send IFTTT event (return code: ");
    Serial.print(responseCode);
    Serial.println(").");
  }
}

void sendDataToAppServer(char* timestamp, int distance, int volume) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Can't send data to server (WiFi not connected).");
  }

  String url = String(ADD_DATA_URL);

  url.replace("{timestamp}", String(timestamp));
  url.replace("{distance}", String(distance));
  url.replace("{volume}", String(volume));
  url.replace(" ", "%20");

  HTTPClient http;
  http.begin(url);

  Serial.println("Sending data to:");
  Serial.println(url);
  
  int httpCode = http.GET();
  String response = http.getString();

  Serial.println(response);
}

time_t timeProvider() {
  time_t theTime = readTime();

  timeSyncCounter++;
  timeSyncSuccessful = (theTime != 0);

  Serial.println("timeProvider");
  Serial.print(" timeSyncCounter: ");
  Serial.println(timeSyncCounter);
  Serial.print(" theTime: ");
  Serial.println(theTime);

  return theTime;
}

time_t readTime() {
  time_t timeSd = 0;
  time_t timeInternet = 0;

  if (timeSyncCounter == 0) {
    // Initial time sync
    Serial.println("timeProvider: timeNotSet");

    // Load sd time first to minimize time difference
    // (because loading internet time can take a while)
    timeSd = readTimeSdCard();
    timeInternet = readTimeInternet();

    Serial.print(" timeSd: ");
    Serial.println(timeSd);
    Serial.print(" timeInternet: ");
    Serial.println(timeInternet);

    // Try to use internet time
    // otherwise use sd time.
    if (timeInternet == 0) {
      return timeSd;
    }
    return timeInternet;
  } else {
    // Resync
    Serial.println("timeProvider: resync");

    // Try to use internet time
    // otherwise use current time.
    // (Time is not changed if 0 is returned)
    return readTimeInternet();
  }
}

time_t readTimeInternet() {
  connectWifi();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Can't read time for worldtimeapi.org (WiFi not connected).");
    return 0;
  }

  char url[] = "http://worldtimeapi.org/api/timezone/Europe/Berlin";

  HTTPClient http;
  http.begin(url);

  Serial.println("Receiving time data from:");
  Serial.println(url);
  
  int httpCode = http.GET();
  String response = http.getString();

  Serial.println(response);

  int i = response.indexOf("\"unixtime\":");
  int j = response.indexOf("\",\"", i);

  String unixtime = response.substring(i + 12, j);

  // Add two hours because of timezone
  return unixtime.toInt() + 7200;
}

time_t readTimeSdCard() {
  if (!SD.begin(CHIP_SELECT)) {
    Serial.println("Can't read time from SD card (not initialized).");
    return 0;
  }

  File setupFile = SD.open("unixtime.txt", FILE_READ);
  if (!setupFile) {
    Serial.println("Can't read time from SD card (unixtime.txt not opened).");
    return 0;
  }

  char fileContent[MAX_FILE_SIZE_SETUP] = {0};
  readFileContent(setupFile, fileContent, MAX_FILE_SIZE_SETUP);
  Serial.println(fileContent);
  return atol(fileContent) + 7200;
}

void readFileContent(File file, char fileContent[], int maxFileSize) {
  int i = 0;  
  while (file.available() && i < maxFileSize) {
    fileContent[i] = file.read();
    i++;
  }
}

/**
 * Creates timestamp in format:
 *  DD.MM.YYYY HH:MM:SS
 * (20 characters long)
 */
void getTimestamp(char* timestamp) {
  time_t t = now(); // store the current time in time variable t

  timestamp[0] = (day(t) / 10) + '0';
  timestamp[1] = (day(t) % 10) + '0';
  timestamp[2] = '.';
  timestamp[3] = (month(t) / 10) + '0';
  timestamp[4] = (month(t) % 10) + '0';
  timestamp[5] = '.';
  timestamp[6] = ((year(t) / 1000) % 10) + '0';
  timestamp[7] = ((year(t) / 100) % 10) + '0';
  timestamp[8] = ((year(t) / 10) % 10) + '0';
  timestamp[9] = (year(t) % 10) + '0';
  timestamp[10] = ' ';
  timestamp[11] = (hour(t) / 10) + '0';
  timestamp[12] = (hour(t) % 10) + '0';
  timestamp[13] = ':';
  timestamp[14] = (minute(t) / 10) + '0';
  timestamp[15] = (minute(t) % 10) + '0';
  timestamp[16] = ':';
  timestamp[17] = (second(t) / 10) + '0';
  timestamp[18] = (second(t) % 10) + '0';
  timestamp[19] = '\0';
}
