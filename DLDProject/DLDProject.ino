#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050.h>
#include "MovingSand.h"
#include "MazeGame.h"
#include "DLDProject.h"

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

MPU6050 mpu;

int playerX = 5; // Initial player position (entry hole)
int playerY = 35;

float playerVX = 0, playerVY = 0; // Player velocity
#define MAX_PLAYER_VELOCITY 5.4f  // Maximum velocity for the player
#define PLAYER_SPEED 2.0f         // Increased scaling factor for movement
#define PLAYER_DAMPING 0.95f      // Reduced damping for faster response

bool isGameOn = false;

// Globals for text bounding boxes
int mazeTextX, mazeTextY, mazeTextW, mazeTextH;
int sandTextX, sandTextY, sandTextW, sandTextH;


GameMode currentMode = MODE_MENU;

void setup() {
    Serial.begin(9600);
    Wire.begin();
    
    // Initialize MPU6050 before display to ensure proper I2C communication
    mpu.initialize();
    
    if(!mpu.testConnection()) {
        Serial.println("MPU6050 connection failed");
        while(1);
    }
    
    // Try both common OLED addresses
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("Failed to initialize OLED at 0x3C, trying 0x3D"));
        if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
            Serial.println(F("SSD1306 allocation failed"));
            for(;;);
        }
    }
    
    display.clearDisplay();
    display.display();
    delay(100);  // Short delay after initialization

    // Display heading and subheading
    display.setTextSize(1); // Large text for heading
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(SCREEN_HEIGHT / 2, 0);
    display.println(F("DLD Project"));
    display.setCursor(SCREEN_HEIGHT / 2, 20);
    display.println(F("by Fardin"));
    display.display();

    display.clearDisplay();
    delay(2000);

    drawMenu();
    drawPlayer(playerX, playerY);

    currentMode = MODE_MENU;
}

void loop() {
    if (currentMode == MODE_MENU) {
        isGameOn = true;
        // Read accelerometer data
        int16_t ax, ay, az;
        mpu.getAcceleration(&ax, &ay, &az);

        // Convert accelerometer data to movement values
        float accelX = constrain(ay / 16384.0f, -1.0f, 1.0f);
        float accelY = -constrain(ax / 16384.0f, -1.0f, 1.0f); // Inverted Y-axis

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
        if (!checkCollision(newPlayerX, newPlayerY)) {
            playerX = newPlayerX;
            playerY = newPlayerY;
        }

        // Boundary checks
        if (playerX < 0) playerX = 0;
        if (playerX >= SCREEN_WIDTH) playerX = SCREEN_WIDTH - 1;
        if (playerY < 0) playerY = 0;
        if (playerY >= SCREEN_HEIGHT) playerY = SCREEN_HEIGHT - 1;

        // Redraw the maze and player
        display.clearDisplay();
        drawMenu();
        drawPlayer(playerX, playerY);
        display.display();
    } else if (currentMode == MODE_SAND) {
        loopMovingSand();
        // Add a way to exit sand mode, e.g. button press or timeout
        // Example: if (digitalRead(EXIT_BUTTON_PIN) == LOW) {
        //     currentMode = MODE_MENU;
        //     drawMenu();
        //     drawPlayer(playerX, playerY);
        //     display.display();
        // }
    } else if (currentMode == MODE_MAZE) {
        loopMazeGame();
    }
}

// Improved collision logic: maze walls + text
bool checkCollision(int x, int y) {
    // Text collision: Maze Game
    if (x >= mazeTextX && x <= mazeTextX + mazeTextW &&
        y >= mazeTextY && y <= mazeTextY + mazeTextH) {
        onMazeGameTouched();
    }
    // Text collision: Falling Sand
    if (x >= sandTextX && x <= sandTextX + sandTextW &&
        y >= sandTextY && y <= sandTextY + sandTextH) {
        onFallingSandTouched();
    }

    return false; // No collision
}

void drawMenu() {
    // Draw two texts vertically and store their bounding boxes
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    const char* topText = "Maze Game";
    const char* bottomText = "Falling Sand";

    // Calculate text bounds for top text
    int16_t x1, y1;
    uint16_t w1, h1;
    display.getTextBounds(topText, 0, 0, &x1, &y1, &w1, &h1);

    // Calculate text bounds for bottom text
    int16_t x2, y2;
    uint16_t w2, h2;
    display.getTextBounds(bottomText, 0, 0, &x2, &y2, &w2, &h2);

    // Center texts horizontally, space vertically
    mazeTextX = (SCREEN_WIDTH - w1) / 2;
    mazeTextY = 10;
    mazeTextW = w1;
    mazeTextH = h1;

    sandTextX = (SCREEN_WIDTH - w2) / 2;
    sandTextY = SCREEN_HEIGHT / 2 + 5; // space in the middle
    sandTextW = w2;
    sandTextH = h2;

    display.setCursor(mazeTextX, mazeTextY);
    display.println(topText);
    display.setCursor(sandTextX, sandTextY);
    display.println(bottomText);
}

// Called when player touches "Maze Game" text
void onMazeGameTouched() {
    // Placeholder: you can add your logic here
    Serial.println("Maze Game text touched!");
    setupMazeGame();
    currentMode = MODE_MAZE;
}

// Called when player touches "Falling Sand" text
void onFallingSandTouched() {
    Serial.println("Falling Sand text touched!");
    setupMovingSand();
    currentMode = MODE_SAND;
    // No infinite loop here!
}

void drawPlayer(int x, int y) {
    // Draw the player as a ball
    display.fillCircle(x, y, 3, SSD1306_WHITE); // Radius of 3 for the ball
    display.display();
}
