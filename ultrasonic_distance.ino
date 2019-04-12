#include <TimeLib.h>
#include <SPI.h>
#include <SD.h>

#define TRIGGER 7
#define ECHO 6

int duration = 0;
int distance = 0;
int litre = 0;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");


  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);

  setTime(19, 2, 0, 15, 3, 2019);
}

void loop() {
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

    File myFile = SD.open("test.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (myFile) {
      Serial.print("Writing to test.txt...");

      writeToFile(myFile, distance, litre);
      
      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }

    Serial.print(distance);
    Serial.println("cm");
  }

  delay(100);
}

void writeToFile(File myFile, int distance, int litre) {
  char yearShort[] = "00";

  time_t t = now(); // store the current time in time variable t

  yearShort[0] = ((year(t) / 10) % 10) + '0';
  yearShort[1] = (year(t) % 10) + '0';
  
  myFile.print(hour(t));          // returns the hour for the given time t
  myFile.print(":");
  myFile.print(minute(t));        // returns the minute for the given time t
  myFile.print(":");
  myFile.print(second(t));        // returns the second for the given time t
  myFile.print(" ");
  myFile.print(day(t));           // the day for the given time t
  weekday(t);       // day of the week for the given time t
  myFile.print(".");
  myFile.print(month(t));         // the month for the given time t
  myFile.print(".");
  myFile.print(yearShort);          // the year for the given time t

  myFile.print("\t");

  myFile.print(distance);
  myFile.print("cm");

  myFile.print("\t");

  myFile.print(litre);
  myFile.print("l");

  myFile.println();
}

/*
void printTimeString(int col, int row) {
  char yearShort[] = "00";

  time_t t = now(); // store the current time in time variable t

  yearShort[0] = ((year(t) / 10) % 10) + '0';
  yearShort[1] = (year(t) % 10) + '0';

  lcd.setCursor(col, row);

  lcd.print(hour(t));          // returns the hour for the given time t
  lcd.print(":");
  lcd.print(minute(t));        // returns the minute for the given time t
  lcd.print(":");
  lcd.print(second(t));        // returns the second for the given time t
  lcd.print(" ");
  lcd.print(day(t));           // the day for the given time t
  weekday(t);       // day of the week for the given time t
  lcd.print(".");
  lcd.print(month(t));         // the month for the given time t
  lcd.print(".");
  lcd.print(yearShort);          // the year for the given time t
}
*/
