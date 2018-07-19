#include <Arduino.h>
#include <SPI.h>
#include "Humidifier.h"

Humidifier *app;

void setup() {
  Serial.begin(9600);
  Serial.println("Initialising automatic humidifier");
  
  app = new Humidifier();
}

void loop() {
  app.loop();
}

