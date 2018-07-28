#include <Arduino.h>
#include "Humidifier.h"

Humidifier *app;

void setup() {
	app = new Humidifier();
}

void loop() {
	app.loop();
}

