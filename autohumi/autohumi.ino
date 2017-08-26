#include <LiquidCrystal.h>
#include <DHT.h>
#include <Arduino.h>
#include <SPI.h>
#include <U8x8lib.h>

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
int minTemp = 10;
int maxTemp = 40;
int threshTemp = 25;
int threshHumi = 40;
int sprayTime = 120;
boolean lastState = 0;

// allowed values for contrast would be [0 - 255]
// but since nothing can be seen above 75, we only use [0 - 75]
unsigned char contrast = 75;
int temperature = 0;
int humidity = 0;
int initialPotiVal;
boolean potiChanged;
int appState = 0; // init the app state
int sprayState = 0; // init the spray state

// OLED & LCD line buffer
char oledLine[16] = "";
char lcdLine[16] = "";

// times:
unsigned long nowTime = 0;
unsigned long lastSprayCheck = 0;
unsigned long lastTempCheck = 0;
unsigned long sprayStart = 0;

// Humidity & Temperatur sensor:
DHT dht(dDhtPin, DHTTYPE);

// LCD display: 
LiquidCrystal lcd(11, 12, 5, 6, 7, 8);

// OLED display:
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(19, 18);

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
  analogWrite(dLcdContrastPin, 75-contrast);
  dht.begin();
  
  u8x8.begin();
  u8x8.setFlipMode(1);
  u8x8.setFont(u8x8_font_pxplusibmcgathin_f);
  u8x8.draw2x2String(0, 2, "Starting");
  u8x8.draw2x2String(0, 4, "  up!");
  
  lcd.begin(16, 2);
  lcd.print("Auto Humidifier");
  lcd.setCursor(0, 1);
  lcd.print("Initialising...");
  
  delay(2000);
  
  u8x8.clear();
  lcd.clear();
  lcd.print("Humidity ");
  lcd.setCursor(0, 1);
  lcd.print("Temperature ");
  readTempAndHumi();
  printDHTonLCD();
  printDHTonOLED();
  u8x8.drawString(0, 3, "Watching...");
}

void loop() {
  nowTime = millis();
  boolean stateSwitched = updateState();
  
  // We check temperature and humidity every ten seconds
  if (stateSwitched || abs(nowTime - lastTempCheck) > 10000) {
    readTempAndHumi();
    checkSpray();
    printDHTonLCD();
    printDHTonOLED();
    lastTempCheck = nowTime;

  // we check spray state every half second if we are spraying or flushing
  }
  else if ((sprayState == 1 || sprayState == 2) && abs(nowTime - lastSprayCheck) > 500) {
    checkSpray();
    lastSprayCheck = nowTime;
  }

  // we need to check often for the pressed switch button
  delay(50);
}

boolean updateState() {
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
  
  if (stateSwitched) {
    if (appState == 0) {
      lcd.print("Humidity ");
      lcd.setCursor(0, 1);
      lcd.print("Temperature ");
    }
    else if (appState == 1) {
      lcd.print("Spray above:");
      printLCD(threshTemp, String((char)0xDF)+"C   ");
    }
    else if (appState == 2) {
      lcd.print("Spray below:");
      printLCD(threshHumi, "% humidity  ");
    }
    else if (appState == 3) {
      lcd.print("Set spray time:");
      printLCD(sprayTime, " seconds   ");
    }
    else if (appState == 4) {
      lcd.print("Set Contrast:");
      printLCD(contrast/0.75, "%   ");
    }
  }
  if (appState > 0) {
    // if we are in one of the setting states, we read the poti value
    // potiVal will be [0 - 100]
    int potiVal = readStablePotiValue(); // [0 - 100]

    // we add a poti robustness of +/- 10% before we register new values
    if (stateSwitched) {
      initialPotiVal = potiVal;
      potiChanged = false;
    } else if (!potiChanged) {
      if(abs(potiVal - initialPotiVal) > 5) {
        potiChanged = true;
      } else {
        potiVal = initialPotiVal;
      }
    }
    
    if (potiChanged) {
      switch (appState) {
        case 1: // temperature setting state
          threshTemp = minTemp + (maxTemp - minTemp) * potiVal / 100;
          printLCD(threshTemp, String((char)0xDF)+"C   ");
          //printDHTonOLED();
          break;
        case 2: // humidity setting state
          threshHumi = potiVal;
          printLCD(threshHumi, "% humidity  ");
          //printDHTonOLED();
          break;
        case 3: // Spray Duration Setting [60 - 600] seconds
          sprayTime = 30 + potiVal * 5.7;
          printLCD(sprayTime, " seconds   ");
          break;
        case 4: // contrast setting state
          contrast = potiVal * 0.75; // [0 - 75] are reasonable values
          printLCD(potiVal, "%   ");
          analogWrite(dLcdContrastPin, 75 - contrast);
          break;
        // case 4: // brightness setting state
        //   brightness = potiVal * XYZ; // what are reasonable values?
        //   lcd.print(brigthness);
        //   analogWrite(dLcdBrightnessPin, 255 - contrast);
        //   break;
      }
    }
  }
  return stateSwitched;
}

void readTempAndHumi() {
  int humi = dht.readHumidity();
  int temp = dht.readTemperature();

  if (isnan(humi) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  humidity = humi;
  temperature = temp;
}

void checkSpray() {
  // if we are idle and the thresholds are crossed, we start spraying 
  if (sprayState == 0 && (temperature > threshTemp || humidity < threshHumi)) {
    digitalWrite(dInletPin, HIGH);
    sprayStart = nowTime;
    sprayState = 1;
    Serial.println("New spray state: Spraying!");
    u8x8.drawString(0, 3, "Spraying!  ");
    
  // if we are spraying and the time is over, we stop spraying and flush
  } else if (sprayState == 1 && abs(nowTime - sprayStart)/1000 > sprayTime) {
    digitalWrite(dInletPin, LOW);
    digitalWrite(dOutletPin, HIGH);
    sprayState = 2;
    Serial.println("New spray state: Flushing!");
    u8x8.drawString(0, 3, "Flushing!  ");

  // we flush the pipeline for 30 seconds
  } else if (sprayState == 2 && abs(nowTime - sprayStart)/1000 > (sprayTime+30)) {
    digitalWrite(dOutletPin, LOW);
    sprayState = 0;
    Serial.println("New spray state: Watching!");
    u8x8.drawString(0, 3, "Watching...");
    u8x8.drawString(0, 4, "");
  }
  
  if (sprayState == 1) {
    sprintf(oledLine, "Ends in %3ds", sprayStart/1000 + sprayTime - nowTime/1000);
    u8x8.drawString(0, 4, oledLine);
  } else if (sprayState == 2) {
    sprintf(oledLine, "Ends in %3ds", sprayStart/1000 + (sprayTime+30) - nowTime/1000);
    u8x8.drawString(0, 4, oledLine);
  }
}

void printDHTonLCD() {
  if (appState == 0) {
    lcd.setCursor(12, 0);
    sprintf(lcdLine, "%3d%%  ", humidity);
    lcd.print(lcdLine);
    
    lcd.setCursor(12, 1);
    sprintf(lcdLine, "%2d C  ", temperature);
    lcdLine[2] = (char)0xDF;
    lcd.print(lcdLine);
/*    lcd.print(temperature);
    lcd.print((char)0xDF);
    lcd.print("C  ");*/
  }
}

void printDHTonOLED() {
  sprintf(oledLine, "%3d°C now > %2d°C", temperature, threshTemp);
  u8x8.drawUTF8(0, 0, oledLine);
  sprintf(oledLine, "%3d%%  now <%3d%%", humidity, threshHumi);
  u8x8.drawString(0, 1, oledLine);
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

void printLCD(int val, String unit) {
  lcd.setCursor(0, 1);
  lcd.print(val);
  lcd.print(unit);
}
/*
void drawOLED(int line, char * str) {
  u8x8.drawString(0, line, str);
}
*/
