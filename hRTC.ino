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
      RTCTimeValidMillis = millis();
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
