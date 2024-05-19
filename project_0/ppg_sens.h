#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
class ppg_data
{
private:
  // -- INTERNAL OBJECT PROPERTIES -- //
  byte RATE_SIZE; // Increase this for more averaging. 4 is good.
  byte *rates;    // Array of heart rates
  byte rateSpot = 0;
  long lastBeat = 0; // Time at which the last beat occurred
  float beatsPerMinute;
  int beatAvg;
  int age = 0xFF;
  int gender = 0;
  int rhr = 0;
  int thr_top = 0;
  int thr_bottom = 0;
  int hrmax = 0;

  // -- INTERNAL PRIVATE METHOD -- //
  void clear_data()
  {
    // Reset variables used for storing and averaging heart rate data
    for (byte x = 0; x < RATE_SIZE; x++)
    {
      rates[x] = 0;
    }
    rateSpot = 0;
    beatAvg = 0;
  }

int get_restingHeartRate() {
  const unsigned long measurementDuration = 15000; // 15 seconds
  const unsigned long startTime = millis();

  // Reset variables for data collection
  int totalBeats = 0;
  int validBeatsSum = 0;

  Serial.println("Measuring resting heart rate...");

  // Collect heart rate data for the specified duration
  while (millis() - startTime < measurementDuration) {
    get_sensor_value(); // Process sensor data to detect heart rate
    int currentBPM = this->beatAvg;

    // Print current heart rate reading for debugging
    Serial.print("Current BPM: ");
    Serial.println(currentBPM);

    // Check if current heart rate is valid (above 20 BPM)
    if (currentBPM > 20) {
      totalBeats++;
      validBeatsSum += currentBPM;
    }
  }

  // Calculate resting heart rate based on valid heart rate readings
  if (totalBeats > 0) {
    int restingBPM = validBeatsSum / totalBeats; // Calculate average BPM
    Serial.print("Total Valid Heartbeats: ");
    Serial.println(totalBeats);
    Serial.print("Resting Heart Rate (BPM): ");
    Serial.println(restingBPM);
    return restingBPM;
  } else {
    Serial.println("No valid heartbeats detected during measurement.");
    return 0; // Return 0 if no valid heart rate readings were obtained
  }
}

int calculate_THR_Bottom() {
  if (this->gender == 0){
    hrmax = 206.9 - (0.67 * this->age);
    return (hrmax - this->rhr) * 0.5 + this->rhr;
  }else{
    hrmax = 206 - (0.88 * this->age);
    return (hrmax - this->rhr) * 0.5 + this->rhr;
  }
}

int calculate_THR_Top() {
  if (this->gender == 0){
    hrmax = 206.9 - (0.67 * this->age);
    return (hrmax - this->rhr) * 0.7 + this->rhr;
  }else{
    hrmax = 206 - (0.88 * this->age);
    return (hrmax - this->rhr) * 0.7 + this->rhr;
  }
}


public:
  MAX30105 particleSensor;

  ppg_data(int rate_size) // constructor
  {
    this->RATE_SIZE = rate_size;
    this->rates = new byte[rate_size];

    for (int i = 0; i < rate_size; i++)
      this->rates[i] = 0;
  }

  void start()
  {
    while (!particleSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
    {
      Serial.println("Put your Finger on Sensor ");
      delay(500);
    }

    particleSensor.setup();                    // Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0);  // Turn off Green LED

    Serial.println("PPG SENSOR START");
  }

  void get_sensor_value()
  {
    long irValue = particleSensor.getIR();
    if (checkForBeat(irValue) == true)
    {
      // We sensed a beat!
      long delta = millis() - lastBeat;
      lastBeat = millis();

      this->beatsPerMinute = 60 / (delta / 1000.0);

      if (this->beatsPerMinute < 255 && this->beatsPerMinute > 20)
      {
        rates[this->rateSpot++] = (byte)this->beatsPerMinute; // Store this reading in the array
        this->rateSpot %= this->RATE_SIZE;                    // Wrap variable

        // Take average of readings
        this->beatAvg = 0;
        for (byte x = 0; x < this->RATE_SIZE; x++)
          this->beatAvg += this->rates[x];
        this->beatAvg /= this->RATE_SIZE;
      }
    }
  }

  void set_age(int age)
  {
    this->age = age;
  }

  void set_gender(int gender){
    this->gender = gender;
  }

  // BEGIN PROCESS TO COUNTING THR
  void set_thr() {
  if (this->age == 0xFF) {
    Serial.println("Age not set.");
    return; // Exit if age is not set
  }

  this->rhr = get_restingHeartRate(); // Calculate and set resting heart rate

  // Calculate and set THR values based on age and resting heart rate
  this->thr_bottom = calculate_THR_Bottom();
  this->thr_top = calculate_THR_Top();

  Serial.print("THR Bottom: ");
  Serial.println(this->thr_bottom);
  Serial.print("THR Top: ");
  Serial.println(this->thr_top);
}



  float get_bpm()
  {
    return this->beatsPerMinute;
  }

  int get_beatAvg()
  {
    return this->beatAvg;
  }

  int get_rhr()
  {
    return this->rhr;
  }

  int get_thr_top()
  {
    return this->thr_top;
  }

  int get_thr_bottom()
  {
    return this->thr_bottom;
  }
};