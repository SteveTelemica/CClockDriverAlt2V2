// ************************************************* OLED 128x64 *************************
#ifdef OLED_DISPLAY

void OLEDInit() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println( F("SSD1306 allocation failed"));
    // Continue, even though display will not work
  }
}

void OLEDRefresh() {
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Landscape 128*64
  // 21 Size 1 characters across, 8 rows (6 pix across, 8 down)
  //    012345678901234567890
  // 0  NoRTC NoGPS Sat 99 B
  // 1  2024 12 31 12 59 59
  // 2  11:59 >> 00:59 Fwd/Back
  // 3  St 999999   uS 10000
  //                 X    Y
  // Clock State
  // Four line format:
  display.setCursor( 2*6, 0*8);     // Start at top-left corner
  display.print(F("RTC"));
  display.setCursor( 0*6, 0*8);
  if (!RTCok) {
    display.print(F("No"));
  }
  display.setCursor( 8*6, 0*8);
  display.print(F("GPS"));
  display.setCursor( 6*6, 0*8);
  if (!GPSok) {
    display.print(F("No"));
  }
  // Satellite
  display.setCursor(12*6, 0*8);
  display.print( F("Sat") );
  display.setCursor(16*6, 0*8);
  display.print( satelliteMaxIn10m );
  // BST
  display.setCursor(19*6, 0*8);
  if (globalBST) {
    display.print('B');
  } else {
    display.print('G');
  }

  // Date/Time (BST)
  display.setCursor( 0*6, 1*8);
  display.print( realTime.Year());
  display.setCursor( 5*6, 1*8);
  display.print( realTime.Month());
  display.setCursor( 8*6, 1*8);
  display.print( realTime.Day());
  display.setCursor(11*6, 1*8);
  display.print( realTime.Hour());
  display.setCursor(14*6, 1*8);
  display.print( realTime.Minute());
  display.setCursor(17*6, 1*8);
  display.print( realTime.Second());
  // Hand position Actual >> Target
  display.setCursor( 0*6, 2*8);
  display.print(handpositionmin / 60);
  display.setCursor( 2*6, 2*8);
  display.print(':');
  display.setCursor( 3*6, 2*8);
  display.print(handpositionmin % 60);
  display.setCursor( 6*6, 2*8);
  display.print(F(">>"));
  display.setCursor( 9*6, 2*8);
  display.print(minutesValue / 60);
  display.setCursor(11*6, 2*8);
  display.print(':');
  display.setCursor(12*6, 2*8);
  display.print(minutesValue % 60);
  display.setCursor(15*6, 2*8);
  if (!stepBack) {
    display.print(F("Fwd"));
  } else {
    display.print(F("Back"));
  }
  // Steps and interval
  display.setCursor( 0*6, 3*8);
  display.print(F("St"));
  display.setCursor( 3*6, 3*8);
  display.print(stepsNeeded);
  display.setCursor(12*6, 3*8);
  display.print(F("uS "));
  display.setCursor(15*6, 3*8);
  display.print(StepInterval);

#ifdef EIGHTLINEFORMAT
  display.setCursor( 0*6, 0*8);     // Start at top-left corner
  display.print(F("RTC"));
  display.setCursor( 4*6, 0*8);
  if (RTCok) {
    display.print(F("OK"));
  } else {
    display.print(F("Fail"));
  }
  display.setCursor(11*6, 0*8);
  display.print(F("GPS"));
  display.setCursor(15*6, 0*8);
  if (GPSok) {
    display.print(F("OK"));
  } else {
    display.print(F("Fail"));
  }
  // Date/Time (BST)
  display.setCursor( 0*6, 1*8);
  display.print( realTime.Year());
  display.setCursor( 5*6, 1*8);
  display.print( realTime.Month());
  display.setCursor( 8*6, 1*8);
  display.print( realTime.Day());
  display.setCursor(11*6, 1*8);
  display.print( realTime.Hour());
  display.setCursor(14*6, 1*8);
  display.print( realTime.Minute());
  display.setCursor(17*6, 1*8);
  display.print( realTime.Second());
  // Satellite and Accuracy
  display.setCursor( 0*6, 2*8);
  display.print( F("Sat") );
  display.setCursor( 4*6, 2*8);
  display.print( satelliteMaxIn10m );
  display.setCursor( 8*6, 2*8);
  display.print( F("MilAdj") );
  display.setCursor(15*6, 2*8);
  display.print( millisAdjustment );
  // Steps, dir and interval
  display.setCursor( 0*6, 3*8);
  display.print(F("Steps"));
  display.setCursor( 6*6, 3*8);
  display.print(stepsNeeded);
  display.setCursor(16*6, 3*8);
  if (!stepBack) {
    display.print(F("Fwd"));
  } else {
    display.print(F("Back"));
  }
  display.setCursor( 0*6, 4*8);
  display.print(F("Do"));
  display.setCursor( 3*6, 4*8);
  display.print(StepsToDo);
  display.setCursor( 9*6, 4*8);
  display.print(F("uSec"));
  display.setCursor(14*6, 4*8);
  display.print(StepInterval);
  // Hand position Target and Actual
  display.setCursor( 0*6, 5*8);
  display.print(F("Target"));
  display.setCursor( 8*6, 5*8);
  display.print(minutesValue / 60);
  display.setCursor(10*6, 5*8);
  display.print(':');
  display.setCursor(11*6, 5*8);
  display.print(minutesValue % 60);
  display.setCursor( 0*6, 6*8);
  display.print(F("Actual"));
  display.setCursor( 8*6, 6*8);
  display.print(handpositionmin / 60);
  display.setCursor(10*6, 6*8);
  display.print(':');
  display.setCursor(11*6, 6*8);
  display.print(handpositionmin % 60);
#endif
  display.display();
}

#endif
