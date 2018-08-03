#include <Adafruit_VS1053.h>
#include <SD.h>
#include <I2S.h>
#include <Adafruit_NeoPixel.h>

const int RESET_BUTTON = A3;
const int SOUND_BUTTON = A4;
const int LIGHT_BUTTON = A5;
const int DONE_PIN = A2;

const int LED_PIN = 11;
const int NUM_LEDS = 32;

unsigned value = 1;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
#define VS1053_RESET   -1
#define VS1053_CS       6     // VS1053 chip select pin (output)
#define VS1053_DCS     10     // VS1053 Data/command select pin (output)
#define CARDCS          5     // Card chip select pin
// DREQ should be an Int pin *if possible* (not possible on 32u4)
#define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

void setup() {
  /*
   * DEBUG CODE
   */

 // while ( ! Serial ) { delay( 1 ); }
  Serial.begin(9600);

  Serial.println("setup!!!");
  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    // while (1);
  }
 
  strip.begin();
  
  for (int j = 0; j < NUM_LEDS; j++) {
    strip.setPixelColor(j, 0, 0, 0); //pixel num, r, g, b
  }
  strip.show();
  

  delay(1000);
  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    //while (1);  // don't do anything more
  }
  Serial.println("SD OK!");
  
  for (int j = 0; j < NUM_LEDS; j++) {
    strip.setPixelColor(j, 255, 255, 255); //pixel num, r, g, b
  }
  strip.show();
  

  musicPlayer.setVolume(2,2);
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
  musicPlayer.startPlayingFile("Sean.mp3");

  
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  pinMode(SOUND_BUTTON, INPUT_PULLUP);
  pinMode(LIGHT_BUTTON, INPUT_PULLUP);
  pinMode(DONE_PIN, OUTPUT);
  
  /*
   * BELOW CODE FOR ACTUAL ACTIVATION
   */
  //check sd card to get current state
  
  //check if power button is LOW
    //if off state then turn to on
      //play startup tone/sound and lights
      //set state
    //else
      //play shutdown ton/sound and lights
      //set state

  //if sound button is LOW
    //blink green 3 times
    //show current setting in lights
    //monitor pin changes for reconfig

  //if light button is LOW
    //blink yellow 3 times
    //show current setting in lights
    //monitor pin changes for reconfig

  //Get sleep count and next activation target from SD card
  //if time to activate
    //choose random sound file and start playing
    //choose random light file and start running
}



void loop() {
  // repeatedly set the DONE pin HIGH and LOW
  if (digitalRead(RESET_BUTTON) == LOW) {
    
   for (int j = 0; j < NUM_LEDS; j++) {
    strip.setPixelColor(j, 0,0,0); //pixel num, r, g, b
    }
    strip.show();

    musicPlayer.stopPlaying();

    while(1) {
  
      digitalWrite(DONE_PIN, HIGH);
      delay(1);
      digitalWrite(DONE_PIN, LOW);
      delay(1);
    }
    
  }

  

  if (digitalRead(SOUND_BUTTON) == LOW) {
     for (int j = 0; j < NUM_LEDS; j++) {
      strip.setPixelColor(j, 0, 255, 0); //pixel num, r, g, b
    }
    strip.show();
  }

  if (digitalRead(LIGHT_BUTTON) == LOW) {
     for (int j = 0; j < NUM_LEDS; j++) {
      strip.setPixelColor(j, 255, 255, 0); //pixel num, r, g, b
    }
    strip.show();
  }

}
