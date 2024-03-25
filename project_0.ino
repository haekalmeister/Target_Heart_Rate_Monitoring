#include "ppg_sens.h"

ppg_data ppg();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ppg.start();
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0;i<10;i++){
    ppg.run();
  }
  Serial.println(ppg.get_bpm());

}
