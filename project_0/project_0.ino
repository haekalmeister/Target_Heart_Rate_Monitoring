#include "ppg_sens.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ppg_data ppg(4);
  ppg.start();
  while (1) {
    ppg.run();
    float bpm = ppg.get_bpm();
    Serial.print("BPM : ");
    Serial.print(bpm);
    Serial.print("|| Average BPM : ");
    Serial.println(ppg.get_beatAvg());
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("HELLO");
  delay(500);
}
