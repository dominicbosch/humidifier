#include "LiquidCrystal.h"
#include "DHT.h"
#define DHTTYPE DHT11

// analog pins
int aPotiPin = 0;

// digital pins
int dDhtPin = 2;
int dLcdContrastPin = 3;
int dSwitchPin = 4;
int dInletPin = 9;
int dOutletPin = 10;

// Thresholds
int threshTemp = 25;
int threshHumi = 40;

boolean lastState = 0;
int contrast = 0;
int lcdbuf = 16;
int state = 0; // init the run state
int potiVal = 0;
unsigned long lastCheck = 0;
unsigned long time = 0;

DHT dht(dDhtPin, DHTTYPE);
LiquidCrystal lcd(11, 12, 5, 6, 7, 8);

void setup() {
  Serial.begin(9600);
  Serial.println("Initialising automatic humidifier");
  
  pinMode(dInletPin, OUTPUT);
  pinMode(dOutletPin, OUTPUT);
  pinMode(dLcdContrastPin, OUTPUT);
  pinMode(dSwitchPin, INPUT);
  
  // start the PWM signal with high contrast (low pwm value)
  analogWrite(dLcdContrastPin, contrast);
      
  dht.begin();
  
  lcd.begin(lcdbuf, 2);
  lcd.print("Auto Humidifier");
  lcd.setCursor(0, 1);
  lcd.print("Initialising...");
  
  delay(2000);
  lcd.clear();
}

void loop() {
  time = millis();
  
  // Normally we only check every ten seconds whether we need to start spraying water
  if (abs(time - lastCheck) > 10000) {
    checkAndSpray();
    lastCheck = time;
  }
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
        lcd.print("Run state: ");
      } else if (state == 1) {
      lcd.print("Set Temp threshold: ");
      } else if (state == 2) {
      lcd.print("Set Humidity threshold: ");
      } else if (state == 3) {
      lcd.print("Set Contrast threshold: ");
      }
      lcd.setCursor(12, 0);
  
      Serial.print("new state: ");
      Serial.println(state);
    }
  }
  
  // if we are in one of the setting states, we read the poti value
  if (state > 0) potiVal = readStablePotiValue();
  switch(state) {
    case 0: // run state
      break;
    case 1: // humidity setting state
    
      lcd.setCursor(12, 0);
      lcd.print(potiVal);
      break;
    case 2: // temperature setting state
      lcd.setCursor(12, 0);
      lcd.print(potiVal);
      break;
    case 3: // contrast setting state
      lcd.setCursor(12, 0);
      lcd.print(potiVal);
      
      analogWrite(dLcdContrastPin, potiVal);
      break;
  }
  

//  delay(2000);
  // we need to check often for the pressed switch button
  delay(50);
}

boolean debounceButton() {
  boolean state;
  boolean previousState = digitalRead(dSwitchPin);
  int i = 0;
  // if we read 20 times the same value we are quite sure the button is stable
  while (i < 20) {
    delay(1);
    state = digitalRead(dSwitchPin);
    if (state != previousState) {
      i = 0;
      previousState = state;
    }
    i++;
  }
  return state;
}

void checkAndSpray() {
  
  Serial.print("\nRetrieving information from sensor: ");
  Serial.print("Poti: ");
  Serial.println(potiVal);
//  Serial.print("Read sensor: ");
  //delay(100);
  
  
  float humi = dht.readHumidity();
  float temp = dht.readTemperature();
  if (isnan(humi) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  float hic = dht.computeHeatIndex(temp, humi, false);
  /*
  lcd.setCursor(0, 0);
  lcd.print("Humidity: ");
  lcd.setCursor(12, 0);
  lcd.print(humi);
  */
  Serial.print("Humidity (%): ");
  Serial.println(humi);
  /*
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.setCursor(12, 1);
  lcd.print(temp);
  */
  Serial.print("Temperature (°C): ");
  Serial.println(temp);
  
  Serial.print("Heat Index (oC): ");
  Serial.println(hic);
  /*
  if(temp > 26 || humi > 36) {
    digitalWrite(dInletPin, HIGH);
  } else {
    digitalWrite(dInletPin, LOW);
  }*/

}

// Returns a value between 0 and 100
int readStablePotiValue() {
  int res = 0;
  for(int i = 0; i < 10; i++) {
    res += analogRead(aPotiPin);
    delay(1);
  }
  return res / 102.3; // poti values range from 0 to 1023
}
