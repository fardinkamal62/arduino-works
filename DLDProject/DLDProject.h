#ifndef GAME_COMMON_H
#define GAME_COMMON_H

// Define the game modes
enum GameMode {
    MODE_MENU,
    MODE_SAND,
    MODE_MAZE,
    MODE_CREDIT
};

extern GameMode currentMode;

#endif