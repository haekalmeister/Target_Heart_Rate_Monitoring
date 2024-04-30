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
  int rhr = 0;
  int thr_top = 0;
  int thr_bottom = 0;

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

  int get_restingHeartRate()
  {
    // Clear existing data to avoid including previous readings
    clear_data();

    // Start time for the 15-second measurement window
    Serial.println("--BEGIN COUNT RHR--");
    long startTime = millis();

    // Loop for 15 seconds, collecting and averaging heart rate samples
    int totalBeats = 0;
    while (millis() - startTime < 15000)
    {
      get_sensor_value(); // Call the existing run() function to process sensor data
      if (this->beatsPerMinute > 0)
      { // Check if a valid heart rate was detected
        totalBeats++;
      }
      delay(100); // Introduce a small delay between readings
    }

    // Calculate and return the resting heart rate (average beats over 15s)
    if (totalBeats > 0)
    {
      return totalBeats * 4; // Convert total beats to BPM (assuming 15s window)
    }
    else
    {
      return 0; // Return 0 if no heartbeats were detected
    }
  }

  int calculate_THR_Bottom()
  {
    return (50 / 100) * ((220 - this->age) - this->rhr) + this->rhr;
  }

  int calculate_THR_Top()
  {
    return (70 / 100) * ((220 - this->age) - this->rhr) + this->rhr;
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

  // BEGIN PROCESS TO COUNTING THR
  void set_thr()
  {
    if (this->age == 0xFF)
      return;
    Serial.println("--BEGIN COUNT THR--");

    this->rhr = this->get_restingHeartRate();
    this->thr_top = this->calculate_THR_Top();
    this->thr_bottom = this->calculate_THR_Bottom();
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