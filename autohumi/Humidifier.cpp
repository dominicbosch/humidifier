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

	_updateMeasurements();
	
	_displ->clear();
	_displ->writeString(3, "Watching...");
}

void Humidifier::loop() {
	int nowTime = millis();
	bool stateSwitched = _appState->updateState();

	if (stateSwitched) _displ->drawString(0, _appState->getStateText());

	// All states except for the run state need to read the poti
	if (_appState > STATE_RUNNING) _updateStateValue();
	
	// We check temperature and humidity every ten seconds
	if (stateSwitched || abs(nowTime - _lastTempCheck) > 10000) {
		_updateMeasurements();
		sprayState->update(_temperature, _humidity);
		_lastTempCheck = nowTime;

	// we check spray state every half second if we are spraying or flushing
	}
	else if (weShouldCheckSprayState()) {
		sprayState->update(_temperature, _humidity);
		_lastSprayCheck = nowTime;
	}

	// we need to check often for the pressed switch button
	delay(50);
}

void Humidifier::_updateMeasurements() {
	_readTempAndHumi();
	_printDHTonOLED();
}

void Humidifier::_readTempAndHumi() {
	int humi = _dht.readHumidity();
	int temp = _dht.readTemperature();

	if (isnan(humi) || isnan(temp)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}
	_humidity = humi;
	_temperature = temp;
}

void Humidifier::_printDHTonOLED() {
	char buffer[16] = "";
	snprintf(buffer, "%3d°C now > %2d°C", _temperature, _threshTemp);
	_displ.writeUTF8(0, buffer);
	//_u8x8->drawUTF8(0, 0, oledLine);
	snprintf(buffer, "%3d%%  now <%3d%%", _humidity, _threshHumi);
	_displ.writeString(1, buffer)
	// _u8x8->drawString(0, 1, oledLine);
}

// printVariable(int row, String form, int val) {
// 	char buffer[16] = "";
// 	snprintf(buffer, form, val);
// 	_displ.writeString(row, buffer);
// }


void Humidifier::_updateStateValue() {
	// if we are in one of the setting states, we read the poti value
	// potiVal will be [0 - 100]
	int potiVal = readStablePotiValue(); // [0 - 100]
	bool potiChanged;
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
		switch (_appState) {
			case STATE_SET_RUNMODE: // temperature setting state
				_threshTemp = _minTemp + (_maxTemp - _minTemp) * potiVal / 100;
				// printLCD(threshTemp, String((char)0xDF)+"C   ");
				//printDHTonOLED();
				break;
			case STATE_SET_TEMP: // humidity setting state
				_threshHumi = potiVal;
				// displ->printLCD(_threshHumi, "% humidity  ");
				//printDHTonOLED();
				break;
			case STATE_SET_HUMI: // Spray Duration Setting [60 - 600] seconds
				_sprayTime = 30 + potiVal * 5.7;
				// printLCD(_sprayTime, " seconds   ");
				break;
			case STATE_SET_SPRAYTIME: // contrast setting state
				// contrast = potiVal * 0.75; // [0 - 75] are reasonable values
				// printLCD(potiVal, "%   ");
				// analogWrite(dLcdContrastPin, 75 - contrast);
				break;
		}
	}

		//   printLCD(_threshTemp, String((char)0xDF)+"C   ");
	 String buffer[16] = "";
		snprintf(buffer, "Spray above %3d°C", _threshTemp);
	snprintf(buffer, "Spray below %3d%%", _threshHumi);
	snprintf(buffer, "Spray %3ds", _sprayTime);
	snprintf(buffer, "Spray every %3ds", _sprayInterval);

	_displ->drawUTF8(1, buffer);

	if (_sprayState == SPRAY_STATE_SPRAYING) {
		sprintf(oledLine, "Ends in %3ds", _sprayStart/1000 + _sprayTime - nowTime/1000);
		_displ->drawString(4, oledLine);
	} else if (_sprayState == SPRAY_STATE_FLUSHING) {
		sprintf(oledLine, "Ends in %3ds", _sprayStart/1000 + (_sprayTime+30) - nowTime/1000);
		_displ->drawString(4, oledLine);
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

