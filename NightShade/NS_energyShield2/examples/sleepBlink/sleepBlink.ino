/*************************************************************
  sleepBlink - energyShield2                              
  by NightShade Electronics V1.0                          
                                                         
  This sketch blinks the pin 13 LED three times and       
  sleeps for 4 seconds. These results is an approximate   
  20% duty cycle for power comsumption. That means that   
  this blink sketck will run 5 times as long as the       
  standard blink sketch will run on the same battery.         
                                                          
  Created by Aaron D. Liebold                             
  on January 30, 2017

  Distributed under the MIT license
  Copyright 2017 NightShade Electronics
  https://opensource.org/licenses/MIT         
*************************************************************/

// Includes enegyShield2 library files
#include <NS_energyShield2.h>
#include <NS_eS2_Utilities.h>

// Defines NS_energyShield2 class object "es2"
NS_energyShield2 es2;

void setup() {
  es2.begin(); // Initialize energyShield  
  if (es2.readVMPP != -1) es2.setVMPP(-1, 1); // Disable VMPP regulation to allow charging from any source (7V - 23V) and prevent excessive EEPROM writes
  
  pinMode(13, OUTPUT);
}

void loop() {
  // Blinks three times
  blink();
  blink();
  blink();

  // Sleeps power for 4 seconds
  es2.sleepSeconds(4);
}

// Blinks LED
void blink() {
  digitalWrite(13, HIGH);
  delay(166);
  digitalWrite(13, LOW);
  delay(166);
}

