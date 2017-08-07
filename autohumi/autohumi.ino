#include "LiquidCrystal.h"
#include "DHT.h"
#define DHTTYPE DHT11

// analog pins
int aPotiPin = 7;

// digital pins
int dDhtPin = 2;
int dLcdContrastPin = 3;
int dSwitchPin = 4;
int dInletPin = 9;
int dOutletPin = 10;

// Thresholds
int minTemp = 15;
int maxTemp = 35;
int threshTemp = 25;
int threshHumi = 40;

boolean lastState = 0;
unsigned char contrast = 0; // [0 - 255]
int lcdbuf = 16;
int state = 0; // init the run state
int potiVal = 0;
unsigned long lastCheck = 0;
unsigned long time = 0;

DHT dht(dDhtPin, DHTTYPE);
LiquidCrystal lcd(11, 12, 5, 6, 7, 8);


/*
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

char line1[14] = "Temperature:";
char line2[14] = "  25 C";
char line3[14] = "Humidity:";
char line4[14] = "  25%";


//  U8G2_R0  No rotation, landscape
//  U8G2_R1 90 degree clockwise rotation
//  U8G2_R2 180 degree clockwise rotation
//  U8G2_R3 270 degree clockwise rotation
//  U8G2_MIRROR No rotation, landscape, display content is mirrored (v2.6.x)


U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, 19, 18);

void setup(void) {
  u8g2.begin();
  line2[4] = (char)176;
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_9x15_tf);
    //    u8g2.setFont(u8g2_font_7x13_tf);
    //    u8g2.setFont(u8g2_font_crox1t_tf);

    u8g2.drawStr(0, 10, line1);
    u8g2.drawStr(0, 25, line2);
    u8g2.drawStr(0, 49, line3);
    u8g2.drawStr(0, 64, line4);
  } while ( u8g2.nextPage() );
}

void loop(void) {
  
  delay(500);
}
*/





void setup() {
  Serial.begin(9600);
  Serial.println("Initialising automatic humidifier");
  
  // set pin states
  pinMode(dInletPin, OUTPUT);
  pinMode(dOutletPin, OUTPUT);
  pinMode(dLcdContrastPin, OUTPUT);
  pinMode(dSwitchPin, INPUT);
  
  // contrast will be between 0 and 255
  // start the PWM signal with high contrast (low pwm value)
  analogWrite(dLcdContrastPin, contrast);
  dht.begin();
  
  lcd.begin(lcdbuf, 2);
  lcd.print("Auto Humidifier");
  lcd.setCursor(0, 1);
  lcd.print("Initialising...");
  
  delay(2000);
}

void loop() {
  time = millis();
  
  // Check whether the user whishes to switch the state
  boolean buttonPressed = debounceButton();
  if (buttonPressed != lastState) {
    lastState = buttonPressed; 
    if (buttonPressed) {
      state++;
      if (state > 3) state = 0;

      lcd.clear();
      lcd.setCursor(0, 0);
      if (state == 0) {
        lcd.print("Humidity: ");
        lcd.setCursor(0, 1);
        lcd.print("Temp: ");
      } else if (state == 1) {
        lcd.print("Temp. threshold:");
      } else if (state == 2) {
        lcd.print("Humi. threshold:");
      } else if (state == 3) {
        lcd.print("Set Contrast:");
      }
  
      Serial.print("new state: ");
      Serial.println(state);
    }
  }
  
  // if we are in one of the setting states, we read the poti value
  if (state > 0) {
    // potiVal will be [0 - 100]
    Serial.print("\nRetrieving information from poti: ");
    potiVal = readStablePotiValue();
    Serial.print("Poti: ");
    Serial.println(potiVal);
    lcd.setCursor(0, 1);
    switch(state) {
      case 1: // humidity setting state
        threshHumi = potiVal;
        lcd.print(threshHumi);
        break;
      case 2: // temperature setting state
        threshTemp = minTemp + (maxTemp - minTemp) / 100 * potiVal;
        lcd.print(threshTemp);
        break;
      case 3: // contrast setting state
        contrast = potiVal * 2.55;
        lcd.print(contrast);
        analogWrite(dLcdContrastPin, contrast);
        break;
    }
  }
    
  // Normally we only check every ten seconds whether we need to start spraying water
  // or if temperature and humidity are adjusted
  if (state == 1 || state == 2 || abs(time - lastCheck) > 10000) {
    checkAndSpray();
    lastCheck = time;
  }
  
  // we need to check often for the pressed switch button
  delay(50);
}

void checkAndSpray() {
  
//  Serial.print("Read sensor: ");
  //delay(100);
  
  
  float humi = dht.readHumidity();
  float temp = dht.readTemperature();
  if (isnan(humi) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  if (state == 0) {
    lcd.setCursor(12, 0);
    lcd.print(humi);
    lcd.setCursor(12, 1);
    lcd.print(temp);
  }
  float hic = dht.computeHeatIndex(temp, humi, false);

  Serial.print("Humidity (%): ");
  Serial.println(humi);

  Serial.print("Temperature (°C): ");
  Serial.println(temp);
  
  Serial.print("Heat Index (oC): ");
  Serial.println(hic);
  
  if(temp > threshTemp || humi > threshHumi) {
    digitalWrite(dInletPin, HIGH);
  } else {
    digitalWrite(dInletPin, LOW);
  }
}

boolean debounceButton() {
  boolean state;
  boolean previousState = digitalRead(dSwitchPin);
  int i = 0;
  // if we read 10 times the same value we are quite sure the button is stable
  while (i++ < 10) {
    delay(1);
    state = digitalRead(dSwitchPin);
    if (state != previousState) {
      i = 0;
      previousState = state;
    }
  }
  return state;
}

// Returns a value between 0 and 100
int readStablePotiValue() {
  int res = 0;
  int num = 10;
  for(int i = 0; i < num; i++) {
    res += analogRead(aPotiPin);
    delay(1);
  }
  // poti values range from 0 to 1023 therefore the last 10 values correspond to 100
  return res / num / 10.13;
}
