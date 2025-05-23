#include "MazeGame.h"

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// Use the shared variables (don't redefine them)
// float playerVX, playerVY are declared as extern in the header

#define MAX_PLAYER_VELOCITY 5.4f
#define PLAYER_SPEED 2.0f
#define PLAYER_DAMPING 0.95f

void setupMazeGame() {
    display.clearDisplay();
    display.display();
    delay(100);

    // Display heading and subheading
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 0);
    display.println(F("Maze Game"));
    display.setTextSize(1);
    display.setCursor(SCREEN_HEIGHT / 2, 20);
    display.println(F("by Fardin"));
    display.display();
    delay(2000);

    // Reset player position for maze game
    playerX = 5;
    playerY = 35;
    playerVX = 0;
    playerVY = 0;

    drawMaze();
    mazeDrawPlayer(playerX, playerY);
    delay(2000);
}

void loopMazeGame() {
    isGameOn = true;
    // Read accelerometer data
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    // Convert accelerometer data to movement values
    float accelX = constrain(ay / 16384.0f, -1.0f, 1.0f);
    float accelY = -constrain(ax / 16384.0f, -1.0f, 1.0f);

    // Update player velocity
    playerVX = playerVX * PLAYER_DAMPING + (accelX * PLAYER_SPEED);
    playerVY = playerVY * PLAYER_DAMPING + (accelY * PLAYER_SPEED);

    // Limit velocity
    playerVX = constrain(playerVX, -MAX_PLAYER_VELOCITY, MAX_PLAYER_VELOCITY);
    playerVY = constrain(playerVY, -MAX_PLAYER_VELOCITY, MAX_PLAYER_VELOCITY);

    // Update player position
    int newPlayerX = playerX + (int)playerVX;
    int newPlayerY = playerY + (int)playerVY;

    // Check for collisions with maze walls
    if (!mazeCheckCollision(newPlayerX, newPlayerY)) {
        playerX = newPlayerX;
        playerY = newPlayerY;
    }

    // Boundary checks
    if (playerX < 0) playerX = 0;
    if (playerX >= SCREEN_WIDTH) playerX = SCREEN_WIDTH - 1;
    if (playerY < 0) playerY = 0;
    if (playerY >= SCREEN_HEIGHT) playerY = SCREEN_HEIGHT - 1;

    // Check if player reached the exit
    if (playerX >= SCREEN_WIDTH - 1 && playerY >= 50 && playerY <= 60) {
        isGameOn = false;
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(10, 20);
        display.println(F("You Win!"));
        display.display();
        delay(2000);
        playerX = 5; // Reset player position
        playerY = 35;
        currentMode = MODE_MENU; // Return to menu
    }

    // Redraw the maze and player
    display.clearDisplay();
    drawMaze();
    mazeDrawPlayer(playerX, playerY);
    display.display();
    
    // Add a way to exit the maze (e.g., if player tilts sharply)
    if (abs(accelX) > 0.8 && abs(accelY) > 0.8) {
        currentMode = MODE_MENU;
        return;
    }
}

bool mazeCheckCollision(int x, int y) {
    // Check if the player's position collides with maze walls
    if ((x >= 20 && x < 30 && y >= 10 && y < 50) ||  // Vertical wall
        (x >= 40 && x < 90 && y >= 20 && y < 30) ||  // Horizontal wall
        (x >= 90 && x < 100 && y >= 30 && y < 50) || // Vertical wall near exit
        (x >= 70 && x < SCREEN_WIDTH - 10 && y == 50)) { // Horizontal wall near exit
        return true; // Collision detected
    }
    return false; // No collision
}

void drawMaze() {
    // Draw the maze with entry and exit holes
    display.clearDisplay();

    // Draw a border around the screen
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    // Create an entry hole on the left border
    if (isGameOn) {
        display.drawLine(0, 30, 0, 40, SSD1306_WHITE);
    } else {
        display.drawLine(0, 30, 0, 40, SSD1306_BLACK);
        display.setCursor(5, 30);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.print(F("Entry"));
    }

    // Create an exit hole on the right border
    display.drawLine(SCREEN_WIDTH - 1, 50, SCREEN_WIDTH - 1, 60, SSD1306_BLACK);

    // Add internal maze walls
    display.drawRect(20, 10, 10, 40, SSD1306_WHITE); // Vertical wall
    display.drawRect(40, 20, 50, 10, SSD1306_WHITE); // Horizontal wall
    display.drawRect(90, 30, 10, 20, SSD1306_WHITE); // Vertical wall near exit
    display.drawLine(70, 50, SCREEN_WIDTH - 10, 50, SSD1306_WHITE); // Horizontal wall near exit

    // Ensure the exit path is open
    display.drawLine(SCREEN_WIDTH - 1, 50, SCREEN_WIDTH - 1, 60, SSD1306_BLACK);
}

void mazeDrawPlayer(int x, int y) {
    // Draw the player as a ball
    display.fillCircle(x, y, 3, SSD1306_WHITE); // Radius of 3 for the ball
}