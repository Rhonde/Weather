/*************************************************************
  setTimeAndDate - energyShield2
  by NightShade Electronics V1.0

  This sketch sets the time and date in the energyShield2
  Real-time Clock module.

  Created by Aaron D. Liebold
  on January 30, 2017

  Distributed under the MIT license
  Copyright 2017 NightShade Electronics
  https://opensource.org/licenses/MIT
*************************************************************/

// Includes the energyShield 2 library files
#include <NS_energyShield2.h>
#include <NS_eS2_Utilities.h>

// Create NS_energyShield2 class object
NS_energyShield2 es2;

// Define variables
uint8_t year, month, dayOfMonth, dayOfWeek, hour, minute, second;

void setup() {
  // Initialize serial at 9600 bps
  Serial.begin(9600);
  while (!Serial); // Wait for Leonardo

  // Initialize energyShield2 and print error if begin() does not return 0
  if (es2.begin()) {
    Serial.println("enegyShield failed to initialize...");
    while (1);
  }
  
  if (es2.readVMPP != -1) es2.setVMPP(-1, 1); // Disable VMPP regulation to allow charging from any source (7V - 23V) and prevent excessive EEPROM writes
}

void loop() {

  Serial.println("This sketch sets the time and date on the enegyShield 2...\n");
  Serial.println("IMPORTANT: Use \"No Line Ending\"");

  /****************************/
  /* Collect data from user */
  /****************************/
  Serial.print("Please enter the year (YY): ");
  while (!Serial.available()); // Wait to receive serial data
  year = Serial.parseInt(); // Parse received characters into int value
  Serial.println(year); // Print received data

  Serial.print("Please enter the month (1-12): ");
  while (!Serial.available());
  month = Serial.parseInt();
  Serial.println(month);

  Serial.print("Please enter the day of the month (1-31): ");
  while (!Serial.available());
  dayOfMonth = Serial.parseInt();
  Serial.println(dayOfMonth);

  Serial.print("Please enter the day of the week [0-6 (Sun-Sat)]: ");
  while (!Serial.available());
  dayOfWeek = Serial.parseInt();
  switch (dayOfWeek) {            // Print name of day that corresponds to value entered
    case 0:
      Serial.println("Sunday ");
      break;
    case 1:
      Serial.println("Monday ");
      break;
    case 2:
      Serial.println("Tuesday ");
      break;
    case 3:
      Serial.println("Wednesday ");
      break;
    case 4:
      Serial.println("Thursday ");
      break;
    case 5:
      Serial.println("Friday ");
      break;
    case 6:
      Serial.println("Satuday ");
      break;
  }

  Serial.print("Please enter the hour [0-23]: ");
  while (!Serial.available());
  hour = Serial.parseInt();
  Serial.println(hour);

  Serial.print("Please enter the minute (0-59): ");
  while (!Serial.available());
  minute = Serial.parseInt();
  Serial.println(minute);

  Serial.print("Please enter the second (0-59): ");
  while (!Serial.available());
  second = Serial.parseInt();
  Serial.println(second);

  // Program time and date into RTC
  es2.setTimeDate(second, minute, hour, dayOfMonth, dayOfWeek, month, year);
  delay(10); // Give RTC some time to program

  /****************************************/
  /* Print current time and date from RTC */
  /****************************************/
  Serial.println("\nThe current time and date is: ");

  while (1) {

    // Read time and date from RTC into local memory (Must be done to update time and date values)
    es2.readClock();

    // Print the day of the week
    switch ((int) es2.dayOfWeek()) {
      case 0:
        Serial.print("Sunday ");
        break;
      case 1:
        Serial.print("Monday ");
        break;
      case 2:
        Serial.print("Tuesday ");
        break;
      case 3:
        Serial.print("Wednesday ");
        break;
      case 4:
        Serial.print("Thursday ");
        break;
      case 5:
        Serial.print("Friday ");
        break;
      case 6:
        Serial.print("Satuday ");
        break;
    }

    // Print time in HH:MM:SS format
    Serial.print(es2.hour());
    Serial.print(":");
    if (es2.minute() < 10) Serial.print("0");
    Serial.print(es2.minute());
    Serial.print(":");
    if (es2.second() < 10) Serial.print("0");
    Serial.print(es2.second());
    Serial.print(" ");

    // Print date in DD/MM/YY format
    Serial.print(es2.month());
    Serial.print("/");
    Serial.print(es2.dayOfMonth());
    Serial.print("/");
    Serial.print(es2.year());

    Serial.println(); // Print new line

    delay(5000);

  }
}
