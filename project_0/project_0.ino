#include "ppg_sens.h"
#include "oled_.h"
#include <string>
#include <VibrationMotor.h>
#include <ezButton.h>


#define DEBOUNCE_TIME 50
#define MA_WINDOW_SIZE 10 

extern ezButton button2;


ppg_data ppg(4);
oled oled(0x3C);
int age = 0;
int gender = 0;
int rhr = 0;
int thr_top;
int thr_bottom;
int count_above_threshold = 0;
int count_below_threshold = 0;
const int motorPin = 17;
const int threshold_count = 3; 
bool motor_active = false;
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 1000;
unsigned long startTime;
unsigned long elapsedTime;

int bpmBuffer[MA_WINDOW_SIZE];
int bpmBufferIndex = 0;
int bpmBufferCount = 0;

VibrationMotor myVibrationMotor(motorPin);

void setup() {
  Serial.begin(115200);
  Serial.println("BEGIN...");
  button2.setDebounceTime(DEBOUNCE_TIME);

  oled.start();
  oled.clear();

  gender = oled.gender_select(gender);
  ppg.set_gender(gender);
  age = oled.age_select(age);
  ppg.set_age(age);
  ppg.set_thr();  
  rhr = ppg.get_rhr();
  thr_bottom = ppg.get_thr_bottom();
  thr_top = ppg.get_thr_top();

  Serial.print("THR Bottom: ");
  Serial.println(thr_bottom);
  Serial.print("THR Top: ");
  Serial.println(thr_top);

  oled.display_thr(rhr, thr_bottom, thr_top); 

  delay(5000);
  oled.clear();
  startTime = millis();

  for (int i = 0; i < MA_WINDOW_SIZE; i++) {
    bpmBuffer[i] = 0;
  }
}

int calculateMovingAverage(int newBpm) {
  bpmBuffer[bpmBufferIndex] = newBpm;
  bpmBufferIndex = (bpmBufferIndex + 1) % MA_WINDOW_SIZE;
  bpmBufferCount = bpmBufferCount < MA_WINDOW_SIZE ? bpmBufferCount + 1 : MA_WINDOW_SIZE;

  int sum = 0;
  for (int i = 0; i < bpmBufferCount; i++) {
    sum += bpmBuffer[i];
  }

  return sum / bpmBufferCount;
}

void loop() {
  button2.loop();
  if (button2.isPressed()) {
    ESP.restart();
  }

  
  ppg.get_sensor_value();

  
  unsigned long currentTime = millis();
  elapsedTime = (currentTime - startTime) / 1000;  

  
  if (currentTime - lastUpdateTime >= updateInterval) {
    
    lastUpdateTime = currentTime;
    
    int bpm = round(ppg.get_beatAvg());
    bpm = calculateMovingAverage(bpm);  
    thr_bottom = ppg.get_thr_bottom();
    thr_top = ppg.get_thr_top();

    
    if (bpm > thr_top) {
      count_above_threshold++;    
      count_below_threshold = 0;  
    } else if (bpm < thr_top) {
      count_below_threshold++;    
      count_above_threshold = 0; 
    }

    
    if (count_above_threshold > threshold_count && !motor_active) {
      myVibrationMotor.on();  
      Serial.println("Vibration Motor ON");
      motor_active = true;  
    }

    
    if (count_below_threshold > threshold_count && motor_active) {
      myVibrationMotor.off(); 
      Serial.println("Vibration Motor OFF");
      motor_active = false;  
    }

    
    oled.screen_bpm(bpm, thr_bottom, thr_top, elapsedTime, motor_active);
    
    oled.clear();
  }
  delay(10);
}
