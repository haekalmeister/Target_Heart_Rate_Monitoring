#include "ppg_sens.h"
#include "oled_.h"
#include <string>
#include <VibrationMotor.h>

// GLOBAL OBJECT & VARIABLE
ppg_data ppg(4);
oled oled(0x3C);
int age = 0;
int thr_top = 0;
int thr_bottom = 0;
int count_threshold = 0;
bool buzzer_status = false;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("BEGIN...");

  oled.start();
  oled.clear();
  oled.screen_start();

  // BEGIN SELECTING AGE;
  age = oled.age_select(age);
  ppg.set_age(age);

  // START PPG SENSOR
  ppg.start();
  //                                  //
  //  CREATE WAIT INTERFACE FOR DETERMINING THR //
  //                                  //
  ppg.set_thr();
  thr_top = ppg.get_thr_top();
  thr_bottom = ppg.get_thr_bottom();
  //                                  //
  //  CREATE FINISH INTERFACE DETERMINING THR   //
  //                                  //

  delay(2000);
  oled.clear();
}

void loop()
{
  // put your main code here, to run repeatedly:
  unsigned long start = millis();
  ppg.get_sensor_value();
  float bpm = ppg.get_beatAvg();
  Serial.print("|| Average BPM : ");
  Serial.println(ppg.get_beatAvg());

  if (buzzer_status == false)
  {
    ppg.get_beatAvg() > thr_top ? count_threshold++ : count_threshold = 0;
  }
  else
  {
    ppg.get_beatAvg() < thr_top ? count_threshold++ : count_threshold = 0;
  }

  if (count_threshold > 10)
  {
    count_threshold = 0;
    buzzer_status = !buzzer_status;
    if (buzzer_status)
    {
      // buzzer on
    }
    else
    {
      // buzzer off
    }
  }

  oled.screen_bpm(bpm);
  oled.clear();

  int delay_time = 1000 - (millis() - start);
  delay(delay_time);
}