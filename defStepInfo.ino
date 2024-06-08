// Time and Motor Constants
// Est speed needed = 60 rph = 1 rpm = 1/60 rps
// off off off = 6400 steps per rev, but for A4988, On,On,On = 1/16 Step, at 200 step/rev -> 3200 step/rev
// MICROSTEPS is 32 or 16
// 6400*1 steps per minute or 3200
#define STEPSPERMINUTE (200L)*(MICROSTEPS) // 6400L or 3200L for A3988
// 6400 / 60 steps per second = 106.7 s/sec, or 3200 / 60 step/sec = 53
#define STEPSPERSECOND ((STEPSPERMINUTE)/60L) // 106 or 53
// Milliseconds per step 9.4 (approx) or 20 approx
#define MICROSPERSTEP (1000000L/STEPSPERSECOND)
// 1/106.7s = 9400uS step interval
// Max speed timing, 30 times faster
#define MICROSPERSTEPMIN (MICROSPERSTEP/30L)
// Time related constants
#define MINUTESPERDAY 1440
#define MINUTESPER12H  720

void printStepInfo() {
  Serial.print( F("STEPSPERMINUTE "));
  Serial.println( STEPSPERMINUTE );
  Serial.print( F("STEPSPERSECOND "));
  Serial.println( STEPSPERSECOND );
  Serial.print( F("MICROSPERSTEP "));
  Serial.println( MICROSPERSTEP );
  Serial.print( F("MICROSPERSTEPMIN "));
  Serial.println( MICROSPERSTEPMIN );
}
