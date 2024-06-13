#pragma once

void radioInit();
bool radioIsInitialized();
void radioLoop();
char* getRadioAddressString();
uint8_t getRadioChannel(); 
void setRadioSettings(uint8_t channel, const char *radioAddress);