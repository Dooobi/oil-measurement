#include <TimeLib.h>
#include <SoftwareSerial.h>

#define TRIGGER 7
#define ECHO 6

#define ESP8266_BAUD_RATE 115200

SoftwareSerial ESPserial(2, 3); // RX, TX (arduino)

void setup() {
  Serial.begin(9600);
  ESPserial.begin(ESP8266_BAUD_RATE);

  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);

  setTime(19, 2, 0, 15, 3, 2019);
}

void loop() {
  int duration = 0;
  int distance = 0;
  int litre = 0;
  char timestamp[18];

  digitalWrite(TRIGGER, LOW);

  delay(5);
  digitalWrite(TRIGGER, HIGH);

  delay(10);
  digitalWrite(TRIGGER, LOW);

  duration = pulseIn(ECHO, HIGH);
  distance = (duration / 2) / 29.1;

  if (distance >= 500 || distance <= 0) {
    Serial.println("Kein Messwert");
  }
  else {
    litre = map(distance, 10, 160, 10000, 0);
    getTimestamp(timestamp);

    Serial.print(distance);
    Serial.println("cm");

    sendDataToEsp8266(timestamp, distance, litre);
  }

  delay(5000);
}

void sendDataToEsp8266(char timestamp[], int distance, int litre) {
  ESPserial.print(timestamp);
  ESPserial.print(";");
  ESPserial.print(distance);
  ESPserial.print(";");
  ESPserial.print(litre);
  ESPserial.print("\0");
}

void getTimestamp(char* timestamp) {
//timestamp = "00.00.00 00:00:00";

  time_t t = now(); // store the current time in time variable t

  timestamp[0] = (day(t) / 10) + '0';
  timestamp[1] = (day(t) % 10) + '0';
//timestamp[2] = ".";
  timestamp[3] = (month(t) / 10) + '0';
  timestamp[4] = (month(t) % 10) + '0';
//timestamp[5] = ".";
  timestamp[6] = ((year(t) / 10) % 10) + '0';
  timestamp[7] = (year(t) % 10) + '0';
//timestamp[8] = " ";
  timestamp[9] = (hour(t) / 10) + '0';
  timestamp[10] = (hour(t) % 10) + '0';
//timestamp[11] = ":";
  timestamp[12] = (minute(t) / 10) + '0';
  timestamp[13] = (minute(t) % 10) + '0';
//timestamp[14] = ":";
  timestamp[15] = (second(t) / 10) + '0';
  timestamp[16] = (second(t) % 10) + '0';
  timestamp[17] = '\0';
}
