#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

const int ledPin = 10;        // LED to indicate earthquake detected
const float threshold = 0.3;  // Acceleration threshold (g) for earthquake detection

const int buzzer = 9;  //buzzer to arduino pin 9

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();

  pinMode(ledPin, OUTPUT);

  if (mpu.testConnection()) {
    Serial.println("MPU6050 connected");
  } else {
    Serial.println("MPU6050 connection failed");
    while (1)
      ;
  }
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
    Serial.println("Earthquake detected!");
    pinMode(buzzer, OUTPUT);  // Set buzzer - pin 9 as an output

    tone(buzzer, 1000);  // Send 1KHz sound signal...
    delay(1000);         // ...for 1 sec
    noTone(buzzer);      // Stop sound...
    delay(1000);         // ...for 1sec
  } else {
    digitalWrite(ledPin, LOW);
  }

  delay(200);
}