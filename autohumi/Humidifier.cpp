// s#include <SPI.h>
#include <DHT.h>
#include "Humidifier.h"
#include "AppState.h"
#include "SprayState.h"
#include "Display.h"

#define DHTTYPE DHT11

Humidifier::Humidifier() {
	Serial.begin(9600);
	Serial.println("Initialising automatic humidifier");

	_displ = new Display();
	_appState = new AppState();
	_sprayState = new SprayState();
	
	// Humidity & Temperatur sensor:
	_dht = new DHT(2, DHTTYPE);
	_dht->begin();
	
	delay(2000);

	_displ->clear();
	_displ->writeString(3, "Watching...");
}

void Humidifier::loop() {
	_updateTempAndHumi();

	bool stateSwitched = _appState->updateState();
	if (stateSwitched) _displ->writeString(0, _appState->getStateText());

	// All states except for the run state need to read the poti for a setting value
	if (_appState > STATE_RUNNING) _updateSettingValue();

	if (stateSwitched) {
		// the app state switched
	// // TODO figures need to be printed when state switched
	// char buffer[16] = "";
	// snprintf(buffer, "%3d°C now > %2d°C", _nowTemp, _threshTemp);
	// _displ.writeUTF8(0, buffer);
	// //_u8x8->drawUTF8(0, 0, oledLine);
	// snprintf(buffer, "%3d%%  now <%3d%%", _nowHumidity, _threshHumi);
	// _displ.writeString(1, buffer)
	// // _u8x8->writeString(0, 1, oledLine);
	}

	if (_sprayState->update(_nowTemp, _nowHumidity)) {
		// The spray state changed
      _displ->writeString(3, _sprayState->getStateText());
      _displ->writeString(4, "");
			String buffer[16] = "";

	    if (_sprayState == SPRAY_STATE_SPRAYING) {
	      sprintf(buffer, "Ends in %3ds", _sprayStart/1000 + _sprayTime - nowTime/1000);
	      _displ->writeString(4, buffer);
	    } else if (_sprayState == SPRAY_STATE_FLUSHING) {
	      sprintf(buffer, "Ends in %3ds", _sprayStart/1000 + (_sprayTime+30) - nowTime/1000);
	      _displ->writeString(4, buffer);
	    }
	}

	// we need to check often for the pressed switch button
	delay(50);
}

void Humidifier::_updateTempAndHumi() {
	int nowTime = millis();

	// We check temperature and humidity every ten seconds
	if (abs(nowTime - _lastTempCheck) > 10000) {
		int humi = _dht.readHumidity();
		int temp = _dht.readTemperature();

		if (isnan(humi) || isnan(temp)) {
			Serial.println("Failed to read from DHT sensor!");
			return;
		}
		_nowHumidity = humi;
		_nowTemp = temp;
		_lastTempCheck = nowTime;
	}
}

void Humidifier::_updateSettingValue() {
	// if we are in one of the setting states, we read the poti value
	// potiVal will be [0 - 100]
	int potiVal = readStablePotiValue(); // [0 - 100]
	bool potiChanged;
	int newVal;
	// we add a poti robustness of +/- 10% before we register new values

	//FIXME why do we need to check stateSwitched here?
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
		String buffer[16] = "";
		switch (_appState) {
			case STATE_SET_TEMP: // temperature setting state
				newVal = _minTemp + (_maxTemp - _minTemp) * potiVal / 100;
				_sprayState->setTemperatureThreshold(newVal);
				snprintf(buffer, "Spray above %3d°C", newVal);
				break;
			case STATE_SET_HUMI: // humidity setting state
				newVal = potiVal;
				_sprayState->setHumidityThreshold(newVal);
				snprintf(buffer, "Spray below %3d%%", newVal);
				break;
			case STATE_SET_SPRAYTIME: // Spray Duration Setting [60 - 600] seconds
				newVal = 30 + potiVal * 5.7;
				_sprayState->setSprayTime(newVal);
				snprintf(buffer, "Spray every %3ds", newVal);
				// printLCD(_sprayTime, " seconds   ");
				break;
			case STATE_SET_TIMER: // set spray timer
				// contrast = potiVal * 0.75; // [0 - 75] are reasonable values
				// _sprayState->setTemperatureThreshold(newVal);
				// snprintf(buffer, "Spray %3ds", _sprayTime);
				// printLCD(potiVal, "%   ");
				// analogWrite(dLcdContrastPin, 75 - contrast);
				break;
		}
		_displ->drawUTF8(1, buffer);
	}
}

// the poti value can vary largely, we try to flatten this a bit by
// using an average over 10 measurements
// Returns a value between 0 and 100
int Humidifier::_readStablePotiValue() {
	int res = 0;
	int num = 10;
	for(int i = 0; i < num; i++) {
		res += analogRead(_aPotiPin);
		delay(1);
	}
	// poti values range from 0 to 1023 therefore the last 10 values correspond to 100
	return 100 - (res / num / 10.13); // we flip the value because of the poti direction (100 - ...)
}

