#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>
#define button_1 26
#define button_2 18
#define button_3 19
#define button_4 23

#define DEBOUNCE_TIME 50

 ezButton button4(button_4);
 ezButton button3(button_3);
 ezButton button1(button_1);

bool ageSelect = true;

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

  void age_select(int age){
    display.clearDisplay();
    while(ageSelect){
      button4.setDebounceTime(DEBOUNCE_TIME);
      button3.setDebounceTime(DEBOUNCE_TIME);
      button1.setDebounceTime(DEBOUNCE_TIME);
      button4.loop();
      button3.loop();
      button1.loop();
      int btn4State = button4.getState();
      int btn3State = button3.getState();
      int btn1State = button1.getState();
      if (button4.isPressed()) { // Button 1 pressed (add 1)
      age++;
      } else if (button3.isPressed()) { // Button 2 pressed (add 10)
      age += 10;
      }
      if (button1.isPressed()) { // Button 3 pressed (proceed)
        ageSelect = false;
      }

    // Update and display age on OLED
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(2); // Adjust text size as needed
      display.print("Your Age:");
      display.setCursor(0, 16); // Adjust position as needed
      display.print(age);
      display.display();
      }
      return age;
  }

  void screen_bpm(int bpm){
    display.setCursor(1, 10);
    display.setTextSize(2);
    display.print("BPM : ");
    display.println(bpm);
    display.display();
  }

  void display_num(int x, int y, int num){
    this->clear();
    display.setCursor(x, y);
    display.print(num);
    display.display();
  }
};