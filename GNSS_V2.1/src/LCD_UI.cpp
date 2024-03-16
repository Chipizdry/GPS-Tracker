
#include "LCD_UI.h"
#include "GPS.h"
#include "Buzzer.h"
#include "SPIFFS.h"
#include "PointsFS.h"

uint8_t battery_level_0[8]  = {0b01110,0b11011,0b10001,0b11011,0b10101,0b11011,0b10001,0b11111};
uint8_t battery_level_1[8]  = {0b01110,0b11011,0b10001,0b10001,0b10001,0b10001,0b11111,0b11111};
uint8_t battery_level_2[8]  = {0b01110,0b11011,0b10001,0b10001,0b10001,0b11111,0b11111,0b11111};
uint8_t battery_level_3[8]  = {0b01110,0b11011,0b10001,0b10001,0b11111,0b11111,0b11111,0b11111};
uint8_t battery_level_4[8]  = {0b01110,0b11011,0b10001,0b11111,0b11111,0b11111,0b11111,0b11111};
uint8_t battery_level_5[8]  = {0b01110,0b11011,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111};
uint8_t battery_level_6[8]  = {0b01110,0b11111,0b10101,0b10001,0b11011,0b11011,0b11111,0b11111};

uint8_t wifi_signal_0[8]    = {0b01010,0b00100,0b01010,0b00000,0b00010,0b11001,0b00101,0b10101};
uint8_t wifi_signal_1[8]    = {0b00000,0b00000,0b00000,0b10000,0b10000,0b00000,0b00000,0b00000};
uint8_t wifi_signal_2[8]    = {0b00000,0b01000,0b00100,0b10100,0b10100,0b00100,0b01000,0b00000};
uint8_t wifi_signal_3[8]    = {0b00010,0b01001,0b00101,0b10101,0b10101,0b00101,0b01001,0b00010};

uint8_t signal_0[8]         = {0b00000,0b10001,0b01010,0b00100,0b01010,0b10001,0b00000,0b10000};
uint8_t signal_1[8]         = {0b10000,0b10000,0b10000,0b11100,0b00000,0b00000,0b01000,0b11000};
uint8_t signal_2[8]         = {0b00000,0b00000,0b00000,0b00000,0b00000,0b00100,0b01100,0b11100};
uint8_t signal_3[8]         = {0b00001,0b00010,0b10100,0b01000,0b00010,0b00110,0b01110,0b11110};
uint8_t signal_4[8]         = {0b00001,0b00010,0b10100,0b01001,0b00011,0b00111,0b01111,0b11111};

uint8_t arrow_up[8]         = {0b00100,0b01110,0b10101,0b00100,0b00100,0b00100,0b00000,0b00000};
uint8_t arrow_down[8]       = {0b00000,0b00000,0b00100,0b00100,0b00100,0b10101,0b01110,0b00100};

uint8_t button_up_state;
uint8_t button_down_state;
uint8_t button_select_state;

GPS_Data gps_data_1;
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display


void buttonReader(void * parameter);
void drawScreen(void* parameter);


void initLCD_UI(void)
{
  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_DOWN, INPUT);
  pinMode(BUTTON_SELECT, INPUT);
  
  lcd.init();        // initialize the lcd 
  lcd.backlight();
  lcd.home();
  lcd.print("Connecting...");

  xTaskCreate(
    buttonReader,    // Function that should be called
    "buttonReader",  // Name of the task (for debugging)
    10000,           // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
}

static long up_pressed_time;
static long down_pressed_time;
static long select_pressed_time;

static uint8_t menu_pos = 0;

void buttonReader(void * parameter)
{
  for(;;)
  {
    // UP Button state reader
    if(!digitalRead(BUTTON_UP))
    {
      //buzzerWrite(0);
      if(millis() - up_pressed_time > SHORT_BUTTON_PRESS && millis() - up_pressed_time < LONG_BUTTON_PRESS)
      {
        button_up_state = 1;
      }
      else if(millis() - up_pressed_time > LONG_BUTTON_PRESS)
        button_up_state = 2;
    }
    else
    {
      up_pressed_time = millis();
      button_up_state = 0;
    }

    // DOWN Button state reader
    if(!digitalRead(BUTTON_DOWN))
    {
      if(millis() - down_pressed_time > SHORT_BUTTON_PRESS && millis() - down_pressed_time < LONG_BUTTON_PRESS)
        button_down_state = 1;
      else if(millis() - down_pressed_time > LONG_BUTTON_PRESS)
        button_down_state = 2;
    }
    else
    {
      down_pressed_time = millis();
      button_down_state = 0;
    }

    // SELECT Button state reader
    if(!digitalRead(BUTTON_SELECT))
    {
      if(millis() - select_pressed_time > SHORT_BUTTON_PRESS && millis() - select_pressed_time < LONG_BUTTON_PRESS)
        button_select_state = 1;
      else if(millis() - select_pressed_time > LONG_BUTTON_PRESS)
        button_select_state = 2;
    }
    else
    {
      select_pressed_time = millis();
      button_select_state = 0;
    }

    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}


void drawDebug(void)
{
  static bool wifi_msg_showed = false;
  if (!wifi_msg_showed) {
    lcd.clear();
  }
  while (!wifi_msg_showed) {
    vTaskDelay(50 / portTICK_PERIOD_MS);
    if (WiFi.status() == 3) {
      lcd.setCursor(0, 0);
      lcd.print(" Connected, IP:");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
    }
    else if (WiFi.status() == 6) {
      lcd.setCursor(0, 0);
      lcd.print("  AP created!");
      lcd.setCursor(0, 1);
      lcd.print("192.168.4.1");
    }
    if (button_select_state == 2) {
      wifi_msg_showed = true;
      lcd.clear();
    }
  }
}

void removeDebugData(void)
{  
    if(button_down_state == 2 && button_up_state == 2)
    {
      Serial.println("REMOVED DEBUG FILE!!!!!!!!!!!!!!!!!!!!!!!!!");
      removeDebugFile();
    }
}


void drawScreen(void)
{
    static uint8_t button_down_state_prev, button_up_state_prev, button_select_state_prev;
    static bool menu_pos_lock = false;
    drawDebug();

    drawWIFI(14, 0);
    drawBatteryLevel(15, 0);

    if(button_down_state_prev != button_down_state)
    {
      button_down_state_prev = button_down_state;
      //buzzerWrite(SIGNAL_DONE);
    }

    if (button_up_state == 2) {
      //menu_pos++;
    }
    else if (button_down_state == 2 && !menu_pos_lock) {
      menu_pos++;
      menu_pos_lock = true;
      lcd.clear();
    }
    else if (button_down_state == 0 && menu_pos_lock)
    {
      menu_pos_lock = false;
    }
    
    unsigned int totalBytes = SPIFFS.totalBytes();
    unsigned int usedBytes = SPIFFS.usedBytes();

    if(menu_pos > 2) {
      menu_pos = 0;
    }

static bool isDisplayed = false; 
static bool menuDisplayed = false;
static bool oneCleaning = false;
  if (totalBytes - usedBytes < 100000 && isDisplayed == false)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("   Memory is");
        lcd.setCursor(0, 1);
        lcd.print("  overwhelmed!");
        isDisplayed = true;
        menuDisplayed = true;
      }
      if (oneCleaning == false && !digitalRead(BUTTON_DOWN)) 
      {
        lcd.clear();
        oneCleaning = true;
      }
        

if(menuDisplayed == false || !digitalRead(BUTTON_DOWN))
{       
if (menu_pos == 0)
    {
      lcd.setCursor(0, 0);
      lcd.print("E");
      lcd.print(getEasting(), 3);
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print("N");
      lcd.print(getNorthing(), 3);
      lcd.print(" ");
    }
    else if (menu_pos == 1)
    {
      GPS_Data gps_data;
      getGPS_data(&gps_data);
      lcd.setCursor(0, 0);
      lcd.print("Sats: ");
      lcd.print(gps_data.sats);
      lcd.setCursor(0, 1);


      lcd.print(gps_data.hour);
      lcd.print(":");
      lcd.print(gps_data.minute);
      lcd.print(":");
      lcd.print(gps_data.second);
      lcd.print(" ");
      
     /*
      lcd.print(gps_data.alt_s);
      lcd.print(",");
      lcd.print(gps_data.geoid_height);
     */
    }

    else if (menu_pos == 2)
    {
      lcd.setCursor(0, 0);
      lcd.print("TS: ");
      lcd.print(totalBytes);
      lcd.print(" B");
      lcd.setCursor(0, 1);
      lcd.print("US: ");
      lcd.print(usedBytes);
      lcd.print(" B");
    }
    
    

    if(!digitalRead(BUTTON_UP))
    {
      //buzzerWrite(0);
      if(millis() - up_pressed_time > SHORT_BUTTON_PRESS && millis() - up_pressed_time < LONG_BUTTON_PRESS)
      {
        button_up_state = 1;
      }
      else if(millis() - up_pressed_time > LONG_BUTTON_PRESS)
        button_up_state = 2;
    }
    else
    {
      up_pressed_time = millis();
      button_up_state = 0;
    }

    // DOWN Button state reader
    if(!digitalRead(BUTTON_DOWN))
    {
      if(millis() - down_pressed_time > SHORT_BUTTON_PRESS && millis() - down_pressed_time < LONG_BUTTON_PRESS)
        button_down_state = 1;
      else if(millis() - down_pressed_time > LONG_BUTTON_PRESS)
        button_down_state = 2;
    }
    else
    {
      down_pressed_time = millis();
      button_down_state = 0;
    }

    // SELECT Button state reader
    if(!digitalRead(BUTTON_SELECT))
    {
      if(millis() - select_pressed_time > SHORT_BUTTON_PRESS && millis() - select_pressed_time < LONG_BUTTON_PRESS)
        button_select_state = 1;
      else if(millis() - select_pressed_time > LONG_BUTTON_PRESS)
        button_select_state = 2;
    }
    else
    {
      select_pressed_time = millis();
      button_select_state = 0;
    }
    removeDebugData();
    //vTaskDelay(500 / portTICK_PERIOD_MS);
}
    
}




void drawBatteryLevel(uint8_t position_x, uint8_t position_y)
{
  float battary_voltage = analogRead(VOLTAGE_READ_PIN)*MAX_BATTERY_VOLTAGE/ADC_RESOLUTION;
  if(battary_voltage < MIN_BATTERY_VOLTAGE) battary_voltage = MIN_BATTERY_VOLTAGE;
  uint8_t battery_level = round((battary_voltage-MIN_BATTERY_VOLTAGE)*6/(MAX_BATTERY_VOLTAGE-MIN_BATTERY_VOLTAGE));
  switch(battery_level)
  {
    case 0:
      lcd.createChar(0, battery_level_0);
      lcd.setCursor(position_x, position_y);
      lcd.printByte(0);
      break;
    case 1:
      lcd.createChar(1, battery_level_1);
      lcd.setCursor(position_x, position_y);
      lcd.printByte(1);
      break;
    case 2:
      lcd.createChar(2, battery_level_2);
      lcd.setCursor(position_x, position_y);
      lcd.printByte(2);
      break;
    case 3:
      lcd.createChar(3, battery_level_3);
      lcd.setCursor(position_x, position_y);
      lcd.printByte(3);
      break;
    case 4:
      lcd.createChar(4, battery_level_4);
      lcd.setCursor(position_x, position_y);
      lcd.printByte(4);
      break;
    case 5:
      lcd.createChar(5, battery_level_5);
      lcd.setCursor(position_x, position_y);
      lcd.printByte(5);
      break;
    case 6:
      lcd.createChar(6, battery_level_6);
      lcd.setCursor(position_x, position_y);
      lcd.printByte(6);
      break;
    default:
      break;
  }
}




void drawSignalLevel(uint8_t position_x, uint8_t position_y, uint8_t level)
{
  level > 4 ? level = 4 : level;
  lcd.createChar(0, signal_0);
  lcd.createChar(1, signal_1);
  lcd.createChar(2, signal_2);
  lcd.createChar(3, signal_3);
  lcd.createChar(4, signal_4);
  lcd.setCursor(position_x, position_y);
  switch(level)
  {
    case 0:
      lcd.printByte(0);
      break;
    case 1:
      lcd.printByte(1);
      break;
    case 2:
      lcd.printByte(2);
      break;
    case 3:
      lcd.printByte(3);
      break;
    case 4:
      lcd.printByte(4);
    default:
      break;
  }
}




void drawWIFI(uint8_t position_x, uint8_t position_y)
{
  int level = WiFi.RSSI();
  if(level >= -55) level = 3;
  else if(level >= -75) level = 2;
  else if(level >= -85) level = 1;
  else level = 0;
  
  switch(level)
  {
    case 0:
      lcd.createChar(0, wifi_signal_0);
      lcd.setCursor(position_x, position_y);
      lcd.printByte(0);
      break;
    case 1:
      lcd.createChar(1, wifi_signal_1);
      lcd.setCursor(position_x, position_y);
      lcd.printByte(1);
      break;
    case 2:
      lcd.createChar(2, wifi_signal_2);
      lcd.setCursor(position_x, position_y);
      lcd.printByte(2);
      break;
    case 3:
      lcd.createChar(3, wifi_signal_3);
      lcd.setCursor(position_x, position_y);
      lcd.printByte(3);
      break;
    default:
      break;
  }
}

void drawDebugMsg(String title, String msg, long time)
{
  lcd.setCursor(0, 0);
}