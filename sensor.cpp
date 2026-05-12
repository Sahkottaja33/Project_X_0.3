#include "sensor.h"

void sensorInit(){
	pinMode(SENSOR_PIN, INPUT);
}

bool sensor_ballDetected() {
  int val = analogRead(SENSOR_PIN);
  return val < SENSOR_TRESHOLD;
}// palauttaa true, jos paluuarvo alittaa kynnyksen


int sensor_getRawValue(){
	return analogRead(SENSOR_PIN); // mahdollinen debuggaus serial monitorilla, kynnyksen tarkastus
}
