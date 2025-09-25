#include "UnoJoy.h"

void setup() {
  setupPins();
  setupUnoJoy();
}

void loop() {
  dataForController_t controllerData = getControllerData();
  setControllerData(controllerData);
}

void setupPins(void) {
  for (int i = 2; i <= 13; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);

  pinMode(A0, INPUT); // Accelerator
  pinMode(A2, INPUT); // Brake
  pinMode(A3, INPUT); // Clutch
}

// Define sensor calibration range (adjust these)
const int pedalMin = 420;  // Use your real minimum value
const int pedalMax = 730;  // Use your real maximum value

// Define deadzone threshold
const int deadzoneLow = 5;
const int deadzoneHigh = 250;

// Smoothed values for exponential moving average
int smoothedAccel = 0;
int smoothedBrake = 0;
int smoothedClutch = 0;

dataForController_t getControllerData(void) {
  dataForController_t controllerData = getBlankDataForController();

  // Digital buttons (Active LOW)
  controllerData.triangleOn   = !digitalRead(2);
  controllerData.circleOn     = !digitalRead(3);
  controllerData.squareOn     = !digitalRead(4);
  controllerData.crossOn      = !digitalRead(5);
  controllerData.l1On         = !digitalRead(6);
  controllerData.l2On         = !digitalRead(7);
  controllerData.r1On         = !digitalRead(8);
  controllerData.r2On         = !digitalRead(9);
  controllerData.selectOn     = !digitalRead(10);
  controllerData.startOn      = !digitalRead(11);
  controllerData.homeOn       = !digitalRead(12);
  controllerData.dpadUpOn     = !digitalRead(13);

  // Pedals with smoothing and calibration
  controllerData.leftStickY   = readSmoothedAnalogPedal(A0, smoothedAccel); // Accelerator
  controllerData.rightStickX  = readSmoothedAnalogPedal(A2, smoothedBrake); // Brake
  controllerData.rightStickY  = readSmoothedAnalogPedal(A3, smoothedClutch); // Clutch

  return controllerData;
}

int readSmoothedAnalogPedal(int pin, int& smoothedValue) {
  const int samples = 32;
  long total = 0;

  for (int i = 0; i < samples; i++) {
    total += analogRead(pin);
    delayMicroseconds(150);
  }

  int average = total / samples;

  // Apply exponential smoothing: current = 90% previous + 10% new
  smoothedValue = (smoothedValue * 9 + average) / 10;

  // Map to 0–255 joystick range
  int output = map(smoothedValue, pedalMin, pedalMax, 0, 255);
  output = constrain(output, 0, 255);

  // Apply deadzone
  if (output < deadzoneLow) output = 0;
  if (output > deadzoneHigh) output = 255;

  return output;
}
