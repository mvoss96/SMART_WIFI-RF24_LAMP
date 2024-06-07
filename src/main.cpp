#include <Arduino.h>

#include "network.h"

void setup()
{
  Serial.begin(115200);
  delay(100); // Wait for the serial connection to be establised
  Serial.print("\n\ncompile time: ");
  Serial.println(__DATE__ " " __TIME__);

  wifiSetup();
}

void loop()
{

  wifiLoop();
}
