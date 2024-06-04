void StartGPS() {
  if (LogGPS) Serial.println(F("G Start"));
  if (pGPSSerial != NULL) {
    delete pGPSSerial;
    pGPSSerial = NULL;
  }
  pGPSSerial = new SoftwareSerial( PIN_GPS_RX, PIN_GPS_TX);
  pGPSSerial->begin(9600);
  pGPSSerial->println("");
}

void StopGPS() {
  if (LogGPS) Serial.println(F("G Stop"));
  if (pGPSSerial != NULL) {
    delete pGPSSerial;
    pGPSSerial = NULL;
  }
}

// Output to GPS to reset baud rate and report period
void setupGPS() {
  if (pGPSSerial != NULL) {
    if (LogGPS) Serial.println(F("G Config"));  
    printGPS( "$PUBX,41,1,7,3,9600,0*" );  
    // These commands may be ignored if it's a u-Blox fake, e.g. Techtotop
    printGPSconf( "RMC,0,5" ); // Rec minimum
    printGPSconf( "GSA,0,5" ); // # Active sats
    printGPSconf( "GGA,0,0" ); // Essential fix data
    printGPSconf( "GSV,0,0" ); // Sats in view - several messages, slow this down
    printGPSconf( "GLL,0,0" ); // Old lat/long & fix time
    printGPSconf( "VTG,0,0" ); // Velocity
  }
}

void printGPSconf( char *str ) {
  char cat[30];
  strcpy( cat, "$PUBX,40,"); 
  strcat( cat, str); 
  strcat( cat, ",0,0,0,0*");
  printGPS( cat);
}

// Print NMEA Message to GPSSerial
void printGPS( char *str) {
  if (LogGPS) Serial.print( "G Send: ");
  if (LogGPS) Serial.print( str);
  pGPSSerial->print( str);
  int cs = checksum( str);
  // 2 digit hex checksum and CRLF = FF\r\n
  if ( !(cs & 0xF0) ) {
    if (LogGPS) Serial.print( '0');
    pGPSSerial->print( "0");
  }
  if (LogGPS) Serial.print( cs & 0xFF , 16);
  pGPSSerial->print( cs & 0xFF , 16);
  if (LogGPS) Serial.print( F("\r\n") );
  pGPSSerial->print( "\r\n" );
}

// Checksum xor of all between $ and *, not incl.
int checksum( char *str) {
  int r = 0;
  // Omit 1st '$' and last '*' char
  for( char * x = str+1; *(x+1); x++) {
    r ^= *x;
  }
  return r;
}

// Read GPS serial data, set buffer smaller than max serial buffer
#define GPSBUFFERSIZE (_SS_MAX_RX_BUFF-30)
char GPSInputBuffer[ GPSBUFFERSIZE];
int GPSInputIndex = 0;
int ReadGPS() {
  if (pGPSSerial != NULL) {
    char x = 0;
    static unsigned long readtime;
    while ( pGPSSerial->available()) {
      x = pGPSSerial->read();
      if (x >= 32) {
        GPSInputBuffer[ GPSInputIndex++] = x;
        // Terminate string
        GPSInputBuffer[ GPSInputIndex] = 0;
        if (GPSInputIndex >= GPSBUFFERSIZE) {
          GPSInputIndex = 0; // Overflow
          if (LogGPS) Serial.println( F("G Overflow"));
          break;
        }
        // If $ then set read time
        if (x == '$') {
          readtime = millis();
        }
      } else {
        break; // Start processing at the end of each line
      }
    }
    // Are we at the end of a string
    if (GPSInputIndex > 0 && x == '\r') {
      if (LogGPS) Serial.print(F("G Received: "));
      if (LogGPS) Serial.println( GPSInputBuffer);
  
      // Parse
      GPSParse( GPSInputBuffer, readtime);
  
      GPSInputIndex = 0; // Reset
    }
  }
  return GPSInputIndex;
}

// Two digits to an int
int TwoDig2Int( char *in) {
  return ( (in[0]-'0') * 10 + (in[1]-'0') );
}

// GPS Parsing
// Now using non-library functions instead of TinyGPS, as we can get time quicker because we
// don't need a valid location too.
#define MAXFIELDS   24
#define MAXFIELDLEN 12
// Defining here to ensure we have the memory
char gpsdata[MAXFIELDS][MAXFIELDLEN];
char timefield[MAXFIELDLEN]; //HHmmss
char datefield[MAXFIELDLEN]; //ddMMyy
int satellitecount = 0;
unsigned long satelliteCountResetTime = 0;

void GPSParse( char *buffer, unsigned long readtime ) {
  int curField = 0;
  int curChar = 0;
  int curchecksum = 0;

  bool endmessage = false;
  for (char *p = buffer; *p != 0; p++) {
    // Process each character received as an array of fields and strings
    // Check field length
    if (curChar >= MAXFIELDLEN) {
      // Too many, give up
      endmessage = true;
      break;
    }
    // Field separators are , and * for the checksum
    if (*p == ',' || *p == '*') {
      // End this field and start next field
      gpsdata[curField][curChar] = 0;
      curField++;
      curChar = 0;
      curchecksum ^= *p;
      if (curField >= MAXFIELDS) {
        // Too many, give up
        endmessage = true;
        break;
      }
    } else {
      // Save current character
      if (!endmessage) {
        gpsdata[curField][curChar] = *p;
        curChar++;
        gpsdata[curField][curChar] = 0;
        curchecksum ^= *p;
      }
    }
  }
  bool validGPS = false;
  if (!endmessage) {
    // Checksum - correct by 'removing' $ * and checksum
    curchecksum ^= '$' ^ '*' ^ gpsdata[curField][0] ^ gpsdata[curField][1];
    if (LogGPS) Serial.print( F("G Parsed: "));
    for (int i = 0; i <= curField; i++) {
      if (LogGPS) {
        Serial.print( i);
        Serial.print( "=");
        Serial.print( gpsdata[i]);
        Serial.print( ",");
      }
    }
    if (LogGPS) Serial.print( F("chk="));
    if (LogGPS) Serial.println( curchecksum, 16);
    unsigned long c = strtol(gpsdata[curField], NULL, 16);
    if (curchecksum == c) {
      if (LogGPS) Serial.println( F("G GOOD"));
      validGPS = true;
    } else {
      if (LogGPS) Serial.println( F("G BAD"));
    }
  }

  // Phase 2 - Test for a complete line with Time and Date
  if ( validGPS) {
    // Need 13 fields inc checksum
    // $GNRMC or
    // $GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70
    //    0     1    2    3    4    5     6    7    8      9     10  11 12
    // Alternate form, next field # will be 14, this used when regaining comms
    ///$GPRMC,221641.00,V, , , , , , ,261020,  ,  ,N*78
    ///   0     1       2 3 4 5 6 7 8   9    10 11 12 13
    if ((curField == 13) || (curField == 14)) {
      // First field is GPRMC or GNRMC
      //Serial.print(F("Type, "));
      //Serial.println( gpsdata[0] );

      if (!strcmp( "$GPRMC", gpsdata[0] ) || !strcmp( "$GNRMC", gpsdata[0] ) ) {
        if (LogGPS) Serial.println(F("G GPRMC/GNRMC"));

        // OK so far, copy Date and Time fields
        strcpy( timefield, gpsdata[1] );
        strcpy( datefield, gpsdata[9] );
        if (LogGPS) {
          Serial.print(F("G Read Time, "));
          Serial.print( readtime);
          Serial.print(F(","));
          Serial.print( datefield); // ddmmyy
          Serial.print(F(","));
          Serial.print( timefield); // HHMMSS.cc
          Serial.print(F(","));
          Serial.println(gpsdata[2]); // Type A
        }
        // Set GPS Time
        GPSnow = RtcDateTime( TwoDig2Int(datefield+4),
                              TwoDig2Int(datefield+2),
                              TwoDig2Int(datefield),
                              TwoDig2Int(timefield),
                              TwoDig2Int(timefield+2),
                              TwoDig2Int(timefield+4));
        // Check GPS Health
        if ( (GPSnow.Year() > 2023     ) &&
             (GPSnow.Month() <= 12     ) &&
             (GPSnow.Day() <= 31       )    ) {
          // Drive time into the system: setTimeAndOffset();
          GPSok = true;
          GPSTimeValidMillis = readtime;
          if (LogStatus) {
            Serial.print( F("S GPS Time: " ) );
            PrintNow( GPSnow, GPSTimeValidMillis);
          }
        } else {
          if (LogError) Serial.println( F("E GPS time invalid" ) );
        }
      }
    }

    // Need 19 fields inc checksum
    // Or $GNGSA
    // $GPGSA,A,3,04,05,06,09,12,13,14,24,25,26,27,28,2.5,1.3,2.1*39
    //    0   1 2  3  4  5  6  7  8  9 10 11 12 13 14  15  16  17 18
    //          Fix type 1=none
    //            12 sat slots-----------------------  PDop HDop VDop

    if (curField == 19) {
      //Serial.print(F("GPGSA"));
      //Serial.print(curField);
      if (!strcmp( "$GPGSA", gpsdata[0] ) || !strcmp( "$GNGSA", gpsdata[0] ) ) {
        if (LogGPS) Serial.println( F("G GPGSA or GNGSA"));

        // OK so far, get # satellites in field 3-14
        satellitecount = 0;
        for( int i = 3; i < 15; i++) {
          if ( gpsdata[i][0] != 0) {
            satellitecount++;
          }
        }
        if (LogStatus) {
          Serial.print(F("S Sats, "));
          Serial.print( satellitecount );
          Serial.print(F(",Type,"));
          Serial.print( gpsdata[1] );
          Serial.print(F(",Fix,"));
          Serial.print( gpsdata[2] );
          Serial.print(F(",Prec,"));
          Serial.println( gpsdata[15]);
        }
        // Remember the max satellites
        if (satellitecount > satelliteMaxIn10m) {
          satelliteMaxIn10m = satellitecount;
        }
        // Reset max every 10 minutes
        if (millis() - satelliteCountResetTime > 600000L) {
          satelliteMaxIn10m = satellitecount;
          satelliteCountResetTime = millis();
        }
      }
    }
  }
}
