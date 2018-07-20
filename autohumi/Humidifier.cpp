/*
  Humidifier.cpp - Humidifier control
*/
#include "Arduino.h"
#include <DHT.h>
#include "Humidifier.h"
#include "Display.h"
#include "InputSwitch.h"
#include "OutputSwitch.h"

Humidifier::Humidifier(int aPotiPin, int dDhtPin, int dSwitchPin, int dInletPin,
      int dOutletPin, int dOledClockPin, int dOledDataPin, int minTemp,
      int maxTemp, int threshTemp, int threshHumi, int sprayTime) {

  // Humidity & Temperatur sensor:
  _displ = new Display(dOledClockPin, dOledDataPin);
  _toggle = new InputSwitch(dSwitchPin);
  _inletSwitch = new OutputSwitch(dInletPin);
  _outletSwitch = new OutputSwitch(dOutletPin);
  
  
  _dht = new DHT(dDhtPin, DHTTYPE);
  _dht->begin();
  
  delay(2000);
  _displ->clear();

  updateMeasurements();
  _displ->writeLine(3, "Watching...");
};

Humidifier::loop() {
  nowTime = millis();
  bool stateSwitched = updateState();

  if (stateSwitched) printNewState();
  // All states except for the run state need to read the poti
  if (appState > STATE_RUNNING) updatePotiSetting();
  
  // We check temperature and humidity every ten seconds
  if (stateSwitched || abs(nowTime - lastTempCheck) > 10000) {
    updateMeasurements();
    checkSpray();
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


// Check whether the state button has been pressed
bool updateState() {
  bool stateSwitched = false;
  // Check whether the user whishes to switch the state
  bool buttonState = _toggle.getState();
  if (buttonState != lastState) {
    lastState = buttonState; 
    if (buttonState == 1) { // button is pressed
      stateSwitched = true;
      appState++;
      if (appState > MAX_STATE) appState = 0;
      Serial.print("new appState: ");
      Serial.println(appState);
    }
  }
  return stateSwitched;
}

void printNewState() {
  switch (appState) {
    case STATE_RUNNING: u8x8.drawString(0, 0, "Humidifier Active!"); break;
    case STATE_SET_RUNMODE: u8x8.drawString(0, 0, "Set Run Mode!"); break;
    case STATE_SET_TEMP: u8x8.drawString(0, 0, "Set Temperature!"); break;
    case STATE_SET_HUMI: u8x8.drawString(0, 0, "Set Humidity!"); break;
    case STATE_SET_SPRAYTIME: u8x8.drawString(0, 0, "Set Spray time!"); break;
    case STATE_SET_TIMER: u8x8.drawString(0, 0, "Set timer!"); break;
  }

  else if (appState == STATE_SET_TEMP) {
    lcd.print("Spray above:");
    printLCD(threshTemp, String((char)0xDF)+"C   ");
  }
  else if (appState == STATE_SET_HUMI) {
    lcd.print("Spray below:");
    printLCD(threshHumi, "% humidity  ");
  }
  else if (appState == STATE_SET_SPRAYTIME) {
    lcd.print("Set spray time:");
    printLCD(sprayTime, " seconds   ");
  }
  else if (appState == STATE_SET_TIMER) {
    lcd.print("Set Contrast:");
    printLCD(contrast/0.75, "%   ");
  }
}

void updatePotiSetting() {
  // if we are in one of the setting states, we read the poti value
  // potiVal will be [0 - 100]
  int potiVal = readStablePotiValue(); // [0 - 100]
  bool potiChanged;
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
    }
  }
}

void readTempAndHumi() {
  int humi = _dht.readHumidity();
  int temp = _dht.readTemperature();

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

void updateMeasurements() {
  readTempAndHumi();
  printDHTonOLED();
}

void printDHTonOLED() {
  char buffer[16] = "";
  snprintf(buffer, "%3d°C now > %2d°C", temperature, threshTemp);
  _displ.writeUTF8(0, buffer);
  //_u8x8->drawUTF8(0, 0, oledLine);
  snprintf(buffer, "%3d%%  now <%3d%%", humidity, threshHumi);
  _displ.writeString(buffer)
  // _u8x8->drawString(0, 1, oledLine);
}
