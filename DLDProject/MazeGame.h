#ifndef MAZE_GAME_H
#define MAZE_GAME_H

#include <Adafruit_SSD1306.h>
#include <MPU6050.h>
#include "DLDProject.h"

// Function declarations with unique maze-specific names
void setupMazeGame();
void loopMazeGame();
bool mazeCheckCollision(int x, int y); // Renamed!
void drawMaze();
void mazeDrawPlayer(int x, int y); // Renamed!

// External references to global objects
extern Adafruit_SSD1306 display;
extern MPU6050 mpu;

// External references to shared variables
extern int playerX;
extern int playerY;
extern float playerVX;
extern float playerVY;
extern bool isGameOn;

#endif