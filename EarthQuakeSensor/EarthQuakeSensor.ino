#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // Common I2C address for SSD1306 displays

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MPU6050 mpu;

const int ledPin = 10;        // LED to indicate earthquake detected
const float threshold = 0.3;  // Acceleration threshold (g) for earthquake detection
const int buzzer = 9;         // buzzer to arduino pin 9

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();

  pinMode(ledPin, OUTPUT);

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while(1); // Don't proceed, loop forever
  }
  
  // Initial display setup
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Earthquake Sensor");
  display.println("Monitoring...");
  display.display();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 connected");
  } else {
    Serial.println("MPU6050 connection failed");
    while (1);
  }
}

// Function to calculate approximate Richter scale value
float calculateRichterScale(float acceleration) {
  // This is a simplified approximation
  float adjustedAccel = abs(acceleration - 1.0);
  if (adjustedAccel > threshold) {
    // Convert to approximate Richter scale (simplified formula)
    return log10(adjustedAccel * 10) + 2.0;
  }
  return 0.0;
}

void loop() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Convert raw values to 'g'
  float xg = ax / 16384.0;
  float yg = ay / 16384.0;
  float zg = az / 16384.0;

  // Calculate total acceleration vector magnitude
  float totalAccel = sqrt(xg * xg + yg * yg + zg * zg);

  Serial.print("Accel: ");
  Serial.print(xg, 2);
  Serial.print(", ");
  Serial.print(yg, 2);
  Serial.print(", ");
  Serial.print(zg, 2);
  Serial.print(" | Total: ");
  Serial.println(totalAccel, 2);

  if (abs(totalAccel - 1.0) > threshold) {  // Subtract 1g for gravity
    digitalWrite(ledPin, HIGH);
    
    // Calculate approximate Richter scale value
    float richterValue = calculateRichterScale(totalAccel);
    
    Serial.print("Earthquake detected! Approximate Richter: ");
    Serial.println(richterValue, 1);
    
    // Update OLED with Richter scale value
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("EARTHQUAKE!");
    
    display.setTextSize(3);
    display.setCursor(0, 25);
    display.print("R: ");
    display.println(richterValue, 1);
    display.display();
    
    pinMode(buzzer, OUTPUT);  // Set buzzer - pin 9 as an output

    tone(buzzer, 1000);  // Send 1KHz sound signal...
    delay(1000);         // ...for 1 sec
    noTone(buzzer);      // Stop sound...
    delay(1000);         // ...for 1sec

    // Initial display setup
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Earthquake Sensor");
  display.println("Monitoring...");
  display.display();
  } else {
    digitalWrite(ledPin, LOW);
  }

  delay(200);
}