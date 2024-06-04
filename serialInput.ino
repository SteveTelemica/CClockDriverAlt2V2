// Read serial input, e.g. to set time
#define SERIALBUFFERSIZE 30
char SerialInputBuffer[ SERIALBUFFERSIZE];
int SerialInputIndex = 0;
void ReadInput() {
  char x = 0;
  while ( Serial.available()) {
    x = Serial.read();
    if (x >= 32) {
      SerialInputBuffer[ SerialInputIndex++] = x;
      // Terminate string
      SerialInputBuffer[ SerialInputIndex] = 0;
      if (SerialInputIndex >= SERIALBUFFERSIZE) {
        SerialInputIndex = 0; // Overflow
        if (LogError) Serial.println(F("E Serial Overflow."));
        break;
      }
    }
  }
  // Are we at the end of a string
  if (SerialInputIndex > 0 && x == '\r') {
    Serial.print(F("Serial Received: "));
    Serial.println( SerialInputBuffer);

    // Set RTC date and time SDec 26 2009 12:34:56
    //                       012345678901234567890
    if (SerialInputBuffer[0] == 'S' && SerialInputIndex == 21) {
      Serial.println( F("RTC set from input") );
      RtcDateTime input = RtcDateTime(SerialInputBuffer+1, 
                                      SerialInputBuffer+13);
      RTC.SetDateTime(input);
      setupRTC();
    }
    // LG or Lg etc. to activate/deactivate GPS/RTC Logging
    // Letter is upper case to activate, lower to deactivate
    if (SerialInputBuffer[0] == 'L') {
      Serial.print( F("Change logging "));
      Serial.print( SerialInputBuffer[1] );
      bool UpperCase = (SerialInputBuffer[1] & 0x20) == 0;
      if (UpperCase) {
        Serial.println( F(" ON") );
      } else {
        Serial.println( F(" OFF") );
      }
      //Serial.println( UpperCase );
      //Serial.println( (SerialInputBuffer[1] & 0x5F) );
      if ((SerialInputBuffer[1] & 0x5F) == 'G') LogGPS = UpperCase;
      if ((SerialInputBuffer[1] & 0x5F) == 'R') LogRTC = UpperCase;
      if ((SerialInputBuffer[1] & 0x5F) == 'M') LogMotor = UpperCase;
      if ((SerialInputBuffer[1] & 0x5F) == 'E') LogError = UpperCase;
      if ((SerialInputBuffer[1] & 0x5F) == 'S') LogStatus = UpperCase;
      if ((SerialInputBuffer[1] & 0x5F) == 'D') LogDisplay = UpperCase;
      if ((SerialInputBuffer[1] & 0x5F) == 'A') {
        LogGPS = UpperCase;
        LogRTC = UpperCase;
        LogMotor = UpperCase;
        LogError = UpperCase;
        LogStatus = UpperCase;
        LogDisplay = UpperCase;
        //Serial.println( "All" );
      }
    }

    SerialInputIndex = 0; // Reset
  }
}
