/**
 */
#define GAMEPAD
#ifdef GAMEPAD
#include <BleGamepad.h>
BleGamepad device;
#define RESET_BUTTON BUTTON_2
#define MOVE_BUTTON  BUTTON_1
#else
#include <BleKeyboard.h>
BleKeyboard device;
#define RESET_BUTTON KEY_ESC
#define MOVE_BUTTON  '5'
#define RIGHT_BUTTON '6'
#define LEFT_BUTTON '4'
#define BOTH_BUTTON '8'
#endif

#include "debounce.h"

#define LED 2

#define RESET_TOUCH_PIN 4 
#define LEFT 32
#define RIGHT 33
#define RESET 14
#define ROTATION 13
Debounce rotation(ROTATION, LOW, 5);
Debounce left(LEFT, HIGH, 20);
Debounce right(RIGHT, HIGH, 20);
Debounce reset(RESET, HIGH, 20);//


boolean pressedLeft = false;
boolean pressedRight = false;
boolean pressedReset = false;
boolean pressedBoth = false;
boolean touchReset = false;
boolean rotationActive = false;

void setup() {
  pinMode(LED,OUTPUT);
  digitalWrite(LED,0);
  pinMode(LEFT,INPUT_PULLDOWN);
  pinMode(RIGHT,INPUT_PULLDOWN);
  pinMode(RESET,INPUT_PULLDOWN);
  pinMode(ROTATION,INPUT);
  rotationActive = rotation.getState();
  
//  Serial.begin(115200);
//  Serial.println("Starting BLE work!");
  device.begin();
}

void loop() {

  if(device.isConnected()) {
    digitalWrite(LED,1);
    
    boolean leftState = left.getState();
    boolean rightState = right.getState();
    boolean bothState = leftState && rightState; // && digitalRead(LEFT) && digitalRead(RIGHT);

#ifdef GAMEPAD
    boolean axesChanged = false;
#endif

    if (leftState) {
      if (!pressedLeft) {
        pressedLeft = true;
#ifdef GAMEPAD
        axesChanged = true;
#else                
        device.press(LEFT_BUTTON);
#endif
      }
    }
    else {
      if (pressedLeft) {
        pressedLeft = false;
#ifdef GAMEPAD
        axesChanged = true;
#else        
        device.release(LEFT_BUTTON);
#endif
      }
    }
    
    if (rightState) {
      if (!pressedRight) {
        pressedRight = true;
#ifdef GAMEPAD
        axesChanged = true;
#else
        device.press(RIGHT_BUTTON);
#endif
      }
    }
    else {
      if (pressedRight) {
        pressedRight = false;
#ifdef GAMEPAD
        axesChanged = true;
#else
        device.release(RIGHT_BUTTON);
#endif
      }
    }

#ifndef GAMEPAD
    if (bothState) {
      if (!pressedBoth) {
        pressedBoth = true;
        device.press(BOTH_BUTTON);
      }
    }
    else {
      if (pressedBoth) {
        pressedBoth = false;
        device.release(BOTH_BUTTON);
      }
    }
#endif    

    if (reset.getState()) {
      if (!pressedReset) {
        pressedReset = true;
        device.press(RESET_BUTTON);
      }
    }
    else {
      if (pressedReset) {
        pressedReset = false;
        device.release(RESET_BUTTON);
      }
    }

/*
    if (touchRead(RESET_TOUCH_PIN)<40) {
      if (!touchReset) {
        bleKeyboard.press(KEY_ESC);
        touchReset = true;
      }
    }
    else {
      if (touchReset) {
        bleKeyboard.release(KEY_ESC);
        touchReset = false;
      }
    }
*/
  
    if (rotation.getState()) {
      if (!rotationActive) {
        rotationActive = true;
        device.press(MOVE_BUTTON);
      }
    }
    else {
      if (rotationActive) {
        rotationActive = false;
        device.release(MOVE_BUTTON);
      }
    }

#ifdef GAMEPAD
    if (axesChanged) {
      device.setAxes(pressedLeft ? 0 : 16384, pressedRight ? 0 : 16384);
    }
#endif

  }
  else {
//    Serial.println("Waiting 2 seconds...");
    delay(2000);
  }
}
