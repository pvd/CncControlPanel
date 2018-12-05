#include <Adafruit_SSD1306.h>

#include <Wire.h>
#include "RotaryEncoder.h"
#include "ProbeBitmaps.h"

// Pins; also change the PCINT mask when changing the encoder pins
#define PIN_MPG_X_A A8
#define PIN_MPG_X_B A9

#define PIN_MPG_Y_A A10
#define PIN_MPG_Y_B A11

#define PIN_MPG_Z_A A12
#define PIN_MPG_Z_B A13

#define PIN_DISPLAY_RESET 4

// Constants
static const uint8_t SYNC_CHAR         = 0x55;
static const uint8_t TAIL_CHAR         = 0xAA;
static const unsigned long WD_TIMEOUT  = 1000;
static const uint16_t SYNC_SEARCH_LEDS = 0x1FFF;
static const uint16_t WD_LEDS          = 0x7FF;

//---------------------------------------------------------
struct TInStatus
{
  uint8_t  sync;  // Sync character
  uint16_t leds;  // Leds status
  uint8_t  tail; // Trail character
}__attribute__((packed));

struct TOutStatus
{
  uint8_t  sync;      // Sync character
  int32_t  mpgXcount; // Nr of MPG X pulses
  int32_t  mpgYcount; // Nr of MPG Y pulses
  int32_t  mpgZcount; // Nr of MPG Z pulses
  uint16_t buttons;   // Button status
  uint8_t  tail;      // Tail character
}__attribute__((packed));

//---------------------------------------------------------
RotaryEncoder    mpgX(PIN_MPG_X_A, PIN_MPG_X_B);
RotaryEncoder    mpgY(PIN_MPG_Y_A, PIN_MPG_Y_B);
RotaryEncoder    mpgZ(PIN_MPG_Z_A, PIN_MPG_Z_B);
Adafruit_SSD1306 probeDisplay(PIN_DISPLAY_RESET);  //128 x 64 pixels
TInStatus        inStatusRec;
TOutStatus       outStatusRec;
unsigned long    watchdog;     // simple watchdog by means of the time since the last received packet from linuxcnc

//---------------------------------------------------------
void UpdateLeds(uint16_t value)
{
  // the leds are all inverted
  value = ~value;

  // Update the leds via the two IO exanders
  Wire.beginTransmission(0x3E); 
  Wire.write( uint8_t(value & 0xFF) );              
  Wire.endTransmission();    

  Wire.beginTransmission(0x3F); 
  Wire.write( uint8_t(value >> 8 ) );              
  Wire.endTransmission();   
}

//---------------------------------------------------------
void SearchForSync()
{
  // Read data from the serial port until the sync character is found
  while ( Serial.peek() != SYNC_CHAR )
  {
    Serial.read();

    // Indicate that we are searching for the sync
    UpdateLeds(SYNC_SEARCH_LEDS);
    continue;
  }
}

//---------------------------------------------------------
void setup() 
{
  Serial.begin(115200);

  probeDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  probeDisplay.clearDisplay();

  probeDisplay.drawCircle(64, 32, 40, WHITE);
  probeDisplay.drawBitmap(60, 0, bmpArrowUp, 8, 16, 1);
  probeDisplay.drawBitmap(24, 28, bmpArrowLeft, 16, 8, 1);
  probeDisplay.drawBitmap(88, 28, bmpArrowRight, 16, 8, 1);
  probeDisplay.drawBitmap(60, 48, bmpArrowDown, 8, 16, 1);
  
  probeDisplay.display();

  PCICR |= (1 << PCIE2);    // This enables Pin Change Interrupt 2 that covers the Analog input pins 8..15 (PCINT16..PCINT23)
  PCMSK2 |= (1 << PCINT16) | (1 << PCINT17);  // PCINT 16 & 17
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);  // PCINT 18 & 19
  PCMSK2 |= (1 << PCINT20) | (1 << PCINT21);  // PCINT 20 & 21
 
  // All pins of PORT A & C are used for the buttons, make them all inputs
  DDRA = 0x0;
  DDRC = 0x0;

  // Enable the pullups on all the pins of PORT A & C
  PORTA = 0xFF;
  PORTC = 0xFF;

  // Init the out status record
  outStatusRec.sync      = SYNC_CHAR;
  outStatusRec.mpgXcount = 0;
  outStatusRec.mpgYcount = 0;
  outStatusRec.mpgZcount = 0;
  outStatusRec.buttons   = 0;
  outStatusRec.tail      = TAIL_CHAR;

  watchdog = millis();
  UpdateLeds(0xFFFF);
}

// The Interrupt Service Routine for Pin Change Interrupt 2
ISR(PCINT2_vect) 
{
  // Call tick of every MPG to check for a state change
  mpgX.tick(); // just call tick() to check the state.
  mpgY.tick(); // just call tick() to check the state.
  mpgZ.tick(); // just call tick() to check the state.
}

//---------------------------------------------------------
void loop()
{
  if ( Serial.available() >= sizeof(TInStatus) )
  {
    watchdog = millis();
    
    // Read the in status record
    Serial.readBytes(reinterpret_cast<char*>(&inStatusRec), sizeof(TInStatus));

    // Only use the incoming status when both the sync and tail match
    // When there is a mismatch, try to recync
    if ( (inStatusRec.sync == SYNC_CHAR) && (inStatusRec.tail == TAIL_CHAR) )
    {
      // Update the leds with new status record
      UpdateLeds(inStatusRec.leds);
  
      // Update the status record and transmit it 
      outStatusRec.mpgXcount = mpgX.getPosition();
      outStatusRec.mpgYcount = mpgY.getPosition();
      outStatusRec.mpgZcount = mpgZ.getPosition();
      outStatusRec.buttons   = PINC;
      outStatusRec.buttons   = (outStatusRec.buttons << 8) | (PINA);
        
      Serial.write(reinterpret_cast<char*>(&outStatusRec), sizeof(outStatusRec));
    }
    else  // somthing went wrong, out of sync, try to find the sync again
    {
      SearchForSync();
    }
  }

  if ( (millis() - watchdog) > WD_TIMEOUT )
  {
    UpdateLeds(WD_LEDS);
  }
}




