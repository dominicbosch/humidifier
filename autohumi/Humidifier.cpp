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
	_appState = new AppState(_displ);
	_sprayState = new SprayState(_displ);
	
	// Humidity & Temperatur sensor:
	_dht = new DHT(2, DHTTYPE);
	_dht->begin();
	
	delay(2000);

	_displ->clear();
	_displ->printString(3, "Watching...");
}

void Humidifier::loop() {
	unsigned long nowTime = millis();

	bool stateSwitched = _appState->updateState();
	if (stateSwitched) _appState->printStateText(0);

	// We check temperature and humidity only every ten seconds
	if (abs(nowTime - _lastTempCheck) > 10000) {
		_updateTempAndHumi();
		_lastTempCheck = nowTime;
	}

	if (_appState > STATE_RUNNING) {
		// All states except for the run state need to read the poti for a setting value

		// if we are in one of the setting states, we read the poti value
		int _nowPoti = _readStablePotiValue(); // [0 - 100]
		if (_potiHasChanged(stateSwitched)) _updateSettingValue();
	} else if (stateSwitched) {
		// only the run state needs to print the current temp and humidity
		_printTempAndHumi();
	}


  // we check spray state only every half second
  if (abs(nowTime - _lastSprayCheck) > 500) {
		if (_sprayState->update(_nowTemp, _nowHumidity)) {
			// The spray state changed
	    _sprayState->printStateText(3);
		}
  	_sprayState->printCountdown(4);
    _lastSprayCheck = nowTime;
  }

	// we need to check often for the pressed switch button
	delay(50);
}

void Humidifier::_updateTempAndHumi() {
	int humi = _dht->readHumidity();
	int temp = _dht->readTemperature();

	if (isnan(humi) || isnan(temp)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}
	_nowHumidity = humi;
	_nowTemp = temp;
	_printTempAndHumi();
}

void Humidifier::_printTempAndHumi() {
	char *buffer;

	buffer = _displ->getBufferLine(0);
	snprintf(buffer, BUFFER_SIZE, "%3d°C now > %2d°C", _nowTemp, _sprayState->getTempThresh());
	_displ->printBufferLineAsUTF8(0);

	*buffer = _displ->getBufferLine(1);
	snprintf(buffer, BUFFER_SIZE, "%3d%%  now <%3d%%", _nowHumidity, _sprayState->getHumiThresh());
	_displ->printBufferLineAsString(1);
}

bool Humidifier::_potiHasChanged(bool stateSwitched) {
	if (stateSwitched) {
		_initialPotiVal = _nowPoti;
		_potiChanged = false;
	} else if (!_potiChanged) {
		// we add a poti robustness of +/- 5% before we register new values
		if(abs(_nowPoti - _initialPotiVal) > 5) {
			_potiChanged = true;
		} else {
			_nowPoti = _initialPotiVal;
		}
	}
}

void Humidifier::_updateSettingValue() {
	char *buffer = _displ->getBufferLine(1);
	int newVal;
	switch (_appState->getState()) {
		case STATE_SET_TEMP: // temperature setting state
			newVal = _minTemp + (_maxTemp - _minTemp) * _nowPoti / 100;
			_sprayState->setTempThresh(newVal);
			snprintf(buffer, BUFFER_SIZE, "Spray above %3d°C", newVal);
			break;
		case STATE_SET_HUMI: // humidity setting state
			newVal = _nowPoti;
			_sprayState->setHumiThresh(newVal);
			snprintf(buffer, BUFFER_SIZE, "Spray below %3d%%", newVal);
			break;
		case STATE_SET_SPRAYTIME: // Spray Duration Setting [60 - 600] seconds
			newVal = 30 + _nowPoti * 5.7;
			_sprayState->setSprayTime(newVal);
			snprintf(buffer, BUFFER_SIZE, "Spray every %3ds", newVal);
			// printLCD(_sprayTime, " seconds   ");
			break;
		case STATE_SET_TIMER: // set spray timer
			// contrast = _nowPoti * 0.75; // [0 - 75] are reasonable values
			// _sprayState->setTemperatureThreshold(newVal);
			// snprintf(buffer, BUFFER_SIZE, "Spray %3ds", _sprayTime);
			// printLCD(_nowPoti, "%   ");
			// analogWrite(dLcdContrastPin, 75 - contrast);
			break;
	}
	_displ->printBufferLineAsUTF8(1);
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

