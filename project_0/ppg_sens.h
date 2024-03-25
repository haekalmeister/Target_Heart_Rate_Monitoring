#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

class ppg_data {
private:
  byte RATE_SIZE;  //Increase this for more averaging. 4 is good.
  byte* rates;     //Array of heart rates
  byte rateSpot = 0;
  long lastBeat = 0;  //Time at which the last beat occurred
  float beatsPerMinute;
  int beatAvg;

public:
  MAX30105 particleSensor;

  ppg_data(int rate_size)  //constructor
  {
    this->RATE_SIZE = rate_size;
    this->rates = new byte[rate_size];

    for (int i = 0; i < rate_size; i++)
      this->rates[i] = 0;
  }

  void start() {
    while (!particleSensor.begin(Wire, I2C_SPEED_FAST))  //Use default I2C port, 400kHz speed
    {
      Serial.println("Put your Finger on Sensor ");
      delay(500);
    }

    particleSensor.setup();                     //Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x0A);  //Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0);   //Turn off Green LED

    Serial.println("PPG SENSOR START");
  }

  void run() {
    long irValue = particleSensor.getIR();
    if (checkForBeat(irValue) == true) {
      //We sensed a beat!
      long delta = millis() - lastBeat;
      lastBeat = millis();

      this->beatsPerMinute = 60 / (delta / 1000.0);

      if (this->beatsPerMinute < 255 && this->beatsPerMinute > 20) {
        rates[this->rateSpot++] = (byte)this->beatsPerMinute;  //Store this reading in the array
        this->rateSpot %= this->RATE_SIZE;                     //Wrap variable

        //Take average of readings
        this->beatAvg = 0;
        for (byte x = 0; x < this->RATE_SIZE; x++)
          this->beatAvg += this->rates[x];
        this->beatAvg /= this->RATE_SIZE;
      }
    }
  }

  float get_bpm() {
    return this->beatsPerMinute;
  }

  int get_beatAvg() {
    return this->beatAvg;
  }
};