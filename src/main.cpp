#include "wifiFunctions.h"
#include "ledControl.h"
#include "radio.h"
#include "chipID.h"

#include <Arduino.h>

// void printMemoryInfo(){
//   uint32_t flashSize = ESP.getFlashChipSize();
//   Serial.printf("Flash Size: %u bytes\n", flashSize);

//   uint32_t psramSize = ESP.getPsramSize();
//   Serial.printf("PSRAM Size: %u bytes\n", psramSize);

//   uint32_t freeMemory = ESP.getFreeHeap();
//   Serial.printf("Free Memory: %u bytes\n", freeMemory);

//   uint32_t Totalheap = ESP.getHeapSize();
//   Serial.printf("Total heap: %u bytes\n", Totalheap);

//   uint32_t FreePSRAM = ESP.getFreePsram();
//   Serial.printf("Free PSRAM: %u bytes\n", FreePSRAM);
// }

void printSpiPins()
{
  Serial.printf("MISO: %d\n", MISO);
  Serial.printf("MOSI: %d\n", MOSI);
  Serial.printf("SCK: %d\n", SCK);
}

// radio task
void radioTask(void *pvParameters)
{
  radioInit(); // Initialize the RF radio
  for (;;)
  {
    radioLoop();
    ledUpdate();
    vTaskDelay(10); // Delay to allow other tasks to run
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100); // Wait for the serial connection to be established
  Serial.print("\n\ncompile time: ");
  Serial.println(__DATE__ " " __TIME__);
  Serial.println(ChipID::getChipID());
  ledInit();                                                // Initialize the LED pins
  xTaskCreate(radioTask, "radioTask", 4096, NULL, 1, NULL); // Create the radio task
  wifiInit();                                               // Initialize WiFi and MQTT settings
}

void loop()
{
  wifiLoop();
}
