
#include "main.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include "Buzzer.h"



using namespace   std;



bool BtnSvPressed = false;
String pointsListData;
String debugData;
String pointsAllocationListData;
double currentAllocationCoordinates[3];



void mqttCallback(String topic, String payload);
void savePointBtnCallback();
void debugDataSave();
void savePointToListBtnCallback();
void deletePrevPointCallback();
void savePointListCallback();
void requestMntpListCallback();
void connectNtripCallback();
void loadFileBtnCallback();
void saveProjSettingsBtnCallback();
void sendData();
void uiTask(void * parameter);




void setup()
{
  
  COMMUNICATION_SERIAL.begin(115200);
  GPS_SERIAL.begin(38400); // used for GPS

  vTaskDelay(10 / portTICK_PERIOD_MS);
  initBuzzer();
  Serial.println("Init_Test");
  initPointsFS();
  Serial.println("SPiffs_init_OK");
  initLCD_UI();
  initGPS();
  init_NTRIP();

  

  parameters(); // создаем параметры
  jee.udp(jee.mc); // Ответ на UDP запрс. в качестве аргуиена - переменная, содержащая id по умолчанию
  jee.led(2, false); // назначаем пин на светодиод, который нам будет говорит о состоянии устройства. (быстро мигает - пытается подключиться к точке доступа, просто горит (или не горит) - подключен к точке доступа, мигает нормально - запущена своя точка доступа)
  jee.ap(20000); // если в течении 20 секунд не удастся подключиться к Точке доступа - запускаем свою (параметр "wifi" сменится с AP на STA)
  jee.ui(interface); // обратный вызов - интерфейс
  jee.update(update); // обратный вызов - вызывается при введении данных в веб интерфейс, нужна для сравнения значений пременных с параметрами
  jee.begin(true); // Инициализируем JeeUI2 фреймворк. Параметр bool определяет, показывать ли логи работы JeeUI2 (дебаг)
  update(); // 'этой функцией получаем значения параметров в переменные

  saveProjSettingsBtnCallback();

  xTaskCreate(
    uiTask,    // Function that should be called
    "uiTask",  // Name of the task (for debugging)
    10000,           // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );

}

void loop()
{
  drawScreen();
}

void uiTask(void * parameter)
{
  for(;;)
  {
    static long times_1;
    jee.handle(); // цикл, необходимый фреймворку
    jee.btnCallback("save_point_btn", savePointBtnCallback);
    jee.btnCallback("delete_point_btn", deletePrevPointCallback);
    jee.btnCallback("save_point_a_list", savePointListCallback);
    jee.btnCallback("request_mntp_list", requestMntpListCallback);
    jee.btnCallback("connect_ntrip", connectNtripCallback);
    jee.btnCallback("load_file", loadFileBtnCallback);
    jee.btnCallback("save_proj_settings", saveProjSettingsBtnCallback);
    jee.btnCallback("save_point_a_list_btn", savePointToListBtnCallback);

  
    if((millis() - times_1) > 500)
    {
      int ntrip_status = ntripGetStatus();
      String status_string = "<font size=3>";
      String main_collor = "<span style=\"color: #ffff00\">";
      switch (ntrip_status)
      {
      case NTRIP_OK:
        status_string = "";
        main_collor = "<span style=\"color: #00ff00\">";
        break;

      case NTRIP_NO_CONNECTION:
        status_string += "<br><span style=\"color: #ff0000\">NTRIP: NO CONNECTION!</span>";
        break;

      case NTRIP_NO_FIXED_POSITION:
        status_string += "<br><span style=\"color: #ff0000\">NO GPS FIX!</span>";
        break;

      case NTRIP_MSG_DECODE_FAILURE:
        status_string += "<br><span style=\"color: #ff0000\">RTCM PARSING FAILURE!</span>";
        break;
    
      default:
        break;
      }

      status_string += "</h1>";

      if (jee.param("op_mode") == "source_pos")
      {
        jee.var("positionMonitor", main_collor + "<font size=5>LAT: " + String(getLat(), 8) + "<br>LON: " + String(getLon(), 8) +
                "<br>ALT: " + String(getAltS(), 3) + status_string + "</span>");
      }
      else if (jee.param("op_mode") == "target_pos")
      {
        jee.var("positionMonitor", main_collor + "<font size=5>N: " + String(getNorthing(), 3) + "<br>E: " + String(getEasting(), 3) +
                "<br>ALT: " + String(getAltL(), 3) + status_string + "</span>");
      }
      openPointsFile(jee.param("file_cn").toInt(), &pointsListData);
      jee.var("points_saved_list", POINTS_FILE_NAME + jee.param("file_cn") + ":\n" + pointsListData);


      if (jee.param("op_mode") == "source_pos")
      {
        jee.var("distanceMonitor", main_collor + "<font size=5>LAT: " + String(currentAllocationCoordinates[0] - getLat(), 8) + 
                "<br>LON: " + String(currentAllocationCoordinates[1] - getLon(), 8) +
                "<br>ALT: " + String(currentAllocationCoordinates[2] - getAltS(), 3) + status_string + "</span>");
      }
      else if (jee.param("op_mode") == "target_pos")
      {
        jee.var("distanceMonitor", main_collor + "<font size=5>N: " + String(currentAllocationCoordinates[0] - getNorthing(), 3) + 
                "<br>E: " + String(currentAllocationCoordinates[1] - getEasting(), 3) +
                "<br>ALT: " + String(currentAllocationCoordinates[2] - getAltL(), 3) + status_string + "</span>");
      }


      vTaskDelay(1 / portTICK_PERIOD_MS);
      times_1 = millis();
    }
  }
}

void mqttCallback(String topic, String payload){ // функция вызывается, когда приходят данные MQTT
  Serial.println("Message [" + topic + " - " + payload + "] ");
}

void sendData(){
  static unsigned long i;
  static unsigned int in;
  if(i + (in * 1000) > millis() || dht_int == 0) return; // если не пришло время, или интервал = 0 - выходим из функции
  i = millis();
  in = mqtt_int;
}

void savePointListCallback()
{

}

void savePointBtnCallback()
{
  String pointName =  jee.param("point_name");
  if (jee.param("name_auto_num") == "true") {
    openPointsFile(jee.param("file_cn").toInt(), &pointsListData);
    uint32_t pl_len = pointsListData.length();
    uint32_t pn_len = pointName.length();
    uint32_t name_exist_cnt = 0;
    for (size_t i = 0; i < pl_len; i++) {
      if (pointsListData.charAt(i) == '\n') {
        if (pointsListData.substring(i+1, i+1+pn_len) == pointName) {
          name_exist_cnt++;
        }
      }
      else if (i == 0) {
        if (pointsListData.substring(i, i+pn_len) == pointName) {
          name_exist_cnt++;
        }
      }
    }
    if(name_exist_cnt) {
      pointName += String(name_exist_cnt);
    }
    else if (pointName == "") {
      pointName = "1";
    }
  }
  
  {
    /* code */
  }
  
  debugDataSave();

  savePoint(jee.param("file_cn").toInt(), &pointsListData, pointName, getNorthing(), getEasting(), getAltL());
  openPointsFile(jee.param("file_cn").toInt(), &pointsListData);
  jee.var("points_saved_list", POINTS_FILE_NAME + jee.param("file_cn") + ":\n" + pointsListData);
  buzzerWrite(SIGNAL_DONE);
}

void debugDataSave()
{
  if (jee.param("save_debug_data") == "true")
  {
    saveDataStruct();
  }
}


void deletePrevPointCallback()
{
  deletePrevPoint(jee.param("file_cn").toInt(), &pointsListData);
  openPointsFile(jee.param("file_cn").toInt(), &pointsListData);
  jee.var("points_saved_list", pointsListData);
  buzzerWrite(SIGNAL_DELETE);
}

void debugNTRIP(String dbg_msg)
{
  static String ntrip_debug_str;
  if(dbg_msg.length() >= 256) return;

  if(ntrip_debug_str.length() + dbg_msg.length() >= 1024)
  {
    ntrip_debug_str.remove(0, (ntrip_debug_str.length() + dbg_msg.length() - 1024));
  }

  ntrip_debug_str += dbg_msg;
  jee.var("ntrip_dbg", POINTS_FILE_NAME + jee.param("file_cn") + ":\n" + ntrip_debug_str);
}

void requestMntpListCallback()
{
  onRequestNTRIP = true;
  debugNTRIP("Requesting mount points list ...\n");
  String mntp_table[64];
  uint8_t mntp_num =  getSourceTable(mntp_table, debugNTRIP);
  if(!mntp_num) {
    debugNTRIP("No mount points found\n");
  }
  else debugNTRIP("\n\nMount points list:\n");
  for (uint8_t i = 0; i < mntp_num; i++) {
    Serial.println();
    Serial.println(mntp_table[i]);
    debugNTRIP(String(i) + ") " + mntp_table[i] + "\n");
  }
  Serial.println("RequestComlite");
  onRequestNTRIP = false;
}

void connectNtripCallback()
{
  onRequestNTRIP = true;
  requestRaw(debugNTRIP);
  onRequestNTRIP = false;
}

void loadFileBtnCallback()
{
  Serial.println(jee.param("file_cn").toInt());
  openPointsFile(jee.param("file_cn").toInt(), &pointsListData);
  jee.var("file_content", pointsListData);
//===25/08===
  openDebugFile(&debugData);
  jee.var("file_content", debugData);
//===25/08===
}

void saveProjSettingsBtnCallback()
{
  setAntennaHeight(jee.param("antenna_height").toFloat());
  setCentralParallel(jee.param("central_parallel").toFloat());
  setCentralMeridian(jee.param("central_meridian").toFloat());
  setFalseEasting(jee.param("false_easting").toFloat());
  setFalseNorthing(jee.param("false_northing").toFloat());
  setTargetSemimajor(jee.param("t_elipsoid_semimajor").toFloat());
  setTargetFlattening(jee.param("t_elipsoid_flattening").toFloat());
}

void savePointToListBtnCallback()
{
  Serial.println(jee.param("point_to_a_list"));
  String coordinates_string = jee.param("point_to_a_list");
  String coordinate_temp;
  uint16_t coordinates_pos_cnt = 0;
  double coordinates[3];
  bool invalid_coordinates = false;
  


  for (size_t i = 0; i < 3; i++)
  {
    while(coordinates_pos_cnt <= coordinates_string.length() && 
        (!isDigit(coordinates_string[coordinates_pos_cnt])))
    {
      coordinates_pos_cnt++;
    }
    
    while(coordinates_pos_cnt <= coordinates_string.length() &&
          (isDigit(coordinates_string[coordinates_pos_cnt]) ||
          coordinates_string[coordinates_pos_cnt] == '.'))
    {
      coordinate_temp+=coordinates_string[coordinates_pos_cnt];
      coordinates_pos_cnt++;
    }

    if(coordinate_temp.length() > 0)
    {
      coordinates[i] = coordinate_temp.toDouble();
    }
    else
    {
      invalid_coordinates = true;
    }
    
    Serial.println(coordinate_temp);
    coordinate_temp.clear();
  }

  if(!invalid_coordinates)
  {
    buzzerWrite(SIGNAL_DONE);
    jee.var("points_a_list", pointsAllocationListData);

    Serial.println("VALIDE");
    for (size_t i = 0; i < 3; i++)
    {
      currentAllocationCoordinates[i] = coordinates[i];
      Serial.println(currentAllocationCoordinates[i]);
    }
  }
  else
  {
    buzzerWrite(SIGNAL_FAILURE);
    Serial.println("INVALIDE!!!");
  }
  

}