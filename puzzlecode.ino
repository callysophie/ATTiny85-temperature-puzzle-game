/*
ATTiny85 based puzzle game

Plays Happy Birthday when temperature goes below a given figure
Displays a Happy Birthday message when temperature goes above a given number 

https://github.com/callysophie/ATTiny85-temperature-puzzle-game

Cally

*/

// Includes necessary libraries
#include <OneWire.h>
#include <DS18B20.h>
#include <TinyWireM.h>
#include <Tiny4kOLED.h>

// Defines pins

#define SPEAKER_PIN         (1) // Can be 1 or 4
#define ONE_WIRE_BUS PB3

// Sets up DS18B20 sensor

OneWire oneWire(ONE_WIRE_BUS);
DS18B20 sensor(&oneWire);



// Notes for song
#define NOTE_C  (0)
#define NOTE_CS (1)
#define NOTE_D  (2)
#define NOTE_DS (3)
#define NOTE_E  (4)
#define NOTE_F  (5)
#define NOTE_FS (6)
#define NOTE_G  (7)
#define NOTE_GS (8)
#define NOTE_A  (9)
#define NOTE_AS (10)
#define NOTE_B  (11)

// Cater for 16 MHz, 8 MHz, or 1 MHz clock
const int Clock = ((F_CPU/1000000UL) == 16) ? 4 : ((F_CPU/1000000UL) == 8) ? 3 : 0;
const uint8_t scale[] PROGMEM = {239,226,213,201,190,179,169,160,151,142,134,127};

uint8_t location = 0;
bool leftToRight = false;

const int Output = 1; // Can be 1 or 4
int bpm, beat_every, dur2, dur4, dur8, dur8dot, dur16;


void note (int n, int octave) {
  int prescaler = 8 + Clock - (octave + n/12);
  if (prescaler<1 || prescaler>15 || octave==0) prescaler = 0;
  DDRB = (DDRB & ~(1<<Output)) | (prescaler != 0)<<Output;
  OCR1C = pgm_read_byte(&scale[n % 12]) - 1;
  GTCCR = (Output == 4)<<COM1B0;
  TCCR1 = 1<<CTC1 | (Output == 1)<<COM1A0 | prescaler<<CS10;
}

int calculateDuration(int beatEvery, int noteDuration, bool isDotted) {
  int duration = (beatEvery * 4) / noteDuration;
  int prolonged = isDotted ? (duration / 2) : 0;
  return duration + prolonged;
}

void setup() {
  
  // Starts sensor
  sensor.begin();
  sensor.setResolution(9);
  // Starts pins
  pinMode(SPEAKER_PIN, OUTPUT);

  // Initialize the wire library, but do not send any initialization sequence to the oled.
  // This leaves the screen area being the default for 128x64 screens
//  oled.begin();
  oled.begin(0,0);

  // The default state of the SSD1306 does not turn on the internal charge pump
  oled.enableChargePump();

  // Two rotations are supported,
  // The begin() method sets the rotation to 1,
  // but the begin(0,0) method doe not include that initialization.
  oled.setRotation(1);

  // In order to use double buffering on the 128x64 screen,
  // the zoom feature needs to be used, which doubles the height
  // of all pixels.
  oled.enableZoomIn();

  // Two fonts are supplied with this library, FONT8X16 and FONT6X8
  // Other fonts are available from the TinyOLED-Fonts library
  // This example only uses a single font, so it can be set once here.
  // The characters in the 8x16 font are 8 pixels wide and 16 pixels tall.
  // 2 lines of 16 characters exactly fills 128x32.
  oled.setFont(FONT8X16);

  // Setup the first half of memory.
  updateDisplay();

  // Turn on the display.
  oled.clear();
  oled.on();
  
}

// Happy Birthday melody 
void melody2() {
  int bpm = 125;
  int beat_every = 60000 / bpm;
  int dur2 = calculateDuration(beat_every, 2, false);
  int dur4 = calculateDuration(beat_every, 4, false);
  int dur8 = calculateDuration(beat_every, 8, false);
  int dur8dot = calculateDuration(beat_every, 8, true);
  int dur16 = calculateDuration(beat_every, 16, false);

  note(NOTE_D, 5); delay(dur8dot); note(0, 0);
  note(NOTE_D, 5); delay(dur16); note(0, 0);
  note(NOTE_E, 5); delay(dur4); note(0, 0);
  note(NOTE_D, 5); delay(dur4); note(0, 0);
  note(NOTE_G, 5); delay(dur4); note(0, 0);
  note(NOTE_FS, 5); delay(dur2); note(0, 0);

  note(NOTE_D, 5); delay(dur8dot); note(0, 0);
  note(NOTE_D, 5); delay(dur16); note(0, 0);
  note(NOTE_E, 5); delay(dur4); note(0, 0);
  note(NOTE_D, 5); delay(dur4); note(0, 0);
  note(NOTE_A, 5); delay(dur4); note(0, 0);
  note(NOTE_G, 5); delay(dur2); note(0, 0);

  note(NOTE_D, 5); delay(dur8dot); note(0, 0);
  note(NOTE_D, 5); delay(dur16); note(0, 0);
  note(NOTE_D, 6); delay(dur4); note(0, 0);
  note(NOTE_B, 5); delay(dur4); note(0, 0);
  note(NOTE_G, 5); delay(dur4); note(0, 0);
  note(NOTE_FS, 5); delay(dur4); note(0, 0);
  note(NOTE_E, 5); delay(dur2); note(0, 0);

  note(NOTE_G, 6); delay(dur8dot); note(0, 0);
  note(NOTE_C, 6); delay(dur16); note(0, 0);
  note(NOTE_B, 5); delay(dur4); note(0, 0);
  note(NOTE_G, 5); delay(dur4); note(0, 0);
  note(NOTE_A, 5); delay(dur4); note(0, 0);
  note(NOTE_G, 5); delay(dur2); note(0, 0);
}

void loop() {

oled.on();

  // Begins sensor and requests temperature
  sensor.begin();    
  sensor.requestTemperatures();
  // Waits for sensor to send result
  while (!sensor.isConversionComplete());  
  // Turns on OLED and runs updateDisplay function if sensor temp is below a certain amount
  if (sensor.getTempC() <= 15) {
    oled.clear();
    oled.on();   
    updateDisplay();
    delay(1000);
  // Turns off OLED if sensor temp above that amount 
  } else {
      oled.clear();
      oled.off();
  }
  // Runs melody2 function if sensor temp is above a certain amount
  if (sensor.getTempC() >= 28) {
    // It's useful to include an LED to help debug
    digitalWrite(PB4, HIGH);
    melody2();
  } else {
      digitalWrite(PB4, LOW);
  } 
  
  delay(1000);

 
}

void updateDisplay() {
  // Clear whatever random data has been left in memory.
  oled.clear();

  // Position the text cursor
  // In order to keep the library size small, text can only be positioned
  // with the top of the font aligned with one of the four 8 bit high RAM pages.
  // The Y value therefore can only have the value 0, 1, 2, or 3.
  // usage: oled.setCursor(X IN PIXELS, Y IN ROWS OF 8 PIXELS STARTING WITH 0);
  oled.setCursor(location, 0);

  // Write text to oled RAM.
  oled.print(F("Happy Birthday"));

  // Write it again
  oled.setCursor(location, 2);
  oled.print(F("   Niece!"));

  delay(250);
}
