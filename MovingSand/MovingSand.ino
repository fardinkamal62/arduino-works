#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Fluid simulation parameters
#define FLUID_PARTICLES 16  // Particle count
#define SIM_WIDTH SCREEN_WIDTH
#define SIM_HEIGHT SCREEN_HEIGHT
#define GRAVITY 1.50f
#define DAMPING 15.9f  // Energy loss from collision
#define MAX_VELOCITY 2.4f

MPU6050 mpu;

struct Particle {
    float x, y;       // position
    float vx, vy;     // velocity
};

Particle particles[FLUID_PARTICLES];
float accelX = 0, accelY = 0;

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
    display.setTextSize(1); // Large text for heading
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(SCREEN_HEIGHT / 2, 0);
    display.println(F("Falling Sand"));
    display.setCursor(SCREEN_HEIGHT / 2, 20);
    display.println(F("by Fardin"));
    display.display();
    delay(2000); // Show for 2 seconds
    
    // Initialize particles
    for(int i = 0; i < FLUID_PARTICLES; i++) {
        particles[i].x = random(SIM_WIDTH);
        particles[i].y = SIM_HEIGHT - random(10);
        particles[i].vx = 0;
        particles[i].vy = 0;
    }
}

void loop() {
    // Read accelerometer data
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    
    // Convert accelerometer data to simulation coordinates
    accelX = constrain(ay / 16384.0f, -1.0f, 1.0f);
    accelY = -constrain(ax / 16384.0f, -1.0f, 1.0f); // Inverted Y-axis
    
    updateParticles();
    drawParticles();
    
    // delay(1); // ~60fps
}

void updateParticles() {
    // Update particles with simplified physics
    for(int i = 0; i < FLUID_PARTICLES; i++) {
        // Update velocity
        particles[i].vx = particles[i].vx * 0.9f + (accelX * GRAVITY);
        particles[i].vy = particles[i].vy * 0.9f + (accelY * GRAVITY);
        
        // Limit velocity
        particles[i].vx = constrain(particles[i].vx, -MAX_VELOCITY, MAX_VELOCITY);
        particles[i].vy = constrain(particles[i].vy, -MAX_VELOCITY, MAX_VELOCITY);
        
        // Update position
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        
        // Boundary checks
        if(particles[i].x < 0) {
            particles[i].x = 0;
            particles[i].vx = -particles[i].vx * DAMPING;
        } else if(particles[i].x >= SIM_WIDTH) {
            particles[i].x = SIM_WIDTH - 1;
            particles[i].vx = -particles[i].vx * DAMPING;
        }
        
        if(particles[i].y < 0) {
            particles[i].y = 0;
            particles[i].vy = -particles[i].vy * DAMPING;
        } else if(particles[i].y >= SIM_HEIGHT) {
            particles[i].y = SIM_HEIGHT - 1;
            particles[i].vy = -particles[i].vy * DAMPING;
        }
    }
    
    // Simplified particle interaction (optional - can be removed if memory is tight)
    for(int i = 0; i < FLUID_PARTICLES; i++) {
        for(int j = i + 1; j < FLUID_PARTICLES; j++) {
            float dx = particles[j].x - particles[i].x;
            float dy = particles[j].y - particles[i].y;
            float distSq = dx*dx + dy*dy;
            
            if(distSq < 100 && distSq > 0) {
                float push = 0.5f / sqrt(distSq);
                particles[i].x -= dx * push;
                particles[i].y -= dy * push;
                particles[j].x += dx * push;
                particles[j].y += dy * push;
            }
        }
    }
}

void drawParticles() {
    display.clearDisplay();
    
    // Draw particles as simple pixels to save memory
    for(int i = 0; i < FLUID_PARTICLES; i++) {
        display.drawPixel(
            (int)particles[i].x, 
            (int)particles[i].y, 
            SSD1306_WHITE
        );
    }
    
    display.display();
}