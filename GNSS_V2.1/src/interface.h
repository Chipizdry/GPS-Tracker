// глобальные переменные для работы с ними в программе
int dht_int; // интервал замера температуры и влажности в секундах 
int ds_int; // интервал замера ds18b20 в секундах 
int mqtt_int; // интервал отправки данных по MQTT в секундах 
float tem; // тут храним температуру
float hum; // влажность
String point_a_list;
String mntp_table[16];
uint8_t mntp_num;

void interface();

void parameters(){

// создаем параметры для нашего проекта
  jee.var("wifi", "STA"); // режим работы WiFi по умолчанию ("STA" или "AP")  (параметр в энергонезависимой памяти)
  jee.var("ssid", "TP-Link_FA4F"); // имя точки доступа к которой подключаемся (параметр в энергонезависимой памяти)
  jee.var("pass", "19481555"); // пароль точки доступа к которой подключаемся (параметр в энергонезависимой памяти)

  jee.var("op_mode", "");
  jee.var("points_saved_list", "Points");
  jee.var("point_name", "");
  jee.var("name_auto_num", "");
  
  jee.var("save_debug_data", "");

  jee.var("point_to_a_list", "");
  jee.var("points_a_list", "");

  jee.var("ntrip_host", "host");
  jee.var("ntrip_port", "0000");
  jee.var("ntrip_mntp", "mount point");
  jee.var("ntrip_user", "user");
  jee.var("ntrip_passwd", "");
  jee.var("ntrip_dbg", "");

  jee.var("antenna_height", "0");
  jee.var("central_parallel", "0");
  jee.var("central_meridian", "0");
  jee.var("false_easting", "0");
  jee.var("false_northing", "0");
  jee.var("t_elipsoid_semimajor", "6378245.0");
  jee.var("t_elipsoid_flattening", "298.3");

  jee.var("file_cn", "0");
}

void update(){ // функция выполняется после ввода данных в веб интерфейсе. получение параметров из веб интерфейса в переменные
  ntripSetParams(jee.param("ntrip_host"), jee.param("ntrip_port").toInt(), 
                jee.param("ntrip_mntp"), jee.param("ntrip_user"), jee.param("ntrip_passwd"));
}

void formProjectionSettings(){
    jee.checkbox(F("p_use_rtcm"), F("Use&#8205RTCM"));
    if(jee.param("p_use_rtcm") == "true") {
      Serial.println("Check param is OK");
      jee.button(F("b_get_rtcm"), F("gray"), F("Request RTCM"));
    }
}



void interface(){ // функция в которой мф формируем веб интерфейс
  jee.app("GNSS Prototype V2"); // название программы (отображается в веб интерфейсе)

  // создаем меню
  jee.menu("Position");
  jee.menu("Point Allocation");
  jee.menu("NTRIP");
  jee.menu("Projection Settings");
  jee.menu("Wi-Fi Settings");
  jee.menu("Files");
  // создаем контент для каждого пункта меню

  jee.page(); // page divider
  // Main page
  jee.option("source_pos", "Source coordinates");
  jee.option("target_pos", "Target coordinates");
  jee.select("op_mode", "Operating mode");

  jee.pub("positionMonitor", "", "", "#000000", "#5DFF00");
  jee.text("point_name", "Point name");
  jee.checkbox("name_auto_num", "Autonumbering");
  jee.checkbox("save_debug_data", "Debugger");
  jee.button("save_point_btn", "#000000", "Save", 1);
  jee.textarea("points_saved_list", "Saved points", true);
  jee.button("delete_point_btn", "#000000", "Delete previous", 1);

  jee.page();
  // Points allocation page
  jee.pub("distanceMonitor", "", "", "#000000", "#5DFF00");
  jee.text("point_to_a_list", "Point (X, Y, height)");
  jee.button("save_point_a_list_btn", "#000000", "Add point to list");
  
  //jee.textarea("points_a_list", "Points allocation list", true);
 
  jee.page();
  // NTRIP Settings
  jee.text("ntrip_host", "Host");
  jee.number("ntrip_port", "Port");
  //Serial.print("Mount point number: ");
  //mntp_num = getSourceTable(mntp_table);
  /*
  Serial.println(mntp_num);
  for (int i = 0; i < mntp_num; i++)
  {
    jee.option(mntp_table[i], mntp_table[i]);
    Serial.print("Mount point: ");
    Serial.println(mntp_table[i]);
  }
  jee.select("ntrip_mntp", "Mount point(Update page to get a list)");
  */
  jee.text("ntrip_mntp", "Mount point");
  jee.text("ntrip_user", "User");
  jee.password("ntrip_passwd", "Password");
  jee.textarea("ntrip_dbg", "NTRIP debug monitor", true);
  jee.button("request_mntp_list", "#000000", "Request mount points");
  jee.button("connect_ntrip", "#000000", "Connect");

  

  jee.page(); // page divider RTCM Settings
  jee.number("antenna_height", "Antenna height");
  jee.number("central_parallel", "Central Parallel");
  jee.number("central_meridian", "Central Meridian");
  jee.number("false_easting", "False Easting");
  jee.number("false_northing", "False Northing");
  jee.number("t_elipsoid_semimajor", "Target Elipsoid Semimajor (a)");
  jee.number("t_elipsoid_flattening", "Target Elipsoid Flattening (1/f)");
  jee.button("save_proj_settings", "#000000", "Save");
  


  jee.page(); // page divider
  // Страница "Настройки WiFi"
  jee.formWifi(); // форма настроек Wi-Fi
  jee.page(); // page divider

  for (int i = 0; i < POINTS_FILES_NUM; i++) {
    jee.option(String(i), POINTS_FILE_NAME + String(i));
  }
  //===25/08===
  jee.option(String(4), "debug_file");
  //===25/08===
  jee.select("file_cn", "File");
  jee.button("load_file", "#000000", "Load file");
  jee.textarea("file_content", "File content", true);
  jee.page(); // page divider
}