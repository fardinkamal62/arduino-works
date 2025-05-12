#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050.h>

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
    display.clearDisplay();
    display.setTextSize(2); // Large text for heading
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 0);
    display.println(F("Maze Game"));
    display.setTextSize(1);
    display.setCursor(SCREEN_HEIGHT / 2, 20);
    display.println(F("by Fardin"));
    display.display();
    delay(2000); // Show for 2 seconds

    drawMaze();
    drawPlayer(playerX, playerY);
    delay(2000);
}

void loop() {
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
    }

    // Redraw the maze and player
    display.clearDisplay();
    drawMaze();
    drawPlayer(playerX, playerY);
    display.display();
}

bool checkCollision(int x, int y) {
    // Check if the player's position collides with maze walls
    // Example collision logic based on maze structure
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

    display.display();
}

void drawPlayer(int x, int y) {
    // Draw the player as a ball
    display.fillCircle(x, y, 3, SSD1306_WHITE); // Radius of 3 for the ball
    display.display();
}
