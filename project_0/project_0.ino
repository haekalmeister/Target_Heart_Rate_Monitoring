#include "ppg_sens.h"
#include "oled_.h"
#include <string>
#include <VibrationMotor.h>
#include <ezButton.h>

#define button_2 18
#define DEBOUNCE_TIME 50
#define MA_WINDOW_SIZE 10 // Size of the moving average window

extern ezButton button2;

// GLOBAL OBJECT & VARIABLE
ppg_data ppg(4);
oled oled(0x3C);
int age = 0;
int gender = 0;
int rhr = 0;
int thr_top;
int thr_bottom;
int count_above_threshold = 0;
int count_below_threshold = 0;
bool buzzer_status = false;
const int motorPin = 17;
const int threshold_count = 3; // Adjust as needed
bool motor_active = false;
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 1000;
unsigned long startTime;
unsigned long elapsedTime;
bool inTHRZone = false; // Whether currently in the THR zone

int bpmBuffer[MA_WINDOW_SIZE];
int bpmBufferIndex = 0;
int bpmBufferCount = 0;

// Kalman filter variables
float kalman_estimate = 0.0;
float kalman_error = 1.0;
const float kalman_q = 0.1; // Process noise covariance
const float kalman_r = 1.0; // Measurement noise covariance

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
  ppg.set_thr();  // Calculate thresholds
  rhr = ppg.get_rhr();
  thr_bottom = ppg.get_thr_bottom();
  thr_top = ppg.get_thr_top();

  Serial.print("THR Bottom: ");
  Serial.println(thr_bottom);
  Serial.print("THR Top: ");
  Serial.println(thr_top);

  oled.display_thr(rhr, thr_bottom, thr_top);  // Display thresholds on OLED

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

  // Read sensor value
  ppg.get_sensor_value();

  // Get current time
  unsigned long currentTime = millis();
  elapsedTime = (currentTime - startTime) / 1000; // Calculate elapsed time in seconds

  // Check if enough time has passed since the last update
  if (currentTime - lastUpdateTime >= updateInterval) {
    // Update the last update time
    lastUpdateTime = currentTime;
    // Calculate BPM
    int bpm = round(ppg.get_beatAvg());
    bpm = calculateMovingAverage(bpm); // Apply the moving average filter
    

    // Display heart rate on Serial Monitor
    //Serial.print("Heart Rate (BPM): ");
    //Serial.println(bpm);

    // Get THR values from ppg_data instance (static for testing, replace with dynamic values)
    int thr_top = ppg.get_thr_top();   // Adjust as needed
    int thr_bottom = ppg.get_thr_bottom(); // Adjust as needed

    // Check if heart rate is above thr_top
    if (bpm > thr_top) {
      count_above_threshold++;    // Increment counter for values above thr_top
      count_below_threshold = 0;  // Reset below threshold counter
    } else if (bpm < thr_top) {
      count_below_threshold++;    // Increment counter for values below thr_top
      count_above_threshold = 0;  // Reset above threshold counter
    }

    // Check if motor should be turned on based on consecutive values above thr_top
    if (count_above_threshold > threshold_count && !motor_active) {
      myVibrationMotor.on(255);  // Turn on vibration motor with maximum intensity
      Serial.println("Vibration Motor ON");
      motor_active = true;  // Set motor flag to active
    }

    // Check if motor should be turned off based on consecutive values below thr_top
    if (count_below_threshold > threshold_count && motor_active) {
      myVibrationMotor.off();  // Turn off vibration motor
      Serial.println("Vibration Motor OFF");
      motor_active = false;  // Set motor flag to inactive
    }

    // Display BPM and THR on OLED
    oled.screen_bpm(bpm, thr_bottom, thr_top, elapsedTime, motor_active);
    // Clear the OLED for the next update
    oled.clear();
  }
  delay(10);
}
