void setup() {
  Serial.begin(115200); // Faster to reduce disruption than 9600
  Serial.print( F("CClockDriverAlt2V2. Compiled time: ") );
  Serial.print( F(__DATE__));
  Serial.print( " ");
  Serial.println( F(__TIME__));
  Serial.println( F("Type: 'SApr 23 2024 19:38:14' to set RTC") );
  Serial.println( F("Type: 'LG' 'LR' or 'LM' to toggle logs for GPS, RTC or Motor") );

  printStepInfo();

  // Input forward/reverse switches
  pinMode(PIN_OUT_LOW, OUTPUT);
  digitalWrite(PIN_OUT_LOW, LOW);
  pinMode(PIN_IN_FWD, INPUT_PULLUP);
  pinMode(PIN_IN_REV, INPUT_PULLUP);
  pinMode(PIN_OUT_HIGH, OUTPUT);
  digitalWrite(PIN_OUT_HIGH, HIGH);


  setupEEPROM( &handpositionmin, &handpositionstep);

  setupRTC();
  
  StartGPS();
  setupGPS();

  SetupMotor();

#ifdef OLED_DISPLAY
  OLEDInit(); 
#endif

  // Variable 'now' should default to any date, but set time from EEPROM value
  // So it carries on from where it left off after power cut
  // Then it will 
  // (Use 1 Jan to avoid BST issues)
  now = RtcDateTime( 00,
                     01,
                     01,
                     handpositionmin / 60,
                     handpositionmin % 60,
                     0);
}
