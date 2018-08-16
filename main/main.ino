#include <Adafruit_BLEEddystone.h>
#include <Adafruit_BLEMIDI.h>
#include <Adafruit_BLEGatt.h>
#include <Adafruit_BluefruitLE_UART.h>
#include <Adafruit_BLEBattery.h>
#include <Adafruit_ATParser.h>
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>

#include <Adafruit_VS1053.h>
#include <SD.h>
#include <Adafruit_NeoPixel.h>

#include "BluefruitConfig.h"

#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

const int RESET_BUTTON = A3;
const int SOUND_BUTTON = A4;
const int LIGHT_BUTTON = A5;
const int DONE_PIN = A2;
const int LED_PIN_1 = 12;
const int LED_PIN_2 = 11;
const int NUM_LEDS = 4;

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

Adafruit_NeoPixel strips[2] = {Adafruit_NeoPixel(NUM_LEDS, LED_PIN_1, NEO_GRB + NEO_KHZ800),Adafruit_NeoPixel(NUM_LEDS, LED_PIN_2, NEO_GRB + NEO_KHZ800)};
#define VS1053_RESET   -1
#define VS1053_CS       6     // VS1053 chip select pin (output)
#define VS1053_DCS     10     // VS1053 Data/command select pin (output)
#define CARDCS          5     // Card chip select pin
// DREQ should be an Int pin *if possible* (not possible on 32u4)
#define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void commLoop() {
  bool done = false;
  char toSend[BUFSIZE+1];
  int count = 0;
  for (int i = 0; i < BUFSIZE; i++) {
    toSend[i] = (char)(65 + (i % 26));
  }
  while (!done) {
    Serial.print("[Send] ");
    Serial.println(toSend);
    ble.print("AT+BLEUARTTX=");
    ble.println(toSend);

    // check response stastus
    if (! ble.waitForOK() ) {
      Serial.println(F("Failed to send?"));
    }
    if (++count == 10) {
      done = true;
    }
  }
}

void setup() {
  /*
   * DEBUG CODE
   */

  //while ( ! Serial ) { delay( 1 ); }
  Serial.begin(9600);

 
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("******************************"));
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
    Serial.println(F("******************************"));
  }

  delay(5000);

  commLoop();

  pinMode(BLUEFRUIT_SPI_CS, OUTPUT);
  digitalWrite(BLUEFRUIT_SPI_CS, HIGH);

  Serial.println("setup!!!");
  if (! musicPlayer.begin()) { // initialise the music player
   error(F("Couldn't find VS1053, do you have the right pins defined?"));
  }
 
  strips[0].begin();
  strips[1].begin();
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      strips[i].setPixelColor(j, 0, 0, 0); //pixel num, r, g, b
    }
    strips[i].show();
  }

  delay(1000);
  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  if (!SD.begin(CARDCS)) {
    error(F("SD failed, or not present"));
  }
  Serial.println("SD OK!");
  
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      strips[i].setPixelColor(j, 255, 255, 255); //pixel num, r, g, b
    }
    strips[i].show();
  }

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
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < NUM_LEDS; j++) {
        strips[i].setPixelColor(j, 0, 0, 0); //pixel num, r, g, b
      }
      strips[i].show();
    }
    musicPlayer.stopPlaying();
    while (1) {
      digitalWrite(DONE_PIN, HIGH);
      delay(1);
      digitalWrite(DONE_PIN, LOW);
      delay(1);
    }
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < NUM_LEDS; j++) {
        strips[i].setPixelColor(j, 255, 0, 0); //pixel num, r, g, b
      }
      strips[i].show();
    }
  }

  if (digitalRead(SOUND_BUTTON) == LOW) {
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < NUM_LEDS; j++) {
        strips[i].setPixelColor(j, 0, 255, 0); //pixel num, r, g, b
      }
      strips[i].show();
    }
  }

  if (digitalRead(LIGHT_BUTTON) == LOW) {
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < NUM_LEDS; j++) {
        strips[i].setPixelColor(j, 255, 255, 0); //pixel num, r, g, b
      }
      strips[i].show();
    }
  }

}

