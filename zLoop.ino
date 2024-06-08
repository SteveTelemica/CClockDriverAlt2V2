// Read RTC 2 min
#define INTERVALBETWEENRTCREAD 120000UL
unsigned long RTCReadMillis = STARTMILLIS;

// Read GPS for 5s every 60s
#define INTERVALBETWEENGPSREAD 60000UL
unsigned long GPSReadMillis = -INTERVALBETWEENGPSREAD; // Start now
#define DURATIONGPSREAD 5000L
unsigned long GPSDurationMillis = STARTMILLIS;

// Reset GPS 60 min
#define INTERVALBETWEENGPSRESET 3600000UL
unsigned long GPSResetMillis = STARTMILLIS;

// Validate time 50s
#define INTERVALBETWEENTIMEVALID 50000UL
unsigned long CheckTimeValidMillis = STARTMILLIS + 12000UL - INTERVALBETWEENTIMEVALID; // Start after 12s

// Reference Interval time 60m
#define INTERVALBETWEENREFTIME 3600000L
unsigned long ReferenceTimeCheckMillis = STARTMILLIS + 10000UL - INTERVALBETWEENREFTIME; // Start after 10s

// Display Summary time 5s
#define INTERVALBETWEENDISPLAYSUMMARY 5000UL
unsigned long DisplaySummaryMillis = STARTMILLIS;

void loop() {
  globalMillis = millis() + STARTMILLIS;
  
  // At this 50s interval validate time
  if (globalMillis - CheckTimeValidMillis > INTERVALBETWEENTIMEVALID) {
    ValidateTime( &minutesValue, &stepValue);
    CheckTimeValidMillis = globalMillis;
  }

  // Read software serial for (e.g 5s every 60s)
  static bool GPSReadMode = false;
  if (globalMillis - GPSReadMillis > INTERVALBETWEENGPSREAD) {
    GPSReadMode = true;
    GPSReadMillis = globalMillis;
    GPSDurationMillis = globalMillis;
    StartGPS();
  }
  if (globalMillis - GPSDurationMillis > DURATIONGPSREAD) {
    GPSReadMode = false;
    // Push this so we do it once
    GPSDurationMillis = globalMillis + INTERVALBETWEENGPSREAD;
    StopGPS();
  }
  if ( GPSReadMode ) {
    ReadGPS();
  } else {
    ProcessMotorMove( minutesValue, stepValue, CheckTimeValidMillis);  
  }

  // Read Serial, e.g. for setting the time
  ReadInput();

  // Reset GPS - hourly
  if (globalMillis - GPSResetMillis > INTERVALBETWEENGPSRESET) {
    setupGPS();
    GPSResetMillis = globalMillis;
  }

  // Hourly calibration of millis vs GPS
  if (globalMillis - ReferenceTimeCheckMillis > INTERVALBETWEENREFTIME) {
    // If we have a valid reference for calibration
    GetRefCalibration();
    ReferenceTimeCheckMillis = globalMillis;
  }

  // Re-read RTC 2m
  if (globalMillis - RTCReadMillis > INTERVALBETWEENRTCREAD) {
    setupRTC();
    RTCReadMillis = globalMillis;
  }

  // DisplaySummary 5s
  if (globalMillis - DisplaySummaryMillis > INTERVALBETWEENDISPLAYSUMMARY) {
    if (LogDisplay) {
      DisplaySummary();
    }
    DisplaySummaryMillis = globalMillis;
  }

  if (!digitalRead( PIN_IN_FWD) || !digitalRead( PIN_IN_REV) ) {
    // If time is really old, or never set, then change our known time
    if (!Timeok) {
      if (!digitalRead( PIN_IN_FWD) ) {
        now = now + 30;
      } else {
        now = now - 30;
      }
      ValidateTime( &minutesValue, &stepValue);
      DisplaySummary();
    }
  }
}

// Show a page of status info
void DisplaySummary() {
  Serial.println();

  Serial.print(F("Millis: "));
  Serial.print( globalMillis);
  Serial.print(F(", RTCok: "));
  Serial.print( RTCok);
  Serial.print(F(", GPSok: "));
  Serial.print( GPSok);
  Serial.print(F(", Timeok: "));
  Serial.println( Timeok);

  Serial.print( F("GPS Time: " ) );
  PrintNow( GPSnow, GPSTimeValidMillis);

  Serial.print( F("RTC Time: " ) );
  PrintNow( RTCnow, RTCTimeValidMillis);

  Serial.print( F("Use Time: " ) );
  PrintNow( now, TimeValidMillis);

  Serial.print(F("satelliteMaxIn10m: "));
  Serial.print( satelliteMaxIn10m);
  Serial.print(F(", MillisAdjustment: "));
  Serial.println( millisAdjustment);

  Serial.print(F("stepsNeeded: "));
  Serial.print(stepsNeeded);
  Serial.print(F(", stepBack: "));
  Serial.print(stepBack);
  Serial.print(F(", StepsToDo: "));
  Serial.print(StepsToDo);
  Serial.print(F(", StepInterval "));
  Serial.println(StepInterval);

  Serial.print(F("Actual Hand Pos: "));
  Serial.print(handpositionmin / 60);
  Serial.print(':');
  Serial.print(handpositionmin % 60);
  Serial.print(F(", Target Hand Pos: "));
  Serial.print(minutesValue / 60);
  Serial.print(':');
  Serial.print(minutesValue % 60);
  Serial.print(' ');
  if (globalBST) {
    Serial.print(F("BST"));
  } else {
    Serial.print(F("GMT"));
  }
  Serial.println();
  Serial.println();
  
#ifdef OLED_DISPLAY
  OLEDRefresh();
#endif
}
