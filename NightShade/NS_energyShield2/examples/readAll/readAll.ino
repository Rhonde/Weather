/**********************************************************
  ReadAll - energyShield2 by NightShade Electronics V1.0

  This sketch reads voltage, current, fullChargeCapacity,
  remaingingCapacity, SOC, inputVoltage, temperature,
  Time, and Date from an energyShield2 by
  NightShade Electronics.

  Created by Aaron D. Liebold
  on January 30, 2017

  Distributed under the MIT license
  Copyright 2017 NightShade Electronics
  https://opensource.org/licenses/MIT
**********************************************************/

// Include energyShield2 library files
#include <NS_eS2_Utilities.h>
#include <NS_energyShield2.h>

// Create NS_energyShield2 class object called "es2"
NS_energyShield2 es2;

// Define variables
uint16_t FullCapacity, RemainingCapacity, StateOfCharge;
float BatteryVoltage, BatteryCurrent, InputVoltage, Temperature;

void setup() {
  // Initialize Serial
  Serial.begin(9600);
  while (!Serial); // Wait for Leonardo

  // Initialize energyShield and print error if es2.begin does not return 0
  if (es2.begin()) {
    Serial.println("! ERROR - Initialization Failed !");
    while (1);
  }
  
  if (es2.readVMPP() != -1) es2.setVMPP(-1, 1); // Disable VMPP regulation to allow charging from any power supply (7V - 23V) and prevent excessive EEPROM writes

  // Print header
  Serial.println("Voltage, V\tCurrent, A\tFull Capacity, mAh\tRemaining Capacity, mAh\tState of Charge, %\tInput Voltage, V\tTemperature, C\tTime, HH:MM:SS\tDate, DD/MM/YY");
}

void loop() {

  // Read values from energyShield2
  BatteryVoltage = (float) es2.batteryVoltage() / 1000;
  BatteryCurrent = (float) es2.batteryCurrent() / 1000;
  FullCapacity = es2.fullChargeCapacity();
  RemainingCapacity = es2.remainingCapacity();
  StateOfCharge = es2.SOC();
  InputVoltage = (float) es2.inputVoltage() / 1000;
  Temperature = (float) es2.temperature() / 10;

  // Print Results
  Serial.print(BatteryVoltage, 3);
  Serial.print("V    \t");
  Serial.print(BatteryCurrent, 3);
  Serial.print("A   \t");
  Serial.print(FullCapacity);
  Serial.print("mAh             \t");
  Serial.print(RemainingCapacity);
  Serial.print("mAh              \t");
  Serial.print(StateOfCharge);
  Serial.print("%              \t");
  Serial.print(InputVoltage, 2);
  Serial.print("V            \t");
  Serial.print(Temperature, 1);
  Serial.print("C       \t");

  // Read time and date from energyShield and store locally
  // Local values will not update until readClock is called again
  es2.readClock();

  // Print time and date from locally stored values
  Serial.print(es2.hour());
  Serial.print(":");
  if (es2.minute() < 10) Serial.print("0");
  Serial.print(es2.minute());
  Serial.print(":");
  if (es2.second() < 10) Serial.print("0");
  Serial.print(es2.second());
  Serial.print("     \t");
  Serial.print(es2.month());
  Serial.print("/");
  Serial.print(es2.dayOfMonth());
  Serial.print("/");
  Serial.print(es2.year());

  // Print carriage return to start new line
  Serial.println();

  // Wait between reads
  delay(2000);

}

