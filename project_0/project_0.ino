#include "ppg_sens.h"
#include "oled_.h"
#include <string>


int age = 0;


//GLOBAL OBJECT & VARIABLE
ppg_data ppg(4);
oled oled(0x3C);



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
  while (1) {
    ppg.run();
    float bpm = ppg.get_bpm();
    Serial.print("BPM : ");
    Serial.print(bpm);
    Serial.print("|| Average BPM : ");
    Serial.println(ppg.get_beatAvg());

    oled.screen_bpm(bpm);
    oled.clear();
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  // delay(500);
}
