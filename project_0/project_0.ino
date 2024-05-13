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
int count_below_threshold = 0;
bool buzzer_status = false;
const int motorPin = 17;
const int threshold_count = 15;
bool motor_active = false;

VibrationMotor myVibrationMotor(motorPin);

void setup()
{
  Serial.begin(115200);
  Serial.println("BEGIN...");

  oled.start();
  oled.clear();
  age = oled.age_select(age);
  ppg.set_age(age);
  ppg.start();
  ppg.set_thr(); // Calculate thresholds

  int thr_bottom = ppg.get_thr_bottom();
  int thr_top = ppg.get_thr_top();

  Serial.print("THR Bottom: ");
  Serial.println(thr_bottom);
  Serial.print("THR Top: ");
  Serial.println(thr_top);

  oled.display_thr(thr_bottom, thr_top); // Display thresholds on OLED

  delay(2000);
  oled.clear();
}

void loop()
{
  // put your main code here, to run repeatedly:
  unsigned long start = millis();
  ppg.get_sensor_value();
  float bpm = ppg.get_beatAvg();

  // Display heart rate on Serial Monitor
  Serial.print("Heart Rate (BPM): ");
  Serial.println(bpm);

  // Check if heart rate is below thr_top
  if (bpm < thr_top) {
    count_below_threshold++; // Increment counter for consecutive values below thr_top
  } else {
    count_below_threshold = 0; // Reset counter if heart rate exceeds thr_top
  }

  // Check if motor should be turned on/off based on consecutive values below thr_top
  if (count_below_threshold >= threshold_count && !motor_active) {
    myVibrationMotor.on(); // Turn on vibration motor
    Serial.println("Vibration Motor ON");
    motor_active = true; // Set motor flag to active
  } else if (count_below_threshold < threshold_count && motor_active) {
    myVibrationMotor.off(); // Turn off vibration motor
    Serial.println("Vibration Motor OFF");
    motor_active = false; // Set motor flag to inactive
  }

  oled.screen_bpm(bpm);
  oled.clear();
}