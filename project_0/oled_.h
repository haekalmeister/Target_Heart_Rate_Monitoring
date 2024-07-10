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
 ezButton button2(button_2);
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

  int gender_select(int gender) {
    display.clearDisplay();
    bool genderSelect = true;

    while (genderSelect) {
        button4.setDebounceTime(DEBOUNCE_TIME); 
        button1.setDebounceTime(DEBOUNCE_TIME); 
        button4.loop();
        button1.loop();
        int btn4State = button4.getState();
        int btn1State = button1.getState();

        if (button4.isPressed()) { 
            gender = (gender == 0) ? 1 : 0;
        }
        if (button1.isPressed()) { 
            genderSelect = false;
        }

        
        display.clearDisplay();
        if (gender == 0) {
            display_male();
        } else {
            display_female();
        }
    }
    this->calculating();
    return gender;
}

  int age_select(int age){
    display.clearDisplay();
    while(ageSelect){
      button4.setDebounceTime(DEBOUNCE_TIME);
      button3.setDebounceTime(DEBOUNCE_TIME);
      button2.setDebounceTime(DEBOUNCE_TIME);
      button1.setDebounceTime(DEBOUNCE_TIME);
      button4.loop();
      button3.loop();
      button2.loop();
      button1.loop();
      int btn4State = button4.getState();
      int btn3State = button3.getState();
      int btn2State = button2.getState();
      int btn1State = button1.getState();
      if (button4.isPressed()) {
      age++;
      } else if (button3.isPressed()) { 
      age += 10;
      } else if (button2.isPressed()){
        ESP.restart();
      }
      if (button1.isPressed()) { 
        ageSelect = false;
      }

    
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(2); 
      display.print("Your Age:");
      display.setCursor(0, 16); 
      display.print(age);
      display.display();
      }
      this->calculating();
      return age;
  }

void screen_bpm(int bpm, int thr_bottom, int thr_top, unsigned long elapsedTime, bool status) {
    display.clearDisplay();  

   
    unsigned int minutes = elapsedTime / 60;
    unsigned int seconds = elapsedTime % 60;

    // Display BPM
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.print("BPM: ");
    display.println(bpm);

    // Display THR Zone
    display.setCursor(0, 20);  
    display.setTextSize(1);
    display.print("THR: ");
    display.print(thr_bottom);
    display.print(" - ");
    display.println(thr_top);

    // Display elapsed time
    if(status == true){
    display.setCursor(0, 40);  
    display.setTextSize(1.5);
    display.print(minutes);
    display.print("m ");
    display.print(seconds);
    display.println("s");
    display.print("Vibration ON");
    }else{
    display.setCursor(0, 40);  
    display.setTextSize(1.5);
    display.print(minutes);
    display.print("m ");
    display.print(seconds);
    display.println("s");
    display.print("Vibration OFF");
    }
    

    display.display();
}

  void display_num(int x, int y, int num){
    this->clear();
    display.setCursor(x, y);
    display.print(num);
    display.display();
  }

void display_thr(int thr, int thr_bottom, int thr_top) {
    display.clearDisplay();  
    
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("RHR: ");
    display.print(thr); 

    display.setCursor(0, 20);
    display.setTextSize(1);  
    display.print("THR ZONE:");

    display.setCursor(0, 40); 
    display.setTextSize(2);    
    display.print(thr_bottom);
    display.print(" - ");
    display.print(thr_top);

    display.display();
}

  void calculating(){
    this->clear();
    display.setCursor(5,10);
    display.setTextSize(2);
    display.print("MEASURING\n30 sec...");
    display.display();
  }

  void display_male(){
    this->clear();
    display.setCursor(1, 10);
    display.setTextSize(2);
    display.print("> Male\n");
    display.print("Female");
    display.display();
  }
  void display_female(){
    this->clear();
    display.setCursor(1, 10);
    display.setTextSize(2);
    display.print("Male\n");
    display.print("> Female");
    display.display();
  }
  void display_vibrationstatus(bool status){
    this->clear();
    display.setCursor(1,10);
    display.setTextSize(2);
    if (status == true){
      display.print("Vibration ON");
    }
    else{
      display.print("Vibration OFF");
    }
    display.display();
  }
};