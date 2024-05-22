#include "PointsFS.h"
#include "RTCM.h"
#include "Transformations.h"
#include <fstream>
#include "GPS.h"

extern String transponse (uint8_t times);

extern type1021_data data1021;
extern type1023_data data1023;
extern type1025_data data1025;

String path_to_file[POINTS_FILES_NUM];
File pFile;

File dbgFile; 


extern double ecef_x_source, ecef_y_source, ecef_z_source;
extern double ecef_x_target, ecef_y_target, ecef_z_target;
extern double lat_target, lon_target, height_target;
extern double northing_target, easting_target;
extern GPS_Data gpsData;

int initPointsFS()
{

  Serial.println("Init_PointsFS");

   if(!SPIFFS.begin(true)){ //Инициализируем SPIFFS
  Serial.println("An Error has occurred while mounting SPIFFS");
     return-1;
   }
  
  Serial.println("SPiffs_running");
  
    uint8_t file_num;
    for (file_num = 0; file_num < POINTS_FILES_NUM; file_num++) {
     path_to_file[file_num] = "/saved_points_" + String(file_num);
   }

   Serial.println("SPiffs_path_created");

    return 1;
}

int openPointsFile(uint8_t file_num, String *file_text)
{
  if(file_num >= POINTS_FILES_NUM) return -1;

  file_text->clear();
  pFile = SPIFFS.open(path_to_file[file_num], "r");
  while (pFile.available()) {
    *file_text += char(pFile.read());
  }
  pFile.close();
  return 1;
}
//===25/08===
int openDebugFile(String *debug_file_text)
{
  debug_file_text->clear();
  dbgFile = SPIFFS.open("/debug_file.cpp", FILE_READ);
  while(dbgFile.available()) 
  {
    //Serial.write(dbgFile.read());
    *debug_file_text += char(dbgFile.read());
  }  
  dbgFile.close();
  return 1;
}
//===25/08===
int savePoint(uint8_t file_num, String *file_text, String point_name, double coord_x, double coord_y, double coord_z,uint8_t date_year,uint8_t date_mounth,uint8_t date_day,uint8_t date_hour,uint8_t date_minute,uint8_t date_second)
{
  if(file_num >= POINTS_FILES_NUM) return -1;

  String p_line;
  Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  Serial.println(path_to_file[file_num]);
  pFile = SPIFFS.open(path_to_file[file_num], "a");
  Serial.println(pFile.size());
  p_line += String(point_name) += String(", ") += String(coord_x, 3) += String(", ") += String(coord_y, 3)\
         += String(", ") += String(coord_z, 2)+= String("- ")+= String(date_year)+= String("/")+= String(date_mounth)+= String("/")+= String(date_day)+= String("--")+= transponse(date_hour)+= String(":")+= transponse(date_minute)+= String(":")+= transponse(date_second) +=String("\n");
  
  if (pFile.print(p_line)) {
               //  "Сообщение-пример, сохраненное в режиме записи"
    Serial.println("Message successfully written");  
               //  "Сообщение успешно записано"
  }
  else {
    Serial.print("Writing message failed!!");
             //  "Запись сообщения не удалась!!"
  }
  pFile.close();
  Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  
  return 1;
}


int saveDataStruct()
{
  String s1_line; 

  

  Serial.println("******************************");
  dbgFile = SPIFFS.open("/debug_file.cpp", "a");
  Serial.println(SPIFFS.exists("/debug_file.cpp"));
  
      s1_line += String("\n1021:\n") += String("    Counter: ") += String("\n") += String("        Src-Name: ") += String(data1021.src_name_cnt) += String("    Trg-Name: ") 
         += String(data1021.trg_name_cnt) += String("\n") += String("    Name:\n") += String("        Trg: ") += String(data1021.trg_name) += String("    Src: ") 
         += String(data1021.src_name) += String("\n") += String("    Sys ID: ") += String(data1021.sys_id) += String("\n") += String("    Utilized Transformation Message: ") 
         += String(data1021.utl_trn) += String("\n") += String("    Plt Num: ") += String(data1021.plt_num) += String(" ") += String("    Comp Ind: ") += String(data1021.comp_ind) 
         += String(" ") += String("    H Ind: ") += String(data1021.h_ind) += String("\n") += String("    Ori:\n") += String("        Lat: ") += String(data1021.ori_lat) 
         += String(" ") += String("    Lon: ") += String(data1021.ori_lon) += String("\n");   
         dbgFile.print(s1_line);
         s1_line = "";

      s1_line += String("    Extension:\n") += String("        N/S: ") += String(data1021.ns_ext) 
         += String(" ") += String("    E/W: ") += String(data1021.ew_ext) += String("\n") += String("    Translation:\n") += String("        X: ") += String(data1021.trl_x) 
         += String(" ") += String("    Y: ") += String(data1021.trl_y) += String(" ") += String("    Z: ") += String(data1021.trl_z) += String("\n") 
         += String("    Rotation:\n") += String("        X: ") += String(data1021.rotation_x) += String(" ") += String("    Y: ") += String(data1021.rotation_y) 
         += String(" ") += String("    Z: ") += String(data1021.rotation_z) += String("\n");
         dbgFile.print(s1_line);
         s1_line = "";

      s1_line += String("    Scl Corr: ") += String(data1021.scale) += String("\n") 
         += String("1023:\n") += String("    Sys ID: ") += String(data1023.sys_id) += String("\n") += String("    Shift:\n") += String("        Hor: ") 
         += String(data1023.h_shift) += String(" ") += String("    Vert: ") += String(data1023.v_shift) += String("\n") += String("    Origin of Grids:\n")
         += String("        Lat: ") += String(data1023.grids_ori_lat) += String(" ") += String("    Lon: ") += String(data1023.grids_ori_lon) += String("\n")
         += String("    Grid Extension:\n") += String("        N/S: ") += String(data1023.grids_ns_ext) += String(" ") += String("    E/W: ") += String(data1023.grids_ew_ext) 
         += String("\n") += String("    Offset:\n") += String("        Lat: ") += String(data1023.lat_offset) += String(" ") += String("    Lon: ") 
         += String(data1023.lon_offset) += String(" ") += String("    H: ") += String(data1023.h_offset) += String("\n");
         dbgFile.print(s1_line);
         s1_line = "";

      s1_line += String("    Residual:\n");
       for(int i = 0; i < 16; i++)
       { 
         s1_line += String(data1023.lat_res[i]) += String(", ") += String(data1023.lon_res[i]) += String(", ") 
         += String(data1023.h_res[i]) += String("\n");
       }

      s1_line += String("    Interpolation Method:\n") += String("        Hor: ") += String(data1023.h_inter_method) += String(" ") += String("    Ver: ") 
         += String(data1023.v_inter_method) += String("\n") += String("1025:\n") += String("    Sys ID: ") += String(data1025.sys_id) += String(" ") 
         += String("    Proj Type: ") += String(data1025.proj_type) += String("\n") += String("    N Origin:\n") += String("        Lat: ") += String(data1025.n_origin_lat) 
         += String(" ") += String("    Lon: ") += String(data1025.n_origin_lon) += String("\n"); 
         dbgFile.print(s1_line);
         s1_line = "";

      s1_line += String("    N Scale Corr: ") += String(data1025.n_scale) += String("\n") 
         += String("    East: ") += String(data1025.false_easting) += String("    North: ") += String(data1025.false_northing) += String("\n")
         += String("Geodetic Parameters:\n") += String("    Lat(S): ") += String(gpsData.lat, 8) += String(" ") += String("Lon(S): ") += String(gpsData.lon, 8) += String(" ")
         += String("Alt(S): ") += String(gpsData.alt_s, 8) += String("\n") += String("    ECEF (Source):\n") += String("        X: ") += String(ecef_x_source, 8) += String(" ") 
         += String("    Y: ") += String(ecef_y_source, 8) += String(" ") += String("    Z: ") += String(ecef_z_source, 8) += String("\n") += String("    Geoid Height: ")
         += String(gpsData.geoid_height, 8) += String("    GPS Northing: ") += String(gpsData.northing, 8) += String("    GPS Easting: ") += String(gpsData.easting, 8) 
         += String("\n") += String("    GPS Altitude (T): ") += String(gpsData.alt_t, 8) += String("\n");
         dbgFile.print(s1_line);
         s1_line = "";

      s1_line += String("    ECEF (Target):\n")
         += String("        X: ") += String(ecef_x_target, 8) += String(" ") += String("    Y: ") += String(ecef_y_target, 8) += String(" ") += String("    Z: ") 
         += String(ecef_z_target, 8) += String("\n") += String("    Translation:\n") += String("        X: ") += String(helmert_params.translation_x, 8) += String(" ") 
         += String("    Y: ") += String(helmert_params.translation_y, 8) += String(" ") += String("    Z: ") += String(helmert_params.translation_z, 8) += String("\n") 
         += String("    Rotation:\n") += String("        X: ") += String(helmert_params.rotation_x, 8); 
         dbgFile.print(s1_line);
         s1_line = "";

      s1_line += String(" ") += String("    Y: ") += String(helmert_params.rotation_y, 8) 
         += String(" ") += String("    Z: ") += String(helmert_params.rotation_z, 8) += String("\n") += String("    Scale: ") += String(helmert_params.scale, 8) += String("\n")
         += String("    Lat(T): ") += String(lat_target, 8) += String(" ") += String("Lon(T): ") += String(lon_target, 8) += String(" ") += String("H(T): ") += String(height_target, 8) 
         += String("\n") += String("    North(T): ") += String(northing_target, 8) += String("    East(T): ") += String(easting_target, 8)  
         += String("    Height(T): ") += String(height_target-grid_params.height_mean_offset-grid_params.antenna_height, 8) += String("\n");
         dbgFile.print(s1_line);
         s1_line = "";
/*
  dbgFile = SPIFFS.open("/debug_file.cpp", FILE_READ);
  while(dbgFile.available()) 
  {
    Serial.write(dbgFile.read());
  }
*/
  dbgFile.close();
  Serial.println("******************************");
  return 1;
}

void removeDebugFile()
{
  SPIFFS.remove("/debug_file.cpp");
}

int deletePrevPoint(uint8_t file_num, String *file_text)
{
  if(file_num >= POINTS_FILES_NUM) return -1;
  
  uint32_t f_len = file_text->length();
  int32_t  f_pos = f_len;
  uint32_t eofl_cnt = 1;
  Serial.println(f_len);

  file_text->clear();
  pFile = SPIFFS.open(path_to_file[file_num], FILE_READ);
  while (pFile.available()) {
    *file_text += char(pFile.read());
  }
  pFile.close();
  
  do {
    if (file_text->charAt(f_pos) == '\n' && eofl_cnt == 0)
      break;
    else if (file_text->charAt(f_pos) == '\n') {
      eofl_cnt--;
    }
    file_text->remove(f_pos);
    f_pos--;
  } while (f_pos >= 0);

  pFile = SPIFFS.open(path_to_file[file_num], "w");
  if (pFile.print(*file_text)) {

  }
  else {

  }

  pFile.close();
  return 1;
}