#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

#define SENSOR_TRESHOLD 2500 // esim, voi olla muutakin
#define SENSOR_PIN 1

void sensorInit();
bool sensor_ballDetected();
int sensor_getRawValue();

#endif
