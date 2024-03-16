
#ifndef __MAIN_H
#define __MAIN_H


#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <Eigen30.h>
#include "LCD_UI.h"
#include "GPS.h"
#include "RTCM.h"
#include "JeeUI2.h" // Подключаем JeeUI2 фреймворк к проекту
#include "PointsFS.h"


jeeui2 jee; // Создаем объект класса для работы с JeeUI2 фреймворком
#include "interface.h"  // в этот файл вынесена работа с параметрами и с веб интерфейсом


#define COMMUNICATION_SERIAL    Serial
#define GPS_SERIAL              Serial2

bool onRequestNTRIP = false;

void debugNTRIP(String dbg_msg);

#endif /**  MAIN_H */