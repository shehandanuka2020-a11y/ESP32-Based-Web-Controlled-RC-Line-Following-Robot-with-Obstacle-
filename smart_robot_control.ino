/*
 * Project Name: ESP32 Autonomous Smart Robot
 * Author: Danuka Shehan
 * Description: Multi-mode robot controller with Line Following, 
 * Obstacle Avoidance, and Web-based Manual Control.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>


// --- Pin Assignments ---
// Motor Driver (L298N)
#define PIN_IN1 14
#define PIN_IN2 27
#define PIN_IN3 26
#define PIN_IN4 25
#define PIN_ENA 33
#define PIN_ENB 32



// Sensors
#define PIN_IR_LEFT 34
#define PIN_IR_RIGHT 35
#define PIN_TRIG 4
#define PIN_ECHO 5
#define PIN_SERVO 13



// --- Configurations ---
const char* WIFI_SSID = "SmartRobot_AP";
const char* WIFI_PASS = "12345678";
const int MAX_SPEED = 210;
const int SAFE_DISTANCE = 20; // cm



WebServer server(80);
Servo sensorServo;

enum OperationMode { MANUAL, LINE_FOLLOW };
OperationMode currentMode = MANUAL;


// Initialize Hardware
void setup() {
    Serial.begin(115200);
    
    // Motor Pins
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_IN3, OUTPUT);
    pinMode(PIN_IN4, OUTPUT);
    pinMode(PIN_ENA, OUTPUT);
    pinMode(PIN_ENB, OUTPUT);
    
    // Sensor Pins
    pinMode(PIN_IR_LEFT, INPUT);
    pinMode(PIN_IR_RIGHT, INPUT);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    
    sensorServo.attach(PIN_SERVO);
    sensorServo.write(90);

    // Start WiFi Access Point
    WiFi.softAP(WIFI_SSID, WIFI_PASS);
    
    // Web Routes
    server.on("/", handleRoot);
    server.on("/mode/line", []() { currentMode = LINE_FOLLOW; server.send(200); });
    server.on("/mode/manual", []() { currentMode = MANUAL; stopRobot(); server.send(200); });
    
    server.begin();
}



void loop() {
    server.handleClient();

    // Safety: Obstacle Check
    if (readUltrasonic() < SAFE_DISTANCE) {
        stopRobot();
        return;
    }

    // Logic based on mode
    if (currentMode == LINE_FOLLOW) {
        runLineFollowing();
    }
    
    delay(15);
}




// --- Navigation Logic ---

void runLineFollowing() {
    bool left = digitalRead(PIN_IR_LEFT);
    bool right = digitalRead(PIN_IR_RIGHT);

    if (!left && !right) moveForward();
    else if (left && !right) turnLeft();
    else if (!left && right) turnRight();
    else stopRobot();
}



long readUltrasonic() {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    
    long duration = pulseIn(PIN_ECHO, HIGH, 30000);
    return (duration == 0) ? 400 : (duration * 0.034 / 2);
}



// --- Motor Primitives ---

void moveForward() {
    digitalWrite(PIN_IN1, HIGH); digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_IN3, HIGH); digitalWrite(PIN_IN4, LOW);
    analogWrite(PIN_ENA, MAX_SPEED); analogWrite(PIN_ENB, MAX_SPEED);
}

void turnLeft() {
    digitalWrite(PIN_IN1, LOW);  digitalWrite(PIN_IN2, HIGH);
    digitalWrite(PIN_IN3, HIGH); digitalWrite(PIN_IN4, LOW);
}

void turnRight() {
    digitalWrite(PIN_IN1, HIGH); digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_IN3, LOW);  digitalWrite(PIN_IN4, HIGH);
}

void stopRobot() {
    digitalWrite(PIN_IN1, LOW); digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_IN3, LOW); digitalWrite(PIN_IN4, LOW);
}

void handleRoot() {
    server.send(200, "text/html", "<h1>Robot Control</h1><p><a href='/mode/line'>Start Line Follow</a></p>");
}
