#ifndef __POINTSFS_H_
#define __POINTSFS_H_

#include "Arduino.h"
#include "SPIFFS.h"

#define POINTS_FILE_NAME         "saved_points_"
#define POINTS_FILES_NUM         4
#define POINTS_FILES_CAPACITY    100

int initPointsFS();
int openPointsFile(uint8_t file_num, String *file_text);
int openDebugFile(String *debug_file_text);
int savePoint(uint8_t file_num, String *file_text, String point_name, double coord_x, double coord_y, double coord_z);
int deletePrevPoint(uint8_t file_num, String *file_text);
int saveDataStruct();
void removeDebugFile();

#endif /* __POINTSFS_H_ */