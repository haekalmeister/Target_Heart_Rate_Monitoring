#include "ppg_sens.h"
#include "oled_.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ppg_data ppg(4);
  oled oled(0x3C);

  oled.start();
  oled.clear();
  oled.screen_start();

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
  Serial.println("HELLO");
  delay(500);
}
