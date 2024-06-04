// EEPROM CODE - write minute value to a word to remember hand position
// To reduce EEPROM wear, increment mem location each daty
// 900 Locations, 450 Words  100K / 1440 * 450 = 31K days = 86 years minimum
// Write each word 0-1439, then xFFFF when done
// Scan from start looking for non-FFFF values
// Restart at beginning if they are all xFFFF 

// Persistent mem
#include <EEPROM.h>

#define EEPROM_INIT       0x25 // If address 0 is set to this, then EEPROM has been set up
#define EEPROM_BLOCKSTART   20 // Start byte address
#define EEPROM_BLOCKLENGTH 900 // # of Bytes
int eeprom_block_address = EEPROM_BLOCKSTART;

void saveInitialSettings() {
  Serial.println(F("EE INIT START"));
  EEPROM.write(0, EEPROM_INIT);
  for (int i = EEPROM_BLOCKSTART; i < EEPROM_BLOCKSTART + EEPROM_BLOCKLENGTH; i=i+2) {
    Serial.print(F("Clear block address, "));
    Serial.println( i);
    EEPROM.write(i, 255);
    EEPROM.write(i+1, 255);
  }
  // Write settings here
  Serial.println(F("EE INIT END"));
}

void setupEEPROM( int *handposmin, int *handposstep) {
  byte v = EEPROM.read(0);
  if (v == EEPROM_INIT) {
    // Read data here
    Serial.println(F("* EE OK, scanning"));
    for (int i = EEPROM_BLOCKSTART; i < EEPROM_BLOCKSTART + EEPROM_BLOCKLENGTH; i=i+2) {
      Serial.print(F("Block address, "));
      Serial.println( i);
      
      int pos = ((int)EEPROM.read(i) | (((int)EEPROM.read(i+1))<<8) );
      Serial.print(F("Hand Position, "));
      Serial.println( pos);

      if ( (pos >= 0) && (pos < MINUTESPER12H) ) {
        // First non-'spent' valid entry
        eeprom_block_address = i;
        Serial.println(F("Set hand pos from EEPROM"));
        // Valid hand position read from EEPROM
        *handposmin = pos;
        Serial.print(F("Hand Position, "));
        PrintHandPos( *handposmin, 0 );
        // Set step position to 30 seconds to average the start position out
        *handposstep = STEPSPERMINUTE / 2L;
        break;
      }
    }
  } else {
    Serial.println(F("** No EE, saving id"));
    saveInitialSettings();
  }
}

void writeHandPosition( int pos) {
  if (pos == 0) {
    if (LogStatus) Serial.println(F("S New Day") );
    // Cancel last used location
    EEPROM.write(eeprom_block_address, 255);
    EEPROM.write(eeprom_block_address+1, 255);
    // Move to next EEPROM location
    eeprom_block_address = eeprom_block_address + 2;
    if (eeprom_block_address >= EEPROM_BLOCKLENGTH + EEPROM_BLOCKSTART) {
      // Back to the beginning
      if (LogStatus) Serial.println( F( "S EEPROM Rollover") );
      eeprom_block_address = EEPROM_BLOCKSTART;
    }
  }
  // Write new position
  if (LogStatus) {
    Serial.print( F( "S Write EEPROM, ") );
    PrintHandPos( pos, 0);
  }
  EEPROM.write( eeprom_block_address, pos & 255);
  EEPROM.write( eeprom_block_address+1, pos >> 8);
}

void PrintHandPos( unsigned int pos, int steps)
{
  Serial.print( pos );
  Serial.print( ',' );
  Serial.print( pos / 60);
  Serial.print( ':' );
  Serial.print( pos % 60 );
  Serial.print( '+' );
  Serial.println( steps );
}
