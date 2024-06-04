// Read RTC 2 min
#define INTERVALBETWEENRTCREAD 120000L
unsigned long RTCReadMillis = 0;

// Read GPS for 5s every 60s
#define INTERVALBETWEENGPSREAD 60000L
unsigned long GPSReadMillis = -INTERVALBETWEENGPSREAD; // Start now
#define DURATIONGPSREAD 5000L
unsigned long GPSDurationMillis = 0L;

// Reset GPS 60 min
#define INTERVALBETWEENGPSRESET 3600000L
unsigned long GPSResetMillis = 0;

// Validate time 50s
#define INTERVALBETWEENTIMEVALID 50000L
unsigned long CheckTimeValidMillis = 12000L - INTERVALBETWEENTIMEVALID; // Start after 12s

// Reference Interval time 60m
#define INTERVALBETWEENREFTIME 3600000L
unsigned long ReferenceTimeCheckMillis = 10000L - INTERVALBETWEENREFTIME; // Start after 10s

// Display Summary time 5s
#define INTERVALBETWEENDISPLAYSUMMARY 5000L
unsigned long DisplaySummaryMillis = 0;

void loop() {
  // At this 50s interval validate time
  if ((long)millis() - (long)CheckTimeValidMillis > INTERVALBETWEENTIMEVALID) {
    ValidateTime( &minutesValue, &stepValue);
    CheckTimeValidMillis = millis();
  }

  // Read software serial for (e.g 5s every 60s)
  static bool GPSReadMode = false;
  if ((long)millis() - (long)GPSReadMillis > INTERVALBETWEENGPSREAD) {
    GPSReadMode = true;
    GPSReadMillis = millis();
    GPSDurationMillis = millis();
    StartGPS();
  }
  if ((long)millis() - (long)GPSDurationMillis > DURATIONGPSREAD) {
    GPSReadMode = false;
    // Push this so we do it once
    GPSDurationMillis = millis() + INTERVALBETWEENGPSREAD;
    StopGPS();
  }
  if ( GPSReadMode ) {
    ReadGPS();
  } else {
    // Move the motor when GPS is not being read, as it uses SoftwareSerial
    if (minutesValue >= 0) {
      ProcessMotorMove( minutesValue, stepValue, CheckTimeValidMillis);  
    }
  }

  // Read Serial, e.g. for setting the time
  ReadInput();

  // Reset GPS - hourly
  if ((long)millis() - (long)GPSResetMillis > INTERVALBETWEENGPSRESET) {
    setupGPS();
    GPSResetMillis = millis();
  }

  // Hourly calibration of millis vs GPS
  if ((long)millis() - (long)ReferenceTimeCheckMillis > INTERVALBETWEENREFTIME) {
    // If we have a valid reference for calibration
    GetRefCalibration();
    ReferenceTimeCheckMillis = millis();
  }

  // Re-read RTC 2m
  if ((long)millis() - (long)RTCReadMillis > INTERVALBETWEENRTCREAD) {
    setupRTC();
    RTCReadMillis = millis();
  }

  // DisplaySummary 5s
  if ((long)millis() - (long)DisplaySummaryMillis > INTERVALBETWEENDISPLAYSUMMARY) {
    if (LogDisplay) {
      DisplaySummary();
    }
    DisplaySummaryMillis = millis();
  }

}

// Show a page of status info
void DisplaySummary() {
  Serial.println();

  Serial.print(F("Millis: "));
  Serial.print( millis());
  Serial.print(F(", RTCok: "));
  Serial.print( RTCok);
  Serial.print(F(", RTCok: "));
  Serial.print( GPSok);
  Serial.print(F(", Timeok: "));
  Serial.println( Timeok);

  Serial.print( F("GPS Time: " ) );
  PrintNow( GPSnow, GPSTimeValidMillis);

  Serial.print( F("RTC Time: " ) );
  PrintNow( RTCnow, RTCTimeValidMillis);

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
  Serial.println(minutesValue % 60);

  Serial.println();
  
#ifdef OLED_DISPLAY
  OLEDRefresh();
#endif
}
