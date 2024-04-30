#include "ppg_sens.h"
#include "oled_.h"
#include <string>
#include <VibrationMotor.h>

//GLOBAL OBJECT & VARIABLE
ppg_data ppg(4);
oled oled(0x3C);
int age = 0;
int count_threshold = 0;
bool buzzer_status = false;
int thr = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("BEGIN...");

  oled.start();
  oled.clear();
  oled.screen_start();
  oled.age_select(age);
  delay(2000);
  ppg.start();
  oled.determining_thr();
  int thr = ppg.get_THR(age);
  oled.display_thr(thr);
  delay(2000);
  oled.clear();
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long start = millis();
  ppg.run();
  float bpm = ppg.get_beatAvg();
  Serial.print("|| Average BPM : ");
  Serial.println(ppg.get_beatAvg());

  if (buzzer_status == false) {
    ppg.get_beatAvg() > thr ? count_threshold++ : count_threshold = 0;

  } else {
    ppg.get_beatAvg() < thr ? count_threshold++ : count_threshold = 0;
  }

  if (count_threshold > 10) {
    count_threshold = 0;
    buzzer_status = !buzzer_status;
    if (buzzer_status) {
      //buzzer on
    } else {
      //buzzer off
    }
  }

  oled.screen_bpm(bpm);
  oled.clear();

  int delay_time = 1000 - (millis() - start);
  delay(delay_time);
}