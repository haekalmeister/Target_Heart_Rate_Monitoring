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


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("BEGIN...");

  oled.start();
  oled.clear();
  oled.screen_start();
  oled.age_select(age);

  delay(2000);
  oled.clear();

  ppg.start();
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long start = millis();
  ppg.run();
  float bpm = ppg.get_bpm();
  Serial.print("BPM : ");
  Serial.print(bpm);
  Serial.print("|| Average BPM : ");
  Serial.println(ppg.get_beatAvg());

  int thr = ppg.get_THR(age);
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
  delay(delay_time)
  // delay(500);
}
