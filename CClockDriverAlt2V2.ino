// Church Clock Driver Alt 2 V1
// Rewrite for simplicity
// Use bridge motor drive
// S.Beadle, 22-12-19 & 21-08-22 & 21-04-24
// Arduino Pro Mini 5V
// BAUD RATE FOR DEBUG 115200
// OVERRIDE SoftwareSerial.h buffer
#define _SS_MAX_RX_BUFF 150

#define VERSION "Version Alt 2.2"
#define MICROSTEPS (16L) // This A4988 16, First sys is 32

// GPS on pins. L-R   Tx10 , Rx11 , 5v , Gnd
#define PIN_GPS_TX 10
#define PIN_GPS_RX 12 // 11 busted in tower!

// Status to PIN 13 LED
#define PIN_LED_OUT   13

// A4988: Dir=4, Step=5
// Tower: Stepper 2=Direction, 3=Pulse
#define PIN_STEP_DIR   4 // Blue
#define PIN_STEP_PULSE 5 // Orange

// Input signals to move the hands - both active low/pullup
#define PIN_OUT_LOW A0 // Convenient ground signal
#define PIN_IN_FWD  A1
#define PIN_IN_REV  A2
#define PIN_OUT_HIGH A3 // Handy +V signal

// RTC
// RST=8, DAT=7, CLK=6
#define PIN_RTC_CLK 6
#define PIN_RTC_DAT 7
#define PIN_RTC_RST 8

//#define  // Undefine to use the rinkydink alternative

#ifdef STD_DS1302
#include <Wire.h>
// Standard DS1302 Library
#include <ThreeWire.h>   // Support for 1302
#include <RtcDS1302.h>   // Support for 1302
ThreeWire myWire( PIN_RTC_DAT, PIN_RTC_CLK, PIN_RTC_RST ); // IO, SCLK, CE
RtcDS1302<ThreeWire> RTC(myWire);
#else
// Try DS1302 lib http://www.rinkydinkelectronics.com/library.php?id=5
#include <DS1302.h>
#include <RtcDateTime.h> // From the RtcDS1302 library
DS1302 RTC(PIN_RTC_RST, PIN_RTC_DAT, PIN_RTC_CLK); // CE, IO, CLK);
#endif

// Customise this and set buffer size to 150 characters
#include <SoftwareSerial.h>

// Logging Control
bool LogGPS = false;
bool LogRTC = true;
bool LogMotor = false;
bool LogError = true;
bool LogStatus = true;
bool LogDisplay = true;
// Type LG, LR, LM, LE, LS, LD, LA(ll) to enable logging, 
//      Lg, Lr, Lm, Le, Ls, Ld, La(ll) to disable.

SoftwareSerial *pGPSSerial = NULL;


// ************************************************* OLED 128x64 *************************
// Optional Display
#define OLED_DISPLAY
#ifdef OLED_DISPLAY
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#define SSD1306_NO_SPLASH
#define NO_ADAFRUIT_SSD1306_COLOR_COMPATIBILITY
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (A4=SDA, A5=SCL pins) ESP32 SDA=D21, SCL=D22
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 display(OLED_RESET);
#endif
// *****************************************************************************************
unsigned long globalMillis = 0;
#define STARTMILLIS 0x0UL // 0xFFF6D83FUL // define to test rollovers of millis(), set to 0 normally
RtcDateTime RTCnow;
bool RTCok = false;
unsigned long RTCTimeValidMillis = 0;
RtcDateTime GPSnow;
bool GPSok = false;
unsigned long GPSTimeValidMillis = 0;
RtcDateTime now;
// When we have a valid time, either RTC or GPS
bool Timeok = false;
unsigned long TimeValidMillis = STARTMILLIS;
// Use GPS time to calibrate millis() hourly
RtcDateTime GPSref;
unsigned long GPSTimeReference = 0;
int satelliteMaxIn10m = 0;
// Local time for display
RtcDateTime realTime;
// Reference time check
signed long millisAdjustment = 0;

// For display status view
long StepsToDo = 0;
long StepInterval = 0;
bool stepBack = false;
long stepsNeeded = 0;
bool globalBST = false;
// Value verified, unless -1
int minutesValue;
int stepValue;

// Motor control
// Current hand position invalid
// Minutes from midnight
int handpositionmin = -1;
// Steps from minute
int handpositionstep = 0;


void PrintNow( RtcDateTime n, unsigned long valid) {
  // Print what we read
  Serial.print( ' ' );
  Serial.print( n.Day() );
  Serial.print( '/' );
  Serial.print( n.Month() );
  Serial.print( '/' );
  Serial.print( n.Year() );
  Serial.print( ' ' );
  Serial.print( n.Hour() );
  Serial.print( ':' );
  Serial.print( n.Minute() );
  Serial.print( ':' );
  Serial.print( n.Second() );
  Serial.print( '+' );
  Serial.print( globalMillis - valid );
  Serial.print( F("ms at millis ") );
  Serial.println( globalMillis );
}
