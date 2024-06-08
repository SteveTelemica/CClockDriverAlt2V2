// Motor Control


// Setup
void SetupMotor() {
  pinMode( PIN_LED_OUT,    OUTPUT);        // LED
  pinMode( PIN_STEP_DIR,   OUTPUT);        // Dir
  pinMode( PIN_STEP_PULSE, OUTPUT);        // Pulse
  digitalWrite( PIN_LED_OUT,    HIGH);      // LED
  digitalWrite( PIN_STEP_DIR,   LOW);      // Dir
  digitalWrite( PIN_STEP_PULSE, LOW);      // Pulse
}

// Direction - true for backwards
void SetDirection( bool stepBack) {
  // Low = Forward, High = Backward
  digitalWrite( PIN_STEP_DIR, stepBack);
}

void StepMotor( unsigned int delayuS) {
  if (delayuS < 16000) {
    delayMicroseconds( delayuS);
    digitalWrite( PIN_STEP_PULSE, HIGH);
    delayMicroseconds( delayuS);
    digitalWrite( PIN_STEP_PULSE, LOW);
  } else {
    delay( delayuS >> 10L);
    digitalWrite( PIN_STEP_PULSE, HIGH);
    delay( delayuS >> 10L);
    digitalWrite( PIN_STEP_PULSE, LOW);
  }
}

void MoveRecordMotorStep( unsigned int delayuS,
                          bool stepBack) {
  bool newMinute = false;
  SetDirection( stepBack);
  StepMotor( delayuS);
  if (stepBack) {
    handpositionstep--;
    if (handpositionstep < 0) {
      handpositionstep += STEPSPERMINUTE;
      handpositionmin--;
      if (handpositionmin < 0) {
        handpositionmin = MINUTESPER12H-1;
      }
      newMinute = true;
    }
  } else {
    handpositionstep++;
    if (handpositionstep > STEPSPERMINUTE) {
      handpositionstep -= STEPSPERMINUTE;
      handpositionmin++;
      if (handpositionmin >= MINUTESPER12H) {
        handpositionmin = 0;
      }
      newMinute = true;
    }
  }
  if (newMinute) {
    writeHandPosition( handpositionmin);
  }
}

void ProcessMotorMove(int thisMinute, int thisStep, unsigned long CheckTimeValidMillis) {
  static int oldMinute = 0;

  // Don't log every move - will take too long
  static int logCount = 0;
  bool LogMotor2 = false;
  if (LogMotor) {
    logCount++;
    if (logCount == 200) {
      LogMotor2 = true;
      logCount = 0;
    }
  }

  long adjustSteps = ( ( (long)(globalMillis - CheckTimeValidMillis) ) * STEPSPERSECOND )/ 1000L;

  stepsNeeded = ( ((long)thisMinute * STEPSPERMINUTE + thisStep + adjustSteps) - 
                       ((long)handpositionmin * STEPSPERMINUTE + handpositionstep) );

  if (LogMotor2) {
    // Don't log, relates to last clock reading
    //Serial.print(F("M adjustSteps "));
    //Serial.println(adjustSteps);

    Serial.print(F("M stepsNeeded "));
    Serial.print(stepsNeeded);
  }
  while (stepsNeeded > (MINUTESPER12H * STEPSPERMINUTE) ) {
    stepsNeeded -= (MINUTESPER12H * STEPSPERMINUTE);
  }
  while (stepsNeeded < -(MINUTESPER12H * STEPSPERMINUTE) ) {
    stepsNeeded += (MINUTESPER12H * STEPSPERMINUTE);
  }
  if (LogMotor2) {
    Serial.print(F(", Mod= "));
    Serial.print(stepsNeeded);
  }
  // Handle wraparound - just reverse direction if diff over 6 hours
  if ( (stepsNeeded >  360 * STEPSPERMINUTE ) || 
       (stepsNeeded < -360 * STEPSPERMINUTE )   ) {
    stepsNeeded = -stepsNeeded;
    if (LogMotor2) {
      Serial.print(F(", REVERSE"));
    }
  }

  // TEMPORARY *******************************************************
  // Force log if the count > 3
  //if (stepsNeeded > 3) {
  //  Serial.print(stepsNeeded);
  //  Serial.print(',');
  //}

  // Work out stepTarget and intervalTarget
  stepBack = false;
  if (stepsNeeded < 0) {
    stepsNeeded = -stepsNeeded;
    stepBack = true;
  }

  // Take up to say 10,000uS to move the motor in this function
  long TimeAllowed = 10000L;

  // StepsToDo
  StepsToDo = stepsNeeded;

  // How long do we have per step?
  StepInterval = TimeAllowed / StepsToDo;

  // If smaller than minimum allowed
  if (StepInterval < MICROSPERSTEPMIN) {
    StepInterval = MICROSPERSTEPMIN;
    StepsToDo = TimeAllowed / StepInterval;
  }

  if (LogMotor2) {
    // Continuation
    Serial.print(F(", StepsToDo "));
    Serial.print(StepsToDo);

    Serial.print(F(", StepInterval "));
    Serial.println(StepInterval);
  }

  // Motor
  for( long s = 0; s < StepsToDo; s++) {
    MoveRecordMotorStep( StepInterval,
                         stepBack );
  }

  // If the fwd/rev buttons are pressed, move motor without recording position
  if (!digitalRead( PIN_IN_FWD) || !digitalRead( PIN_IN_REV) ) {
    // Unless time is really old
    if (Timeok) {
      // Jump motor forward or back
      if (!digitalRead( PIN_IN_FWD) ) {
        SetDirection( LOW);
      } else {
        SetDirection( HIGH);
      }
      // Move fast for 20ms
      for (int p = (20000 / (MICROSPERSTEPMIN/4)); p--; p == 0) {
        StepMotor( MICROSPERSTEPMIN/4);
      }
    }
  }
}
