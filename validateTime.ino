// Prioritise time source and validity
// Time source needs to be less than an hour old
// If not the source is invalidated
// If neither are valid, we carry on counting millisecs from last
void ValidateTime( int *handposmin, int *handposstep) {
  int AdjTime = 0;
  if (GPSok && millis() - GPSTimeValidMillis < 3600000L) {
    now = GPSnow;
    TimeValidMillis = GPSTimeValidMillis;
    Timeok = true;
    if (LogStatus) Serial.print( F("S Using GPS "));

  } else {
    GPSok = false;
    if (RTCok && millis() - RTCTimeValidMillis < 3600000L) {
      now = RTCnow;
      TimeValidMillis = RTCTimeValidMillis;
      Timeok = true;
      if (LogStatus) Serial.print( F("S Using RTC "));
    } else {
      RTCok = false;
      if (LogError) Serial.println( F("E Old time "));
      // No good time, but check for millis rollover and invalidate
#define TIMEROLLOVER 2000000000L // 23 Days
      if ((long)millis() - (long)TimeValidMillis > TIMEROLLOVER) {
        if (LogError) Serial.println( F("E Old time too old "));
        Timeok = false;
        // Invalid time - move hands to 12:00
        *handposmin = -1;
        *handposstep = 0;
        return; // Don't print time
      } else {
        // Get hours difference from last good time
        int hourdiff = (millis() - TimeValidMillis) / 3600000L;
        // Adjust known time by this times: millisAdjustment
        AdjTime = hourdiff * millisAdjustment;
      }
    }
  }
  // Good time, print it
  if (LogStatus) PrintNow( now, TimeValidMillis);
  // Return Minute, Steps and the offset to add TimeValidMillis
  // Add time offset up to now
  long addTimeOffset = (long)millis() - (long)TimeValidMillis;
  // Also if millisAdjustment is positive, crystal is running fast, so slow down
  realTime = now + ( (addTimeOffset - AdjTime)/ (long)1000L ); // (ignore millisec)

  // Set RTC from GPS if valid
  if (GPSok) {
    RTC.SetDateTime(realTime);
    if (LogRTC) Serial.print( F("R Set RTC from GPS "));
  }
  
  // Convert to Local Time
  int bstOffset = isBST( now.Year(), now.Month(), now.Day(), now.Hour() ) ? 3600 : 0;
  realTime = realTime + bstOffset;
  int minutes = (realTime.Hour()*60 + realTime.Minute() ) % 720;
  int stepvalue = (realTime.Second() * STEPSPERMINUTE ) / 60; // Set to the offset from read-time (ignore millisec)

  if (LogStatus) {
    Serial.print( F("S Hand target Min: "));
    PrintHandPos( minutes, stepvalue);
  }
  *handposmin = minutes;
  *handposstep = stepvalue;
  return;
}

// Return True if in BST
bool isBST(int year, int month, int day, int hour)
{
  // bst begins at 01:00 gmt on the last sunday of march
  // and ends at 01:00 gmt (02:00 bst) on the last sunday of october
  // january, february, and november are out
  if (month < 3 || month > 10) { return false; }
  // april to september are in
  if (month > 3 && month < 10) { return true; }
  // last sunday of march
  int lastMarSunday =  (31 - (5* year /4 + 4) % 7);
  // last sunday of october
  int lastOctSunday = (31 - (5 * year /4 + 1) % 7);
  // in march we are bst if its past 1am gmt on the last sunday in the month
  if (month == 3) {
    if (day > lastMarSunday) {
      return true;
    }
    if (day < lastMarSunday) {
      return false;
    }
    if (hour < 1) {
      return false;
    }
    return true;
  }
  // in october we must be before 1am gmt (2am bst) on the last sunday to be bst
  if (month == 10) {
    if (day < lastOctSunday) {
      return true;
    }
    if (day > lastOctSunday) {
      return false;
    }
    if (hour >= 1) {
      return false;
    }
    return true;
  }
}

void GetRefCalibration() {
  unsigned long milliscal;
  signed long millisAdj = 0;

  if ( GPSok ) {
    milliscal = GPSTimeValidMillis - GPSTimeReference;
    //Serial.print( F("Last GPS ref: "));
    //Serial.println( GPSTimeReference);
    //Serial.print( F("Last GPS Valid: "));
    //Serial.println( GPSTimeValidMillis );
    if (LogStatus) {
      Serial.print( F("S Calib: Millis diff: "));
      Serial.print( milliscal);
      Serial.print( F(", GPS diff: "));
      Serial.println( GPSnow - GPSref);
    }
    // Difference in seconds over 1h
    signed long secdiff = (GPSnow - GPSref) - 3600;
    signed long millisdiff = milliscal - 3600000;
    millisAdj = millisdiff - 1000 * secdiff;
    // If millisAdjustment is positive, crystal is running fast
    if (LogStatus) {
      Serial.print( F("S millisAdj: "));
      Serial.println( millisAdj);
    }
    if ( abs(millisAdj) < 10000) {
      if (LogStatus) Serial.println( F("S Good calibration"));
      millisAdjustment = millisAdj;
    } else {
      if (LogStatus) Serial.println( F("S Failed calibration"));
      millisAdjustment = 0;
    }

    // Get a GPS time ref for next calibration
    GPSref = GPSnow;
    GPSTimeReference = GPSTimeValidMillis;
    //if (LogStatus) Serial.print( F("Reference set: "));
    //if (LogStatus) Serial.println( GPSTimeReference );

  }
}
