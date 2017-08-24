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
int sprayTime = 120;
boolean lastState = 0;
// allowed values for contrast would be [0 - 255]
// but since nothing can be seen above 75, we only use [0 - 75]
unsigned char contrast = 0;
int initialPotiVal;
boolean potiChanged;
byte appState = 0; // init the app state
byte sprayState = 0; // init the spray state

// times:
unsigned long time = 0;
unsigned long lastCheck = 0;
unsigned long sprayStart = 0;

DHT dht(dDhtPin, DHTTYPE);
LiquidCrystal lcd(11, 12, 5, 6, 7, 8);

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
  
  lcd.begin(16, 2);
  lcd.print("Auto Humidifier");
  lcd.setCursor(0, 1);
  lcd.print("Initialising...");
  
  delay(2000);
  
  lcd.clear();
  lcd.print("Humidity: ");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  checkAndSpray();
}

void loop() {
  time = millis();
  boolean stateSwitched = false;
  // Check whether the user whishes to switch the state
  boolean buttonPressed = debounceButton();
  if (buttonPressed != lastState) {
    lastState = buttonPressed; 
    if (buttonPressed) {
      stateSwitched = true;
      appState++;
      if (appState > 4) appState = 0;
      lcd.clear();
      Serial.print("new appState: ");
      Serial.println(appState);
    }
  }
  
  lcd.setCursor(0, 0);
  if (appState == 0) {
    lcd.print("Humidity: ");
    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
  } else {
    // if we are in one of the setting states, we read the poti value
    // potiVal will be [0 - 100]
    Serial.print("\nRetrieving information from poti: ");
    
    int potiVal = readStablePotiValue(); // [0 - 100]
    Serial.println(potiVal);

    // we add a poti robustness of +/- 10% before we register new values
    if (stateSwitched) {
      initialPotiVal = potiVal;
      potiChanged = false;
    } else if (!potiChanged) {
      if(abs(potiVal - initialPotiVal) > 10) {
        potiChanged = true;
      } else {
        potiVal = initialPotiVal;
      }
    }
    
    if (appState == 1) {
      lcd.print("Temp. threshold:");
    } else if (appState == 2) {
      lcd.print("Humi. threshold:");
    } else if (appState == 3) {
      lcd.print("Set Contrast:");
    } else if (appState == 4) {
      lcd.print("Set spray time:");
    }
    lcd.setCursor(0, 1);
    switch (appState) {
      case 1: // humidity setting state
        threshHumi = potiVal;
        lcd.print(threshHumi);
        break;
      case 2: // temperature setting state
        threshTemp = minTemp + (maxTemp - minTemp) / 100 * potiVal;
        lcd.print(threshTemp);
        break;
      case 3: // contrast setting state
        contrast = potiVal * 0.75; // [0 - 75] are reasonable values
        lcd.print(contrast);
        analogWrite(dLcdContrastPin, 255 - contrast);
        break;
      case 4: // Spray Duration Setting [60 - 600] seconds
        sprayTime = 60 + potiVal * 5.4;
        lcd.print(sprayTime);
        break;
      // case 4: // brightness setting state
      //   brightness = potiVal * XYZ; // what are reasonable values?
      //   lcd.print(brigthness);
      //   analogWrite(dLcdBrightnessPin, 255 - contrast);
      //   break;
    }
  }
    
  // Normally we only check every ten seconds whether we need to start spraying water
  // or if temperature and humidity are adjusted
  if (stateSwitched || appState == 1 || appState == 2 || abs(time - lastCheck) > 10000) {
    checkAndSpray();
    lastCheck = time;
  }
  
  // we need to check often for the pressed switch button
  delay(50);
}

void checkAndSpray() {
  float humi = dht.readHumidity();
  float temp = dht.readTemperature();
  if (isnan(humi) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  if (appState == 0) {
    lcd.setCursor(12, 0);
    lcd.print(humi);
    lcd.setCursor(12, 1);
    lcd.print(temp);
  }
  // float hic = dht.computeHeatIndex(temp, humi, false);

  Serial.print("Humidity (%): ");
  Serial.println(humi);

  Serial.print("Temperature (°C): ");
  Serial.println(temp);
  
  // Serial.print("Heat Index (oC): ");
  // Serial.println(hic);
  
  // if we are idle and the thresholds are crossed, we start spraying 
  if(sprayState == 0 && (temp > threshTemp || humi > threshHumi)) {
    digitalWrite(dInletPin, HIGH);
    sprayStart = time;
    sprayState = 1;
    Serial.println("New spray state: Spraying!");
  
  // if we are spraying and the time is over, we stop spraying and flush
  } else if (sprayState == 1 && abs(time - sprayStart) > sprayTime * 1000) {
    digitalWrite(dInletPin, LOW);
    digitalWrite(dOutletPin, HIGH);
    sprayState = 2;
    Serial.println("New spray state: Flushing!");

  // we flush the pipeline for 30 seconds
  } else if (sprayState == 2 && abs(time - sprayStart) > (sprayTime+30) * 1000) {
    digitalWrite(dOutletPin, LOW);
    sprayState = 0;
    Serial.println("New spray state: Idle!");
  }
}

// Since the switches vibrate physically when used, we have to wait for an accurate value
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

// the poti value can vary largely, we try to flatten this a bit by
// using an average over 10 measurements
// Returns a value between 0 and 100
int readStablePotiValue() {
  int res = 0;
  int num = 10;
  for(int i = 0; i < num; i++) {
    res += analogRead(aPotiPin);
    delay(1);
  }
  // poti values range from 0 to 1023 therefore the last 10 values correspond to 100
  return 100 - (res / num / 10.13); // we flip the value because of the poti direction (100 - ...)
}
