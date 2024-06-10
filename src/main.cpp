#include <Arduino.h>

#include "network.h"
#include "ledControl.h"

void setup()
{
  ledInit(); // Initialize the LED pins

  Serial.begin(115200);
  delay(100); // Wait for the serial connection to be establised
  Serial.print("\n\ncompile time: ");
  Serial.println(__DATE__ " " __TIME__);

  wifiSetup(); // Initialize WiFi and MQTT settings
}

void loop()
{

  wifiLoop();
}
