#ifndef MOVING_SAND_H
#define MOVING_SAND_H

#include <Adafruit_SSD1306.h>
#include <MPU6050.h>
#include "DLDProject.h"

// Function declarations
void setupMovingSand();
void loopMovingSand();

// External references to global objects
extern Adafruit_SSD1306 display;
extern MPU6050 mpu;

#endif