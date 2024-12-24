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

#include <Preferences.h>

/* The original system probably had two interruptions per cycle. To match this, we generate a second 
 *  synthetic signal every half rotation in doubleSignal mode. If you don't want this, hold the RESET
 *  button while device is powering on to switch the doubleSignal mode. The device double-blinks the
 *  LED on startup in doubleSignal mode.
 */
#define SYNTHETIC_SIGNAL_MS 50

#include "debounce.h"

#define LED 2

//#define RESET_TOUCH_PIN 4 
#define LEFT 32
#define RIGHT 33
#define RESET 14
#define ROTATION 13
Debounce rotation(ROTATION, LOW, 5);
Debounce left(LEFT, HIGH, 20);
Debounce right(RIGHT, HIGH, 20);
Debounce reset(RESET, HIGH, 20);
Preferences myPreferences;

boolean doubleSignal = true;
uint32_t cycleStart = 0;
uint32_t lastCycleLength = 0;
uint32_t syntheticStart = 0;
boolean needSynthetic = false;

boolean pressedLeft = false;
boolean pressedRight = false;
boolean pressedReset = false;
boolean pressedBoth = false;
boolean touchReset = false;
boolean rotationActive = false;
boolean connected = false;

void setup() {
  pinMode(LED,OUTPUT);
  digitalWrite(LED,0);
  pinMode(LEFT,INPUT_PULLDOWN);
  pinMode(RIGHT,INPUT_PULLDOWN);
  pinMode(RESET,INPUT_PULLDOWN);
  pinMode(ROTATION,INPUT);
  rotationActive = rotation.getState();

  myPreferences.begin("myPrefs", false);

  if (myPreferences.isKey("doubleSignal")) {
    doubleSignal = myPreferences.getBool("doubleSignal");
  }
  
  if (digitalRead(RESET)) {
    doubleSignal = ! doubleSignal;
    myPreferences.putBool("doubleSignal", doubleSignal);
  }
  
  digitalWrite(LED, 1);
  delay(200);
  digitalWrite(LED, 0);
  if (doubleSignal) {
    delay(200);
    digitalWrite(LED, 1);
    delay(200);
    digitalWrite(LED, 0);
  } 
  delay(500);

//  Serial.begin(115200);
//  Serial.println("Starting BLE work!");
  device.begin();
}

void loop() {
  if(device.isConnected()) {
    if (!connected) {
      digitalWrite(LED,1);
      connected = true;
    }
    
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

    uint32_t currentTime = millis();
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
        digitalWrite(LED,0);
        if (doubleSignal) {
          syntheticStart = 0;
          needSynthetic = false;
        }
      }
    }
    else {
      if (rotationActive) {
        rotationActive = false;
        device.release(MOVE_BUTTON);
        digitalWrite(LED,1);
        if (doubleSignal) {
          syntheticStart = 0;
          if (cycleStart > 0) {
            lastCycleLength = currentTime-cycleStart;
          }
          cycleStart = currentTime;
          needSynthetic = true;
        }
      }
    }

    if (doubleSignal) {
      if (needSynthetic && currentTime >= cycleStart + lastCycleLength / 2) {
        needSynthetic = false;
        device.press(MOVE_BUTTON);
        digitalWrite(LED,0);
        syntheticStart = currentTime;
      }
      else if (syntheticStart > 0 && currentTime >= syntheticStart + SYNTHETIC_SIGNAL_MS) {
        needSynthetic = false;
        device.release(MOVE_BUTTON);
        digitalWrite(LED,1);
        syntheticStart = 0;
      }
    }

#ifdef GAMEPAD
    if (axesChanged) {
      device.setAxes(pressedLeft ? 0 : 16384, pressedRight ? 0 : 16384);
    }
#endif

  }
  else {
    connected = false;
    digitalWrite(LED, 0);
    delay(2000);
  }
}
