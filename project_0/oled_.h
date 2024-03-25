#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class oled {
private:
  byte SCREEN_WIDTH = 128;
  byte SCREEN_HEIGHT = 64;
  byte i2c_addr;

  Adafruit_SSD1306 display = Adafruit_SSD1306(this->SCREEN_WIDTH, this->SCREEN_HEIGHT, &Wire, -1);

public:

  oled(int addr) {
    this->i2c_addr = addr;
  }

  void start() {
    while (!display.begin(SSD1306_SWITCHCAPVCC, this->i2c_addr)) {
      Serial.println(F("SSD1306 allocation failed"));
      // Add additional error handling here (e.g., loop a limited number of times, exit program)
      delay(500);
    }

    display.setTextColor(WHITE);
    display.setTextSize(1);
  }

  void clear() {
    display.clearDisplay();
  }

  void screen_start() {
    display.setCursor(0, 10);
    display.println("HELLO THERE!");
    display.display();
  }

  void screen_bpm(int bpm){
    display.setCursor(1, 10);
    display.setTextSize(2);
    display.print("BPM : ");
    display.println(bpm);
    display.display();
  }
};