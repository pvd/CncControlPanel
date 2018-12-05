// -----
// RotaryEncoder.cpp - Library for using rotary encoders.
// This class is implemented for use with the Arduino environment.
// Copyright (c) by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
// More information on: http://www.mathertel.de/Arduino
// -----
// 18.01.2014 created by Matthias Hertel
// 17.06.2015 minor updates.
// 22.05.2018 PVD Updated the tick to use a logic from http://www.pjrc.com/teensy/td_libs_Encoder.html
// -----

#include "Arduino.h"
#include "RotaryEncoder.h"

// ----- Initialization and Default Values -----

RotaryEncoder::RotaryEncoder(int pin1, int pin2) {
  
  // Remember Hardware Setup
  _pin1 = pin1;
  _pin2 = pin2;
  
  // Setup the input pins
  pinMode(pin1, INPUT);
  digitalWrite(pin1, HIGH);   // turn on pullup resistor

  pinMode(pin2, INPUT);
  digitalWrite(pin2, HIGH);   // turn on pullup resistor

  // when not started in motion, the current state of the encoder should be 3
   if (digitalRead(_pin1)) _oldState |= 1;
   if (digitalRead(_pin1)) _oldState |= 2;
 
  // start with position 0;
  _positionExt = 0;
} // RotaryEncoder()


long  RotaryEncoder::getPosition() {
  return _positionExt;
} // getPosition()


void RotaryEncoder::tick(void)
{
     //                           _______         _______       
     //               Pin1 ______|       |_______|       |______ Pin1
     // negative <---         _______         _______         __      --> positive
     //               Pin2 __|       |_______|       |_______|   Pin2

    //  new new old old
    //  pin2  pin1  pin2  pin1  Result
    //  ----  ----  ----  ----  ------
    //  0 0 0 0 no movement
    //  0 0 0 1 +1
    //  0 0 1 0 -1
    //  0 0 1 1 +2  (assume pin1 edges only)
    //  0 1 0 0 -1
    //  0 1 0 1 no movement
    //  0 1 1 0 -2  (assume pin1 edges only)
    //  0 1 1 1 +1
    //  1 0 0 0 +1
    //  1 0 0 1 -2  (assume pin1 edges only)
    //  1 0 1 0 no movement
    //  1 0 1 1 -1
    //  1 1 0 0 +2  (assume pin1 edges only)
    //  1 1 0 1 -1
    //  1 1 1 0 +1
    //  1 1 1 1 no movement
  
    uint8_t p1val = digitalRead(_pin1);
    uint8_t p2val = digitalRead(_pin2);
    uint8_t state = _oldState & 3;
    
    if (p1val) state |= 4;
    if (p2val) state |= 8;
    
    _oldState = (state >> 2);
    switch (state) {
      case 1: case 7: case 8: case 14:
        _positionExt++;
        return;
      case 2: case 4: case 11: case 13:
        _positionExt--;
        return;
      case 3: case 12:
        _positionExt += 2;
        return;
      case 6: case 9:
        _positionExt -= 2;
        return;
    }
} // tick()

// End
