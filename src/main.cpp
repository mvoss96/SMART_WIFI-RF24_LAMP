#include <Arduino.h>

#include "network.h"
#include "ledControl.h"
#include "radio.h"

void setup()
{
  ledInit(); // Initialize the LED pins

  Serial.begin(115200);
  delay(100); // Wait for the serial connection to be establised
  Serial.print("\n\ncompile time: ");
  Serial.println(__DATE__ " " __TIME__);
  radioInit(); // Initialize the RF radio
  wifiInit(); // Initialize WiFi and MQTT settings
}

void loop()
{
  wifiLoop();
  radioLoop();
}
