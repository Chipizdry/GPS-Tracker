#include "RTCM.h"
#include "Transformations.h"
#include "NTRIPClient.h"
#include "GPS.h"

#include <String.h>

type1021_data data1021;
type1023_data data1023;
type1025_data data1025;

extern bool onRequestNTRIP;
bool rtcm_t_received = false;
int ntrip_status = NTRIP_NO_CONNECTION;
GPS_Data gps_data;
NTRIPClient ntrip_client;


struct NTRIP_Settings
{
  

   /*
    String host     = "195.16.76.194";
    int   httpPort = 2999;
    String mntpnt   = "AUTOCS63-3H";
    String user     = "landcom2";
    String passwd   = "landcomZP2";
*/

    String host     = "gnss.org.ua";
    int   httpPort = 2113;
    String mntpnt   = "AUTOCS63-3H";
    String user     = "2022landcom-2";
    String passwd   = "476804";
  
               

} ntripSettings;

void ntripReader(void * parameter);


void ntripSetParams(String host, uint16_t port, String mount_point, String user, String passwd)
{
  ntripSettings.host     = host;
  ntripSettings.httpPort = port;
  ntripSettings.mntpnt   = mount_point;
  ntripSettings.user     = user;
  ntripSettings.passwd   = passwd;

}

uint8_t getSourceTable(String* mount_points_table, void (*debug_func)(String))
{
  bool source_table_received = false;
  long times_1 = 0;
  uint8_t mntp_cnt = 0; 
  ntrip_client.reqSrcTbl(const_cast<char *>(ntripSettings.host.c_str()), ntripSettings.httpPort);
  Serial.println("Ntrip_client_running...");
  times_1 = millis();
  while(!source_table_received && ((millis() - times_1) < 4000))
  {
    while(ntrip_client.available())
    {
      static String src_tbl_string;
      char c = ntrip_client.read();

     
      src_tbl_string += c;
     
      if(c == '\n' || sizeof(src_tbl_string) > 128) {
         Serial.println("Ansver "+src_tbl_string);
          int str_pos_1 = src_tbl_string.indexOf("STR;");
          int str_pos_2 = src_tbl_string.indexOf(";", str_pos_1+4);

          if(str_pos_1 > -1 && str_pos_2 > -1) {
            String mount_point = src_tbl_string.substring(str_pos_1+4, str_pos_2 - str_pos_1);
            mount_points_table[mntp_cnt++] = mount_point;
              
                  /////////////////////////////
            Serial.println("Found "+mount_point);
            /////////////////////////////
          }
           

          //debug_func(src_tbl_string);
          src_tbl_string.clear();

           

      }

         Serial.println( mntp_cnt);

      if(src_tbl_string.indexOf("ENDSOURCETABLE") > - 1) {
        //debug_func(src_tbl_string);
        source_table_received = true;
        Serial.println("ENDSOURCETABLE");
        src_tbl_string.clear();
        break;
      }

       
    }
     
  }

   
  return mntp_cnt;
}

bool requestRaw(void (*debug_func)(String))
{
    debug_func("\n\nEstablishing a connection:\n");
    debug_func("HOST = " + ntripSettings.host + "\n"\
              +"PORT = " + ntripSettings.httpPort + "\n"\
              +"MNTP = " + ntripSettings.mntpnt + "\n"\
              +"USER = " + ntripSettings.user + "\n"\
              +"PASS = " + ntripSettings.passwd + "\n");

    bool result = ntrip_client.reqRaw(const_cast<char *>(ntripSettings.host.c_str()), ntripSettings.httpPort,
                    const_cast<char *>(ntripSettings.mntpnt.c_str()), const_cast<char *>(ntripSettings.user.c_str()),
                    const_cast<char *>(ntripSettings.passwd.c_str()));

    if(result) debug_func("Connection succeeded\n");
    else debug_func("Connection FAILED!!!\n");
    return result;
}

unsigned int rtk_crc24q(const unsigned char *buff, int len)
{
    unsigned int crc=0;
    int i;
    
    for (i=0;i<len;i++) crc=((crc<<8)&0xFFFFFF)^tbl_CRC24Q[(crc>>16)^buff[i]];
    return crc;
}



uint32_t getbitu(const uint8_t *buff, uint32_t pos, uint8_t len)
{
    uint32_t bits = 0;
    for (uint32_t i = pos; i < pos + len; i++)
    {
        bits = (bits << 1) + ((buff[i/8] >> (7 - i%8)) & 1u);
    }
    return bits;
}



int32_t getbits(const uint8_t *buff, uint32_t pos, uint8_t len)
{
    int32_t bits = (int32_t)getbitu(buff, pos, len);
    /* Sign extend, taken from:
     * http://graphics.stanford.edu/~seander/bithacks.html#VariableSignExtend
     */
    int32_t m = 1u << (len - 1);
    return (bits ^ m) - m;
}

// land080720com

void setbitu(uint8_t *buff, uint32_t pos, uint32_t len, uint32_t data)
{
    uint32_t mask = 1u << (len - 1);

    if (len <= 0 || 32 < len)
        return;
        
    for (uint32_t i = pos; i < pos + len; i++, mask >>= 1)
    {
        if (data & mask)
            buff[i/8] |= 1u << (7 - i % 8);
        else
            buff[i/8] &= ~(1u << (7 - i % 8));
    }
}



void setbits(uint8_t *buff, uint32_t pos, uint32_t len, int32_t data)
{
    setbitu(buff, pos, len, (uint32_t)data);
}



void init_NTRIP()
{
  xTaskCreate(
    ntripReader,    // Function that should be called
    "NTRIP_Reader",  // Name of the task (for debugging)
    10000,           // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
}



/**
 *|===============================================================|
 *|                  1021 DATA DECODER                            |
 *|                   decode_type1021                             |
 *| arg:  none, use global defined structure                      |
 *| return:                                                       |
 *| decode type 1021: helmert/abridged molodenski                 |
 *|===============================================================|
 */
/** TODO: Add checking */ 
static int decode_type1021()
{
    uint16_t bc = 36;

    data1021.src_name_cnt = getbitu(rtcm.data_buf, bc, 5); bc+=5;
    for(uint8_t i = 0; i < data1021.src_name_cnt; i++)
    {
        data1021.src_name[i] = getbitu(rtcm.data_buf, bc, 8); bc+=8;
    }

    data1021.trg_name_cnt = getbitu(rtcm.data_buf, bc, 5); bc+=5;
    for(uint8_t i = 0; i < data1021.trg_name_cnt; i++)
    {
        data1021.trg_name[i] = getbitu(rtcm.data_buf, bc, 8); bc+=8;
    }

    data1021.sys_id     = getbitu(rtcm.data_buf, bc, 8);   bc+=8;
    data1021.utl_trn    = getbitu(rtcm.data_buf, bc, 10);  bc+=10;
    data1021.plt_num    = getbitu(rtcm.data_buf, bc, 5);   bc+=5;
    data1021.comp_ind   = getbitu(rtcm.data_buf, bc, 4);   bc+=4;
    data1021.h_ind      = getbitu(rtcm.data_buf, bc, 2);   bc+=2;
    data1021.ori_lat    = getbits(rtcm.data_buf, bc, 19);  bc+=19;
    data1021.ori_lon    = getbits(rtcm.data_buf, bc, 20);  bc+=20;
    data1021.ns_ext     = getbitu(rtcm.data_buf, bc, 14);  bc+=14;
    data1021.ew_ext     = getbitu(rtcm.data_buf, bc, 14);  bc+=14;
    data1021.trl_x      = getbits(rtcm.data_buf, bc, 23);  bc+=23;
    data1021.trl_y      = getbits(rtcm.data_buf, bc, 23);  bc+=23;
    data1021.trl_z      = getbits(rtcm.data_buf, bc, 23);  bc+=23;
    data1021.rotation_x = getbits(rtcm.data_buf, bc, 32);  bc+=32;
    data1021.rotation_y = getbits(rtcm.data_buf, bc, 32);  bc+=32;
    data1021.rotation_z = getbits(rtcm.data_buf, bc, 32);  bc+=32;
    data1021.scale      = getbits(rtcm.data_buf, bc, 25);  bc+=25;
    data1021.maj_s_axis = getbitu(rtcm.data_buf, bc, 24);  bc+=24;
    data1021.min_s_axis = getbitu(rtcm.data_buf, bc, 25);  bc+=25;
    data1021.maj_t_axis = getbitu(rtcm.data_buf, bc, 24);  bc+=24;
    data1021.min_t_axis = getbitu(rtcm.data_buf, bc, 25);  bc+=25;
    return 21;
}



/**
 *|===============================================================|
 *|                  1023 DATA DECODER                            |
 *|                   decode_type1021                             |
 *| arg:  none, use global defined structure                      |
 *| return:                                                       |
 *| decode type 1023: Residuals, elipsoidal grid representation   |
 *|===============================================================|
 */
/** TODO: Add checking */ 
static int decode_type1023()
{
    uint16_t bc = 36;

    data1023.sys_id        = getbitu(rtcm.data_buf, bc, 8);    bc+=8;
    data1023.h_shift       = getbitu(rtcm.data_buf, bc, 1);    bc+=1;
    data1023.v_shift       = getbitu(rtcm.data_buf, bc, 1);    bc+=1;
    data1023.grids_ori_lat = getbits(rtcm.data_buf, bc, 21);   bc+=21;
    data1023.grids_ori_lon = getbits(rtcm.data_buf, bc, 22);   bc+=22;
    data1023.grids_ns_ext  = getbitu(rtcm.data_buf, bc, 12);   bc+=12;
    data1023.grids_ew_ext  = getbitu(rtcm.data_buf, bc, 12);   bc+=12;
    data1023.lat_offset    = getbits(rtcm.data_buf, bc, 8);    bc+=8;
    data1023.lon_offset    = getbits(rtcm.data_buf, bc, 8);    bc+=8;
    data1023.h_offset      = getbits(rtcm.data_buf, bc, 15);   bc+=15;
    for(uint8_t i = 0; i < 16; i++)
    {
        data1023.lat_res[i] = getbits(rtcm.data_buf, bc, 9);   bc+=9;
        data1023.lon_res[i] = getbits(rtcm.data_buf, bc, 9);   bc+=9;
        data1023.h_res[i]   = getbits(rtcm.data_buf, bc, 9);   bc+=9;
    }
    data1023.h_inter_method = getbitu(rtcm.data_buf, bc, 2);   bc+=2;
    data1023.v_inter_method = getbitu(rtcm.data_buf, bc, 2);   bc+=2;

    uint8_t i_prev = 1;
    for(uint8_t i = 0; i < 16; i++)
    {
        Serial.print(data1023.lat_res[i]);
        Serial.print(", ");
        Serial.print(data1023.lon_res[i]);
        Serial.print(", ");
        Serial.print(data1023.h_res[i]);
        Serial.print(" | ");
        if(i - i_prev >= 4)
        {
            Serial.println();
            i_prev = i;
        }
    }
    Serial.println();
    return 23;
}


/**
 *|===============================================================|
 *|                  1023 DATA DECODER                            |
 *|                   decode_type1025                             |
 *| arg:  none, use global defined structure                      |
 *| return:                                                       |
 *| decode type 1025: Projection Types except LCC2SP, OM          |
 *|===============================================================|
 */
/** TODO: Add checking */ 
static int decode_type1025()
{
    uint16_t bc = 36;

    data1025.sys_id         = getbitu(rtcm.data_buf, bc, 8);    bc+=8;
    data1025.proj_type      = getbitu(rtcm.data_buf, bc, 6);    bc+=6;
    data1025.n_origin_lat   = getbits(rtcm.data_buf, bc, 34);   bc+=34;
    data1025.n_origin_lon   = getbits(rtcm.data_buf, bc, 35);   bc+=35;
    data1025.n_scale        = getbitu(rtcm.data_buf, bc, 30);   bc+=30;
    data1025.false_easting  = getbitu(rtcm.data_buf, bc, 36);   bc+=36;
    data1025.false_northing = getbits(rtcm.data_buf, bc, 35);   bc+=35;
    return 25;
}


/**
 *|===============================================================|
 *|                  RTCM3 DATA DECODER                           |
 *|                     decode_rtcm3                              |
 *| arg:  none, use global defined structure                      |
 *| return: int (0 if failure...                                  |
 *|===============================================================|
 */
int decode_rtcm3()
{
    int ret=0, type = getbitu(rtcm.data_buf, 24, 12), week;
    //Serial.println(type);

    switch (type)
    {
        case 1021:
            ret = decode_type1021();
            break;

        case 1023:
            ret = decode_type1023();
            break;

        case 1025:
            ret = decode_type1025();
            break;
    }

    if (ret >= 0)
    {
        type -= 1000;
        //if (1 <= type && type <= 299) rtcm->nmsg3[type]++;
        //else rtcm->nmsg3[0]++;
    }
    return ret;
}




/**
 *|===============================================================|
 *|                     RTCM DECODER                              |
 *|                      inputRtcm                                |
 *| arg:  uint8_t data - rtcm message stream input (ver 3)        |
 *| return: int (0 if failure...                                  |
 *|===============================================================|
 */
/**
 *+----------+--------+-----------+--------------------+----------+
 *| preamble | 000000 |  length   |    data message    |  parity  |
 *+----------+--------+-----------+--------------------+----------+
 *|<-- 8 --->|<- 6 -->|<-- 10 --->|<--- length x 8 --->|<-- 24 -->|
 */
uint8_t inputRtcm(uint8_t data)
{
    /* synchronize frame */
    if (rtcm.msg_byte_cnt == 0)
    {
        if (data != RTCM3PREAMB) return 0;
        rtcm.data_buf[rtcm.msg_byte_cnt++] = data;
        return 0;
    }

    rtcm.data_buf[rtcm.msg_byte_cnt++] = data;

    if (rtcm.msg_byte_cnt == 3)
    {
        rtcm.data_len = getbitu(rtcm.data_buf, 14, 10) + 3; /* length without parity */
    }

    if (rtcm.msg_byte_cnt < 3 || rtcm.msg_byte_cnt < rtcm.data_len + 3) return 0;
    rtcm.msg_byte_cnt = 0;

    /* check parity */
    if (rtk_crc24q(rtcm.data_buf, rtcm.data_len) != getbitu(rtcm.data_buf, rtcm.data_len*8, 24))
    {
        Serial.println("Parity error!!!");
        return 0;
    }
    //else Serial.println("Massage received");
    return decode_rtcm3();
}



void setTransformationValues()
{

  if(rtcm_t_received) {
    if(/*type1021_data*/data1021.maj_s_axis != 0 && /*type1021_data*/data1021.min_s_axis != 0) {
      /*
      source_elipsoid.semimajor  = 6370000.0f + (type1021_data.maj_s_axis/1000.0f);
      source_elipsoid.semiminor  = 6350000.0f + (type1021_data.min_s_axis/1000.0f);
      source_elipsoid.flattening = (source_elipsoid.semimajor - source_elipsoid.semiminor)/source_elipsoid.semimajor;
      source_elipsoid.e_sq       = source_elipsoid.flattening * (2 - source_elipsoid.flattening);

      Serial.print("source_elipsoid.semimajor = ");
      Serial.println(source_elipsoid.semimajor);
      Serial.print("source_elipsoid.semiminor = ");
      Serial.println(source_elipsoid.semiminor);
      */
    }
    /** TODO: target parrams is incorrect in SYSNET provider, check it */
    /*
    if(type1021_data.maj_t_axis != 0 && type1021_data.min_t_axis != 0) {
      target_elipsoid.semimajor  = 6370000.0f + (type1021_data.maj_t_axis/1000.0f);
      target_elipsoid.semiminor  = 6350000.0f + (type1021_data.min_t_axis/1000.0f);
      target_elipsoid.flattening = (target_elipsoid.semimajor - target_elipsoid.semiminor)/target_elipsoid.semimajor;
      target_elipsoid.e_sq       = target_elipsoid.flattening * (2 - target_elipsoid.flattening);
    }
    */
    helmert_params.translation_x = /*type1021_data*/data1021.trl_x/1000.0f;
    helmert_params.translation_y = /*type1021_data*/data1021.trl_y/1000.0f;
    helmert_params.translation_z = /*type1021_data*/data1021.trl_z/1000.0f;
    helmert_params.rotation_x    = /*type1021_data*/data1021.rotation_x*0.00002f*M_PI/(180*3600);
    helmert_params.rotation_y    = /*type1021_data*/data1021.rotation_y*0.00002f*M_PI/(180*3600);
    helmert_params.rotation_z    = /*type1021_data*/data1021.rotation_z*0.00002f*M_PI/(180*3600);
    helmert_params.scale         = /*type1021_data*/data1021.scale*0.00001f + 1.0f;

    Serial.print("helmert_params.translation_x = ");
    Serial.println(helmert_params.translation_x);
    Serial.print("helmert_params.translation_y = ");
    Serial.println(helmert_params.translation_y);
    Serial.print("helmert_params.translation_z = ");
    Serial.println(helmert_params.translation_z);
    Serial.print("helmert_params.rotation_x = ");
    Serial.println(helmert_params.rotation_x);
    Serial.print("helmert_params.rotation_y = ");
    Serial.println(helmert_params.rotation_y);
    Serial.print("helmert_params.rotation_z = ");
    Serial.println(helmert_params.rotation_z);
    Serial.print("helmert_params.scale = ");
    Serial.println(helmert_params.scale);
    

    if(/*type1021_data*/data1021.utl_trn & 0x01) { // check if we need 1023 type message
      grid_params.lat_mean_offset = data1023.lat_offset*0.001f/3600.0f;
      grid_params.lon_mean_offset = data1023.lon_offset*0.001f/3600.0f;
      grid_params.height_mean_offset = data1023.h_offset*0.01f;
      Serial.println("MEAN HEIGHT OFFSET:");
      Serial.println(/*type1021_data*/data1021.h_ind);
      Serial.println(data1023.h_offset);
      //delay(100000);
    }

    if(/*type1021_data*/data1021.utl_trn & 0x04) { // check if we need 1025 type message
      //kruger_params.false_northing = type1025_data.false_northing*0.001f;
      //kruger_params.false_easting = type1025_data.false_easting*0.001f;
      //kruger_params.false_northing = jee.param("false_northing").toFloat;
    }
  }
}



void setAntennaHeight(float a_height)
{
  grid_params.antenna_height = a_height;
}



void setCentralParallel(float c_parallel)
{
  kruger_params.central_parallel = c_parallel;
}



void setCentralMeridian(float c_meridian)
{
  kruger_params.central_meridian = c_meridian;
}



void setFalseEasting(float f_easting)
{
  kruger_params.false_easting = f_easting;
}



void setFalseNorthing(float f_northing)
{
  kruger_params.false_northing = f_northing;
}



void setTargetSemimajor(float t_semimajor)
{
  target_elipsoid.semimajor = t_semimajor;
}



void setTargetFlattening(float t_flattening)
{
  target_elipsoid.flattening = 1.0/t_flattening;
  target_elipsoid.semiminor = (-target_elipsoid.flattening*target_elipsoid.semimajor)+
				                      target_elipsoid.semimajor;
  target_elipsoid.e_sq = target_elipsoid.flattening * (2 - target_elipsoid.flattening);
}



bool getSourceTable(char* srcTable, uint16_t arraySize)
{
  if(ntrip_client.reqSrcTbl(const_cast<char *>(ntripSettings.host.c_str()), ntripSettings.httpPort))
  {
    delay(10);
    uint16_t b_cnt = 0;
    while(ntrip_client.available() && b_cnt < arraySize)
    {
      ntrip_client.readLine(srcTable, arraySize);
      b_cnt++; 
    }
    return true;
  }
  else
  {
    Serial.print("\nSourceTable request error");
    return false;
  }
}


double deg2degmin( double degree )
{
    int deg = (int)degree;
    double min = (degree - deg) * 60;
    double degmin = deg * 100 + min;
    return degmin;
}



unsigned char checksumOf( char* pbuff, int NumOfBytes )
{
    int i;
    unsigned char *buff;
  
    buff = (unsigned char*) pbuff;
  
    unsigned char c = *buff++;
    for ( i = 1; i < NumOfBytes; i++ )  c = c ^ *buff++ ;
  
    return c;
}



int ntripGetStatus()
{
    return ntrip_status;
}



void ntripReader(void * parameter)
{
  bool msg_1021_received = false;
  bool msg_1023_received = false;
  bool msg_1024_received = false;
  bool msg_1025_received = false;
  bool msg_1026_received = false;
  bool msg_1027_received = false;
  for(;;)
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
    static long millis_prev_1;
    static long millis_prev_2;

    if (!onRequestNTRIP) {
      while (ntrip_client.available())
      {
        char c = ntrip_client.read();
        Serial2.print(c); /** Send to GPS */
        uint8_t msg_type = inputRtcm(c);
        if(msg_type != 0) {
          ntrip_status = NTRIP_OK;
          millis_prev_2 = millis();
          if(msg_type == 21 || msg_type == 22) msg_1021_received = true;
          else if(msg_type == 23) msg_1023_received = true;
          else if(msg_type == 24) msg_1024_received = true;
          else if(msg_type == 25) msg_1025_received = true;
          else if(msg_type == 26) msg_1026_received = true;
          else if(msg_type == 27) msg_1023_received = true;

          if(msg_1021_received) {
            uint8_t msg_mask = msg_1023_received | msg_1024_received << 1 | msg_1025_received << 2\
                             | msg_1026_received << 3 | msg_1027_received << 4;
            if((/*type1021_data*/data1021.utl_trn & 0x1f) == msg_mask) {
              rtcm_t_received = true;
              setTransformationValues();
            
            }
          }
        }
        millis_prev_1 = millis();
      }
      if(millis() - millis_prev_1 >= NTRIP_REQUEST_TIMEOUT) {
        getGPS_data(&gps_data);
        if(!gps_data.fix) {
          ntrip_status = NTRIP_NO_FIXED_POSITION;
          /*
          ntrip_client.stop(); //Need to call "stop" function for next request.
          ntrip_client.reqRaw(const_cast<char *>(ntripSettings.host.c_str()), ntripSettings.httpPort,
                              const_cast<char *>(ntripSettings.mntpnt.c_str()), const_cast<char *>(ntripSettings.user.c_str()),
                              const_cast<char *>(ntripSettings.passwd.c_str()));
          ntrip_client.print("$GPGGA,105600.278,5031.406,N,03029.548,E,1,12,1.0,0.0,M,0.0,M,,*65");
          */
          millis_prev_1 = millis();
        }
        else {
          double hms = gps_data.hour * 10000 + gps_data.minute * 100 + gps_data.second;
          char buff[128];          
          sprintf(buff, "$GPGGA,%06.2lf,%.5lf,N,%.5lf,E,%d,%d,%.1lf,%.3lf,M,%.3lf,,",
                  hms, deg2degmin(gps_data.lat), deg2degmin(gps_data.lon),  gps_data.fix, gps_data.sats, 0, 0, 0);
          char csum = checksumOf( buff + 1, strlen( buff ) - 1 );
          sprintf( buff + strlen( buff ), "*%02x\r\n", csum );
          ntrip_client.stop(); //Need to call "stop" function for next request.
          ntrip_client.reqRaw(const_cast<char *>(ntripSettings.host.c_str()), ntripSettings.httpPort,
                              const_cast<char *>(ntripSettings.mntpnt.c_str()), const_cast<char *>(ntripSettings.user.c_str()),
                              const_cast<char *>(ntripSettings.passwd.c_str()));
          ntrip_client.write(buff, sizeof(buff));
          //Serial.println("\n buff:");
          //Serial.println(buff);
          millis_prev_1 = millis();
        }
      }
      if(millis() - millis_prev_2 >= NTRIP_REQUEST_FAILURE && gps_data.fix) {
        ntrip_status = NTRIP_MSG_DECODE_FAILURE;
      }
    }
  }
}
