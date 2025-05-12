#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Not used with I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

MPU6050 mpu;

// Define a buffer for the moving average filter
#define FILTER_SIZE 10
float angleBuffer[FILTER_SIZE] = {0};
int bufferIndex = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Initialize MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1); // Halt execution if MPU6050 fails
  }

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED connection failed");
    while (1); // Halt execution if OLED fails
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  int16_t ax, ay, az;

  // Get acceleration data from MPU6050
  mpu.getAcceleration(&ax, &ay, &az);

  // Calculate tilt angle in degrees
  float angle = atan2((float)ay, (float)az) * 180.0 / PI;

  // Add the new angle to the buffer
  angleBuffer[bufferIndex] = angle;
  bufferIndex = (bufferIndex + 1) % FILTER_SIZE;

  // Calculate the average angle
  float smoothedAngle = 0;
  for (int i = 0; i < FILTER_SIZE; i++) {
    smoothedAngle += angleBuffer[i];
  }
  smoothedAngle /= FILTER_SIZE;

  // Map the smoothed angle to a vertical offset for the display
  int offset = map(smoothedAngle, -45, 45, -20, 20);
  offset = constrain(offset, -20, 20); // Ensure offset stays within bounds

  display.clearDisplay();

  // Draw a line that "tilts" based on the smoothed angle
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    int y = SCREEN_HEIGHT / 2 + (x - SCREEN_WIDTH / 2) * offset / SCREEN_WIDTH;

    // Ensure y is within valid display bounds
    if (y >= 0 && y < SCREEN_HEIGHT) {
      display.drawPixel(x, y, WHITE);
    }
  }

  // Update the display
  display.display();
  delay(50); // Small delay for stability
}
