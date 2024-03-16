#include "Buzzer.h"



void buzzerTask(void * parameter);

bool signal_actve = true;
bool signal_changed = false;
uint8_t signal_type = SIGNAL_DONE;



void initBuzzer()
{
  pinMode(13, OUTPUT);
  ledcSetup(0, 4000, 8);

  xTaskCreate(
    buzzerTask,      // Function that should be called
    "buzzerTask",    // Name of the task (for debugging)
    10000,           // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
}



void signalDelay(long time_ms)
{
  long times_prev = millis();
  signal_changed = false;
  while ((millis() - times_prev) <= time_ms && !signal_changed)
  {
      /** TODO: TimeOUT */
  }
  
}



void buzzerTask(void * parameter)
{
  for(;;)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
    if(signal_actve)
    {
      ledcAttachPin(13, 0);
      switch (signal_type)
      {
      case SIGNAL_DONE:
          ledcWriteTone(0, NOTE_C6);
          signalDelay(100);
          ledcDetachPin(13);
          signalDelay(80);
          ledcAttachPin(13, 0);
          ledcWriteTone(0, NOTE_C6);
          signalDelay(100);
          break;
      case SIGNAL_DELETE:
          ledcWriteTone(0, NOTE_C5);
          signalDelay(100);
          ledcWriteTone(0, NOTE_C4);
          signalDelay(200);
          break;
      case SIGNAL_FAILURE:
          ledcWriteTone(0,NOTE_A5);
          delay(NOTE_SUSTAIN);
          ledcWriteTone(0,NOTE_B5);
          delay(NOTE_SUSTAIN);
          ledcWriteTone(0,NOTE_C5);
          delay(NOTE_SUSTAIN);
          ledcWriteTone(0,NOTE_B5);
          delay(NOTE_SUSTAIN);
          ledcWriteTone(0,NOTE_C5);
          delay(NOTE_SUSTAIN);
          ledcWriteTone(0,NOTE_D5);
          delay(NOTE_SUSTAIN);
          ledcWriteTone(0,NOTE_C5);
          delay(NOTE_SUSTAIN);
          ledcWriteTone(0,NOTE_D5);
          delay(NOTE_SUSTAIN);
          ledcWriteTone(0,NOTE_E5);
          delay(NOTE_SUSTAIN);
          ledcWriteTone(0,NOTE_D5);
          delay(NOTE_SUSTAIN);
          ledcWriteTone(0,NOTE_E5);
          delay(NOTE_SUSTAIN);
          ledcWriteTone(0,NOTE_E5);
          delay(NOTE_SUSTAIN);
          break;
      
      default:
          break;
      }
      signal_actve = false;
      ledcDetachPin(13);
      //ledcWriteTone(1, i);
    }
  }
}



void buzzerWrite(uint8_t signalType)
{
  signal_actve = true;
  signal_changed = true;
  signal_type = signalType;
}