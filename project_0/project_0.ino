#include "ppg_sens.h"
#include "oled_.h"
#include <string>
#include <VibrationMotor.h>
// GLOBAL OBJECT & VARIABLE
ppg_data ppg(4);
oled oled(0x3C);
int age = 0;
int gender = 0;
int thr_top;
int thr_bottom;
int count_above_threshold = 0;
int count_below_threshold = 0;
bool buzzer_status = false;
const int motorPin = 17;
const int threshold_count = 15;
bool motor_active = false;
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 1000;

VibrationMotor myVibrationMotor(motorPin);

void setup()
{
  Serial.begin(115200);
  Serial.println("BEGIN...");

  oled.start();
  oled.clear();

  gender = oled.gender_select(gender);
  ppg.set_gender(gender);
  age = oled.age_select(age);
  ppg.set_age(age);
  ppg.start();
  ppg.set_thr(); // Calculate thresholds

  thr_bottom = ppg.get_thr_bottom();
  thr_top = ppg.get_thr_top();

  Serial.print("THR Bottom: ");
  Serial.println(thr_bottom);
  Serial.print("THR Top: ");
  Serial.println(thr_top);

  oled.display_thr(thr_bottom, thr_top); // Display thresholds on OLED

  delay(5000);
  oled.clear();
}

void loop()
{
// Read sensor value
  ppg.get_sensor_value();

  // Get current time
  unsigned long currentTime = millis();

  // Check if enough time has passed since the last update
  if (currentTime - lastUpdateTime >= updateInterval) {
    // Update the last update time
    lastUpdateTime = currentTime;

    // Calculate BPM
    float bpm = ppg.get_beatAvg();

    // Display heart rate on Serial Monitor
    Serial.print("Heart Rate (BPM): ");
    Serial.println(bpm);

    // Get THR values from ppg_data instance
    int thr_top = ppg.get_thr_top();
    int thr_bottom = ppg.get_thr_bottom();

    // Check if heart rate is above thr_top
    if (bpm > thr_top) {
      count_above_threshold++; // Increment counter for values above thr_top
      count_below_threshold = 0; // Reset below threshold counter
    } else if (bpm < thr_top) {
      count_below_threshold++; // Increment counter for values below thr_top
      count_above_threshold = 0; // Reset above threshold counter
    }

    // Check if motor should be turned on based on consecutive values above thr_top
    if (count_above_threshold > threshold_count && !motor_active) {
      myVibrationMotor.on(); // Turn on vibration motor
      Serial.println("Vibration Motor ON");
      motor_active = true; // Set motor flag to active
    }

    // Check if motor should be turned off based on consecutive values below thr_top
    if (count_below_threshold > threshold_count && motor_active) {
      myVibrationMotor.off(); // Turn off vibration motor
      Serial.println("Vibration Motor OFF");
      motor_active = false; // Set motor flag to inactive
    }

    // Display BPM and THR on OLED
    oled.screen_bpm(bpm, thr_bottom, thr_top);

    // Clear the OLED for the next update
    oled.clear();
  }

  // Add a short delay to reduce sensor reading frequency (optional)
  delay(10); // Adjust this delay as needed to optimize performance
}