/************************************************************************************
   SolarLCD - energyShield2 by NightShade Electronics V1.0

   This sketch displays measured battery/charging values
   on an LCD display using the LiquidCrystal library and
   impliments a maximum power-point tracking algorithm to
   control the voltage regulation of a solar panel for
   charging.

   Theory:
      The concept of this project is to run an LCD display in
   a low power state. To do this, the back light is not
   powered and the power to the Arduino is turned off for 5
   seconds at a time. The LCD itself is run from the RAW pin
   on the energyShield. The RAW pin is unregulated battery
   voltage and does not turn off during sleep. The maintains
   the displayed text.
      When sun is present and the battery begins charging the
   maximum power-point algorithm adjusts the solar panel
   maximum power-point regulation voltage  (VMPP) so that
   the solar panel generates as much power as possible.
      This project has proven to run indefinately with a
   small solar panel. (8-20 volts, 5 watts)

   Wikipedia - MPPT: https://www.wikipedia.org/wiki/Maximum_power_point_tracking


   LCD Wiring guide:
   ********************************************************
   LCD <-----> Arduino

   LED- ----- N/C
   LED+ ----- N/C
   DB7  ----- D4
   DB6  ----- D5
   DB5  ----- D6
   DB4  ----- D7
   DB3  ----- N/C
   DB2  ----- N/C
   DB1  ----- N/C
   DB0  ----- N/C
   E    ----- D11
   R/W  ----- GND
   RS   ----- D12
   V0   ----- Potentiometer to set contrast
   VDD  ----- RAW (Off-header connection. Does not turn off.)
   VSS  ----- GND
   ********************************************************

  Created by Aaron D. Liebold
  on January 30, 2017

  Distributed under the MIT license
  Copyright 2017 NightShade Electronics
  https://opensource.org/licenses/MIT
************************************************************************************/

// Define solar panel maximum power point voltage (manufacturer specified (17.5V is a good guess)
#define ASSUMED_VMPP 17500
#define MAX_VMPP 22000
#define MIN_VMPP 12000

// Include energyShield 2 library files
#include <NS_energyShield2.h>
#include <NS_eS2_Utilities.h>

// include the LiquidCrystal library
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

// Initialize energyShield 2
NS_energyShield2 es2;

// Define variables
int           StateOfCharge, Temperature, BatteryCurrent, RemainingCapacity;
unsigned int  VMPP;
float         BatteryVoltage, InputVoltage;
long          lastMillis;

void setup() {
  // set up the LCD's number of rows and columns:
  lcd.begin(16, 2);

  // Initialize energyShield and print error is begin() does not return 0
  if (es2.begin()) {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Init Failed");
    while (1);
  }

  VMPP = es2.readVMPP();
  if (VMPP == -1) VMPP = ASSUMED_VMPP; // If regulation is disabled (first time) set VMPP to initial guess

  es2.batteryAlert(20); // Battery low LED to alert at 20%

  delay(10);
}

void loop() {
  if (es2.batteryCurrent() > 0) {
    unsigned long lastPower, newPower;
    int reverse = 0;

    // Calculated current power to battery // P = V * I
    lastPower = (unsigned long) es2.batteryVoltage() * es2.batteryCurrent();

    do {
      es2.setVMPP(VMPP, 0); // Set voltage regulation to new value, but do not write EEPROM (EEPROM has an expected write life of 1,000,000 cycles, don't waste it.)

      delay(2000); // Wait for system to settle and fuel gauge to measure new average current (>1 sec)

      // Reprint LCD Screen with MPPT info
      lcd.clear();
      lcd.setCursor(10, 0);
      lcd.print(lastPower / 1000);
      lcd.print("mW");

      lcd.setCursor(0, 1);
      lcd.print("Tracking");
      lcd.setCursor(10, 1);
      lcd.print((float) VMPP / 1000, 1);
      lcd.print('V');

      // Measure and calculate new power
      newPower = (unsigned long) es2.batteryVoltage() * es2.batteryCurrent();

      // If power decreased, change direction
      if (newPower < lastPower) ++reverse;

      // Perturb voltage setpoint in the current direction of travel
      if (reverse % 2) {
        VMPP += 100;
      }
      else {
        VMPP -= 100;
      }

      // Change direction if setpoint get to the edge of the acceptable voltage range (7.0V - 90% Open-circuit Voltage)
      // If VMPP is set above the open-circuit voltage of the solar panel, it will never charge
      if (VMPP > MAX_VMPP || VMPP < MIN_VMPP) ++reverse;

      // Save newPower as lastPower
      lastPower = newPower;

    } while (reverse < 4); // Keep seeking maximum power point voltage (VMPP) until you have changed directions 4 times


    es2.setVMPP(VMPP - 100, 1); // Set VMPP back to last best setting and write to EEPROM so that it remembers the setting even if the sun is lost momentarily
    delay(1000); // Allow voltage to stabilize

    lastMillis = millis();
    lcd.clear();
  }

  // Measure parameters
  BatteryVoltage = (float) es2.batteryVoltage() / 1000;
  BatteryCurrent = es2.batteryCurrent(); // Measures average current of last 1 second // Typically displays active current; not averaged with the sleep time.
  StateOfCharge = es2.SOC();
  Temperature = es2.temperature() / 10;
  RemainingCapacity = es2.remainingCapacity();

  InputVoltage = es2.inputVoltage();
  InputVoltage = (float) InputVoltage / 1000;

  lcd.clear();

  // Re-print LCD display
  lcd.setCursor(0, 0);
  lcd.print(BatteryVoltage, 2);
  lcd.print("V");

  lcd.setCursor(6, 0);
  lcd.print(StateOfCharge);
  lcd.print("%");

  lcd.setCursor(10, 0);
  lcd.print(BatteryCurrent);
  lcd.print("mA");

  lcd.setCursor(0, 1);
  lcd.print("I:");
  lcd.print(InputVoltage, 2);
  lcd.print("V");

  lcd.setCursor(9, 1);
  if (BatteryCurrent > 0) {
    lcd.print("CHG");
  }
  else {
    lcd.print("DSG");
  }

  lcd.setCursor(13, 1);
  lcd.print(Temperature);
  lcd.print("C");

  es2.sleepSeconds(30); // Sleep 30 seconds
}

