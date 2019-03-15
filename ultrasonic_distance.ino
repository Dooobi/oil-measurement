#include <LiquidCrystal.h>
#include <TimeLib.h>

#define TRIGGER 7
#define ECHO 6

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int duration = 0;
int distance = 0;
int litre = 0;

void setup() {
  lcd.begin(16, 2);
  
  Serial.begin(9600);

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
    lcd.setCursor(0, 0);

    lcd.print(distance);
    lcd.print("cm");

    printTimeString(0, 1);

    litre = map(distance, 10, 160, 10000, 0);

    lcd.setCursor(10, 0);
    
    lcd.print(litre);
    lcd.print("l");
    
    Serial.print(distance);
    Serial.println("cm");
  }

  delay(100);
}

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
