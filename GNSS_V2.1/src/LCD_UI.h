#ifndef __LCD_UI_H
#define __LCD_UI_H

#include "Arduino.h"
#include "WiFi.h"
#include <LiquidCrystal_I2C.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

#define VOLTAGE_READ_PIN     32
#define LDC_I2C_ADDRESS      0x27
#define ADC_RESOLUTION       0x0FFF
#define MIN_BATTERY_VOLTAGE  3.0
#define MAX_BATTERY_VOLTAGE  4.2
#define MIN_RSSI             -100
#define MAX_RSSI             -20
#define BUTTON_UP            2
#define BUTTON_DOWN          4
#define BUTTON_SELECT        15
#define SHORT_BUTTON_PRESS   50 //50 ms to preent bounce
#define LONG_BUTTON_PRESS    800 //800 ms 
#define ENC_TYPE_WEP         5
#define ENC_TYPE_TKIP        2
#define ENC_TYPE_CCMP        4
#define ENC_TYPE_NONE        7
#define ENC_TYPE_AUTO        8



void initLCD_UI(void);
void drawScreen(void);
void drawBatteryLevel(uint8_t position_x, uint8_t position_y);
void drawSignalLevel(uint8_t position_x, uint8_t position_y, uint8_t level);
void drawWIFI(uint8_t position_x, uint8_t position_y);

#endif /* LCD_UI_H */