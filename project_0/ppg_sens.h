#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
class ppg_data {
private:
  // -- INTERNAL OBJECT PROPERTIES -- //
  byte RATE_SIZE;
  byte* rates;
  byte rateSpot = 0;
  long lastBeat = 0;
  float beatsPerMinute;
  int beatAvg;
  int age = 0xFF;
  int gender = 0;
  int rhr = 0;
  int thr_top = 0;
  int thr_bottom = 0;
  int hrmax = 0;

  static const int MA_WINDOW_SIZE = 5;
  int32_t ma_buffer[MA_WINDOW_SIZE];
  int ma_index = 0;
  int32_t ma_sum = 0;

  int16_t IR_AC_Max = 20;
  int16_t IR_AC_Min = -20;

  int16_t IR_AC_Signal_Current = 0;
  int16_t IR_AC_Signal_Previous;
  int16_t IR_AC_Signal_min = 0;
  int16_t IR_AC_Signal_max = 0;
  int16_t IR_Average_Estimated;

  int16_t positiveEdge = 0;
  int16_t negativeEdge = 0;
  int32_t ir_avg_reg = 0;

  
  void clear_data() {
    for (byte x = 0; x < RATE_SIZE; x++) {
      rates[x] = 0;
    }
    rateSpot = 0;
    beatAvg = 0;
  }

  void start() {
    while (!particleSensor.begin(Wire, I2C_SPEED_FAST))  // Use default I2C port, 400kHz speed
    {
      Serial.println("Put your Finger on Sensor ");
      delay(500);
    }

    particleSensor.setup();                     // Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x0A);  // Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0);   // Turn off Green LED

    Serial.println("PPG SENSOR START");
  }

  int get_restingHeartRate_avg() {
    const unsigned long measurementDuration = 30000;
    const unsigned long startTime = millis();
    this->start();

    Serial.println("Measuring resting heart rate...");
    while (millis() - startTime < measurementDuration) {
      get_sensor_value();
      int currentBPM = this->beatAvg;


      Serial.print("Current BPM: ");
      Serial.println(currentBPM);
    }

    int restingBPM = 0;


    restingBPM = this->beatAvg;

    if (restingBPM == 0) {
      Serial.println("No valid heartbeats detected during measurement.");
    }
    return restingBPM;
  }


  int calculate_THR_Bottom() {
    if (this->gender == 0) {
      hrmax = 206.9 - (0.67 * this->age);
      return (hrmax - this->rhr) * 0.5 + this->rhr;
    } else {
      hrmax = 206 - (0.88 * this->age);
      return (hrmax - this->rhr) * 0.5 + this->rhr;
    }
  }

  int calculate_THR_Top() {
    if (this->gender == 0) {
      hrmax = 206.9 - (0.67 * this->age);
      return (hrmax - this->rhr) * 0.7 + this->rhr;
    } else {
      hrmax = 206 - (0.88 * this->age);
      return (hrmax - this->rhr) * 0.7 + this->rhr;
    }
  }

  bool checkBeat(int32_t sample) {
    bool beatDetected = false;

    IR_AC_Signal_Previous = IR_AC_Signal_Current;

    IR_Average_Estimated = averageDCEstimator(&ir_avg_reg, sample);
    IR_AC_Signal_Current = lowPassFIRFilter(sample - IR_Average_Estimated);

    if ((IR_AC_Signal_Previous < 0) & (IR_AC_Signal_Current >= 0)) {
      IR_AC_Max = IR_AC_Signal_max;
      IR_AC_Min = IR_AC_Signal_min;

      positiveEdge = 1;
      negativeEdge = 0;
      IR_AC_Signal_max = 0;

      if ((IR_AC_Max - IR_AC_Min) > 15 && (IR_AC_Max - IR_AC_Min) < 1700) {
        beatDetected = true;
      }
    }

    if ((IR_AC_Signal_Previous > 0) & (IR_AC_Signal_Current <= 0)) {
      positiveEdge = 0;
      negativeEdge = 1;
      IR_AC_Signal_min = 0;
    }

    if (positiveEdge & (IR_AC_Signal_Current > IR_AC_Signal_Previous)) {
      IR_AC_Signal_max = IR_AC_Signal_Current;
    }

    if (negativeEdge & (IR_AC_Signal_Current < IR_AC_Signal_Previous)) {
      IR_AC_Signal_min = IR_AC_Signal_Current;
    }

    return beatDetected;
  }

public:
  MAX30105 particleSensor;

  ppg_data(int rate_size)  
  {
    this->RATE_SIZE = rate_size;
    this->rates = new byte[rate_size];

    for (int i = 0; i < rate_size; i++)
      this->rates[i] = 0;
  }

  int32_t movingAverageFilter(int32_t sample) {
    ma_sum = ma_sum - ma_buffer[ma_index] + sample;
    ma_buffer[ma_index] = sample;
    ma_index = (ma_index + 1) % MA_WINDOW_SIZE;
    return ma_sum / MA_WINDOW_SIZE;
  }

  void get_sensor_value() {
    long irValue = particleSensor.getIR();
    int32_t filteredIR = movingAverageFilter(irValue);
    Serial.println(filteredIR);

    if (checkBeat(filteredIR) == true) {
      Serial.println("sensed a beat");
      long delta = millis() - lastBeat;
      lastBeat = millis();

      this->beatsPerMinute = 60 / (delta / 1000.0);

      if (this->beatsPerMinute < 255 && this->beatsPerMinute > 20) {
        rates[this->rateSpot++] = (byte)this->beatsPerMinute; 
        this->rateSpot %= this->RATE_SIZE;                     
        this->beatAvg = 0;
        for (byte x = 0; x < this->RATE_SIZE; x++)
          this->beatAvg += this->rates[x];
        this->beatAvg /= this->RATE_SIZE;
      }
    }
  }

  void set_age(int age) {
    this->age = age;
  }

  void set_gender(int gender) {
    this->gender = gender;
  }

  
  void set_thr() {
    if (this->age == 0xFF) {
      Serial.println("Age not set.");
      return;
    }

    this->rhr = get_restingHeartRate_avg();  

    this->thr_bottom = calculate_THR_Bottom();
    this->thr_top = calculate_THR_Top();

    Serial.print("THR Bottom: ");
    Serial.println(this->thr_bottom);
    Serial.print("THR Top: ");
    Serial.println(this->thr_top);
  }



  float get_bpm() {
    return this->beatsPerMinute;
  }

  int get_beatAvg() {
    return this->beatAvg;
  }

  int get_rhr() {
    return this->rhr;
  }

  int get_thr_top() {
    return this->thr_top;
  }

  int get_thr_bottom() {
    return this->thr_bottom;
  }
};