#ifdef STD_DS1302

bool setupRTC() {
  if (LogRTC) Serial.println( F("R Init/Read"));
  RTC.Begin();

  if (!RTC.GetIsRunning()) {
    if (LogRTC) Serial.println( F("R Not running.") );
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing
    // following line sets the RTC to the date & time this sketch was compiled
    // it will also reset the valid flag internally unless the Rtc device is
    // having an issue
// NEVER CALL THIS
#ifdef UNUSED
    Serial.println( F("R Set from compiled") );
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    RTC.SetDateTime(compiled);
#endif
  }
  else
  {
    if (LogRTC) Serial.println( F("R running"));
  }

  if (!RTC.GetIsRunning()) {
    if (LogRTC) Serial.println( F("R not running, starting" ) );
    RTC.SetIsRunning(true);
  }
  if (!RTC.GetIsRunning()) {
    if (LogError) Serial.println( F("E RTC still not running" ) );
  } else {
    if (LogRTC) Serial.println( F("R running" ) );
    RTCnow = RTC.GetDateTime();

    // Check RTC Health
    if ( (RTCnow.Year() > 2021     ) &&
         (RTCnow.Month() <= 12     ) &&
         (RTCnow.Day() <= 31       )    ) {
      // Drive time into the system: setTimeAndOffset();
      RTCok = true;
      RTCTimeValidMillis = globalMillis;
      if (LogStatus) {
        Serial.print( F("S RTC Time: " ) );
        PrintNow( RTCnow, RTCTimeValidMillis);
      }
    } else {
      if (LogError) Serial.println( F("E RTC time invalid" ) );
      if (LogRTC) {
        Serial.print( F("R Was: " ) );
        Serial.print( RTCnow.Day() );
        Serial.print( "/" );
        Serial.print( RTCnow.Month() );
        Serial.print( "/" );
        Serial.println( RTCnow.Year() );
      }
    }
  }
}

void setRTC( RtcDateTime intime) {
  if (LogRTC) Serial.println( F("R Set RTC"));
  RTC.SetDateTime(intime);
}
#else
bool setupRTC() {
  if (LogRTC) Serial.println( F("R Init/Read"));
  // Set the clock to run-mode, and disable the write protection
  RTC.halt(false);
  RTC.writeProtect(false);
  // Read time
  Time t;
  t = RTC.getTime();
  if (t.year > 2023 && t.mon <= 12 && t.date <= 31) {
    if (LogRTC) Serial.println( F("R valid" ) );
    RTCnow  = RtcDateTime(  t.year - 2000, // 4 dig
                            t.mon,
                            t.date,
                            t.hour,
                            t.min,
                            t.sec );
    RTCok = true;
    RTCTimeValidMillis = globalMillis;
    if (LogStatus) {
      Serial.print( F("S RTC Time: " ) );
      PrintNow( RTCnow, RTCTimeValidMillis);
    }
  } else {
    RTCok = false;
    if (LogError) Serial.println( F("E RTC time invalid" ) );
    if (LogRTC) {
      Serial.print( F("R was: " ) );
      Serial.print( t.date );
      Serial.print( "/" );
      Serial.print( t.mon );
      Serial.print( "/" );
      Serial.println( t.year );
    }
  }
}

void setRTC( RtcDateTime intime) {
  if (LogRTC) Serial.println( F("R Set RTC"));
  RTC.setTime(intime.Hour(), intime.Minute(), intime.Second() );     // Set the time to hh, mm, ss (24hr format)
  RTC.setDate(intime.Day(), intime.Month(), intime.Year() );   // Set the date to dd, MM, YYYY
}
#endif
