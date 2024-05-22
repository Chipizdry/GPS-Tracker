
#include "GPS.h"
#include "TinyGPS++.h"
#include "Transformations.h"

TinyGPSPlus gps; // The TinyGPS++ object
GPS_Data    gpsData;

void GPS_Reader(void * parameter);

void initGPS()
{
  xTaskCreate(
    GPS_Reader,      // Function that should be called
    "GPS_Reader",    // Name of the task (for debugging)
    10000,           // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
}



void getGPS_data(GPS_Data* data)
{
    *data = gpsData;
}

double getNorthing()
{
  return gpsData.northing;
}

double getEasting()
{
  return gpsData.easting;
}

double getAltL()
{
  return gpsData.alt_t;
}

double getLat()
{
  return gpsData.lat;
}

double getLon()
{
  return gpsData.lon;
}

double getAltS()
{
  return gpsData.alt_s;
}

uint16_t getyear()
{
  return gpsData.year;
}

uint8_t getmonth(){

return gpsData.month;
}

uint8_t getday(){

return gpsData.day;
}
  
uint8_t gethour(){

return gpsData.hour;
}  



uint8_t getminute(){
return gpsData.minute;
}  


uint8_t getsecond(){
return gpsData.second;
}  


void GPS_Reader(void * parameter)
{
  for(;;)
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
    while (GPS_SERIAL.available() > 0)
    {
      uint8_t gin = GPS_SERIAL.read();
      //Serial.write(gin);
      if (gps.encode(gin)){
        if (gps.time.isValid()) {
          gpsData.hour = gps.time.hour();
          gpsData.minute = gps.time.minute();
          gpsData.second = gps.time.second();
        }
        if (gps.location.isValid()) {
          gpsData.lat = gps.location.lat();
          gpsData.lon = gps.location.lng();
        }
        if (gps.altitude.isValid()) {
          gpsData.alt_s = gps.altitude.meters();
          gpsData.geoid_height = gps.geoid_height.meters();
        }

        if(gps.satellites.isValid())
        {
                gpsData.sats   = gps.satellites.value(); //number of satellites
                if(gpsData.sats >= ENOUGH_SAT_NUM) gpsData.fix = true;
                else gpsData.fix = false;
        }

        if (gps.location.isValid()) {

          geodeticToGrid(gpsData.lat, gpsData.lon, gpsData.alt_s+gpsData.geoid_height,
                        &gpsData.northing, &gpsData.easting, &gpsData.alt_t);
          //geodeticToGrid(50.5230175, 30.4912766, 123.0,
          //             &gpsData.northing, &gpsData.easting);

        }
      }
    }  
  }
}