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
	_dht = new DHT(3, DHTTYPE); // Digital Pin D3
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
		if (_sprayState->getRunMode() == RUNMODE_SENSOR) {
			_appState->printStateText(LINE_STATE, "Sensor Mode!");
		} else {
			_appState->printStateText(LINE_STATE, "Timer Mode!");
		}
		_displ->clearBufferLine(LINE_SETTING);
	}

	if (_sprayState->getRunMode() == RUNMODE_SENSOR) {
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
		_printSettingValue();
	} else if(_sprayState->getRunMode() == RUNMODE_TIMER) {
		char *buffer = _displ->clearAndGetBufferLine(LINE_SETTING);
		snprintf(buffer, BUFFER_LENGTH, "Spray in %3ds", _sprayState->getSprayETA());
		_displ->printBufferLineAsUTF8(LINE_SETTING);
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
	delay(25);
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
	unsigned int newVal;
	switch (_appState->getState()) {
		case STATE_SET_RUNMODE: // run by sensor or timer
			_sprayState->setRunMode((_nowPoti < 50) ? RUNMODE_SENSOR : RUNMODE_TIMER);
			break;
		case STATE_SET_TEMP: // temperature setting state
			newVal = _minTemp + (_maxTemp - _minTemp) * _nowPoti / 100;
			_sprayState->setTempThresh(newVal);
			_printTempAndHumi();
			break;
		case STATE_SET_HUMI: // humidity setting state
			newVal = _nowPoti;
			_sprayState->setHumiThresh(newVal);
			_printTempAndHumi();
			break;
		case STATE_SET_SPRAYTIME: // Spray Duration Setting [60 - 600] seconds
			newVal = 30 + _nowPoti * 5.7;
			_sprayState->setSprayTime(newVal);
			break;
		case STATE_SET_TIMER: // set spray timer
			_nowTimer = _minTimer + (_maxTimer - _minTimer) / 100 * _nowPoti;
			_sprayState->setSprayInterval(_nowTimer);
			break;
	}
}

void Humidifier::_printSettingValue() {
	char *buffer = _displ->clearAndGetBufferLine(LINE_SETTING);
	switch (_appState->getState()) {
		case STATE_SET_RUNMODE:
			if (_sprayState->getRunMode() == RUNMODE_SENSOR) {
				snprintf(buffer, BUFFER_LENGTH, "Sensor Mode");
			} else {
				snprintf(buffer, BUFFER_LENGTH, "Timer Mode");
			}
			break;
		case STATE_SET_TEMP:
			snprintf(buffer, BUFFER_LENGTH, "Spray > %3d°C", _sprayState->getTempThresh());
			break;
		case STATE_SET_HUMI:
			snprintf(buffer, BUFFER_LENGTH, "Spray < %3d%%", _sprayState->getHumiThresh());
			break;
		case STATE_SET_SPRAYTIME:
			snprintf(buffer, BUFFER_LENGTH, "Spray %3ds", _sprayState->getSprayTime());
			break;
		case STATE_SET_TIMER:
			snprintf(buffer, BUFFER_LENGTH, "Spray every %3ds", _sprayState->getSprayInterval());
			break;
	}
	_displ->printBufferLineAsUTF8(LINE_SETTING);
}

// the poti value can vary largely, we try to flatten this a bit by
// using an average over 10 measurements
// Returns a value between 0 and 100
int Humidifier::_readStablePotiValue() {
	unsigned long res = 0;
	byte num = 10;
	for(byte i = 0; i < num; i++) {
		res += analogRead(_aPotiPin);
		delay(1);
	}

	// poti values range from 0 to 1023 therefore the first 10 values correspond to 100 and...
	if ((res / num) < 30) return 100;
	// the last 10 values correspond to 0
	if ((res / num) > 993) return 0;
	return 101 - (res / num) * 100 / 1004;
}

