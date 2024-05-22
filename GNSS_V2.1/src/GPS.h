#ifndef _GPS_H
#define _GPS_H

#include "Arduino.h"

#define ENOUGH_SAT_NUM    4
#define GPS_SERIAL        Serial2

typedef struct GPS_Data
{
  bool fix;
  uint8_t sats;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  double lat;
  double lon;
  double alt_s;
  double geoid_height;
  double easting;
  double northing;
  double alt_t;
};


void initGPS();
void getGPS_data(GPS_Data* data);
double getLat();
double getLon();
double getNorthing();
double getEasting();
double getAltL();
double getAltS();
 uint16_t getyear();
  uint8_t getmonth();
  uint8_t getday();
  uint8_t gethour();
  uint8_t getminute();
  uint8_t getsecond();




#endif /* GPS_H */