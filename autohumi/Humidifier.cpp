// s#include <SPI.h>
#include <DHT.h>
#include "Humidifier.h"
#include "AppState.h"
#include "SprayState.h"
#include "Display.h"

#define UPDATE_SPRAY 100
#define UPDATE_SENSOR 10000

Humidifier::Humidifier() {

	_displ = new Display();
	_appState = new AppState(_displ);
	_sprayState = new SprayState(_displ);
	
	// Humidity & Temperatur sensor:
	_dht = new DHT(2, DHTTYPE); // Digital Pin D2
	_dht->begin();
	
	delay(2000);

	_displ->clear();
	_updateTempAndHumi();
	_sprayState->printStateText(LINE_SPRAYSTATE);
	_appState->printStateText(LINE_STATE);
}

void Humidifier::loop() {
	unsigned long nowTime = millis();

	bool stateSwitched = _appState->updateState();
	if (stateSwitched) {
		if (_runMode == RUNMODE_SENSOR) {
			_appState->printStateText(LINE_STATE, "Sensor Mode...");
		} else {
			_appState->printStateText(LINE_STATE, "Timer Mode...");
		}
		_displ->clearBufferLine(LINE_SETTING);
	}

	if (_runMode == RUNMODE_SENSOR) {
		// We check temperature and humidity only every ten seconds
		if (abs(nowTime - _lastTempCheck) > UPDATE_SENSOR) {
			_updateTempAndHumi();
			_lastTempCheck = nowTime;
		}
	}

	if (_appState->getState() > STATE_RUNNING) {
		// All states except for the run state need to read the poti for a setting value
		_nowPoti = _readStablePotiValue(); // [0 - 100]
		_checkIfPotiChanged(stateSwitched);
		if (_potiChanged) _updateSettingValue();
	} else if (stateSwitched) {
		// only the run state needs to print the current temp and humidity
		_printTempAndHumi();
	}


	// we check spray state only every half second
	if (abs(nowTime - _lastSprayCheck) > UPDATE_SPRAY) {
		if (_sprayState->update(_nowTemp, _nowHumidity)) {
			// The spray state changed
			_sprayState->printStateText(LINE_SPRAYSTATE);
		}
	_sprayState->printCountdown(LINE_SPRAYCOUNT);
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

	buffer = _displ->clearAndGetBufferLine(LINE_TEMP);
	snprintf(buffer, BUFFER_LENGTH, "%3d°C >  %2d°C", _nowTemp, _sprayState->getTempThresh());
	_displ->printBufferLineAsUTF8(LINE_TEMP);

	buffer = _displ->clearAndGetBufferLine(LINE_HUMI);
	snprintf(buffer, BUFFER_LENGTH, "%3d%%  < %3d%%", _nowHumidity, _sprayState->getHumiThresh());
	_displ->printBufferLineAsString(LINE_HUMI);
}

void Humidifier::_checkIfPotiChanged(bool stateSwitched) {
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
	Serial.println("Updating state");
	char *buffer = _displ->clearAndGetBufferLine(LINE_SETTING);
	int newVal;

	switch (_appState->getState()) {
		case STATE_SET_TEMP: // temperature setting state
			newVal = _minTemp + (_maxTemp - _minTemp) * _nowPoti / 100;
			_sprayState->setTempThresh(newVal);
			snprintf(buffer, BUFFER_LENGTH, "Spray > %3d°C", newVal);
			_printTempAndHumi();
			break;
		case STATE_SET_HUMI: // humidity setting state
			newVal = _nowPoti;
			_sprayState->setHumiThresh(newVal);
			snprintf(buffer, BUFFER_LENGTH, "Spray < %3d%%", newVal);
			_printTempAndHumi();
			break;
		case STATE_SET_SPRAYTIME: // Spray Duration Setting [60 - 600] seconds
			newVal = 30 + _nowPoti * 5.7;
			_sprayState->setSprayTime(newVal);
			snprintf(buffer, BUFFER_LENGTH, "Spray %3ds", newVal);
			// printLCD(_sprayTime, " seconds   ");
			break;
		case STATE_SET_RUNMODE: // run by sensor or timer
			_runMode = (_nowPoti < 50) ? 0 : 1;
			if (_nowPoti < 50) {
				_runMode = 0;
				snprintf(buffer, BUFFER_LENGTH, "Sensor Mode");
			} else {
				_runMode = 1;
				snprintf(buffer, BUFFER_LENGTH, "Timer Mode");
			}
			_sprayState->setRunMode(_runMode);
			break;
		case STATE_SET_TIMER: // set spray timer
			_nowTimer = _minTimer + (_maxTimer - _minTimer) * _nowPoti / 100;
			snprintf(buffer, BUFFER_LENGTH, "Spray every %3ds", _nowTimer);
			_sprayState->setSprayInterval(_nowTimer);
			break;
	}
	_displ->printBufferLineAsUTF8(LINE_SETTING);
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

