
/*******************************************************************************

Glorified calibration test of bare conductive book project.
  sensitivity range via 
     MPR121.setTouchThreshold() / MPR121.setReleaseThreshold();
in setup()
mpr121 docs : https://github.com/BareConductive/mpr121

*******************************************************************************/

// compiler error handling
#include "Compiler_Errors.h"

// touch includes
#include <MPR121.h>
#include <Wire.h>
#define MPR121_ADDR 0x5C
#define MPR121_INT 4

// mp3 includes
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <SFEMP3Shield.h>

// mp3 variables
SFEMP3Shield MP3player;
byte result;
int lastPlayed = 0; // track which track we last played

// touch behaviour definitions
#define firstPin 0
#define lastPin 11

// sd card instantiation
SdFat sd;

// define LED_BUILTIN for older versions of Arduino
#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

void setup() {
  Serial.begin(57600);

  pinMode(LED_BUILTIN, OUTPUT);

  while (!Serial) ; {} //uncomment when using the serial monitor
  Serial.println("Bare Conductive Proximity MP3 player");

  if (!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt();

  if (!MPR121.begin(MPR121_ADDR)) Serial.println("error setting up MPR121");
  MPR121.setInterruptPin(MPR121_INT);

  // Changes from Touch MP3

  // this is the touch threshold - setting it low makes it more like a proximity trigger
  // default value is 40 for touch
  MPR121.setTouchThreshold(20);

  // this is the release threshold - must ALWAYS be smaller than the touch threshold
  // default value is 20 for touch
  MPR121.setReleaseThreshold(7);

  result = MP3player.begin();
  MP3player.setVolume(10, 10);

  if (result != 0) {
    Serial.print("Error starting mp3 player, code: ");
    Serial.println(result);
  }

}

void loop() {
  readTouchInputs();
}


void readTouchInputs() {
  if (MPR121.touchStatusChanged()) {

    MPR121.updateTouchData();

    // only make an action if we have one or fewer pins touched
    // ignore multiple touches

    if (MPR121.getNumTouches() <= 1) {

      /////////////////////////////////////////////////
      // dicks debug; read all pins and output them
      Serial.println("++++ dicks debug ++++");
      Serial.println(" 0  1  2  3  4  5  6  7  8  9  a  b  ");
      for (int i = 0; i < 12; i++) {
        if (MPR121.isNewTouch(i)) {
          Serial.print(" + ");
        } else if (MPR121.isNewRelease(i)) {
          Serial.print(" - ");
        } else {
          Serial.print(" . ");
        }
      }
      Serial.println("+++++ end debug +++++");
      /////////////////////////////////////////////////


      for (int i = 0; i < 12; i++) { // Check which electrodes were pressed
        if (MPR121.isNewTouch(i)) {

          //pin i was just touched
          Serial.print("touch on pin ");
          Serial.println(i);

          digitalWrite(LED_BUILTIN, HIGH);

          if (i <= lastPin && i >= firstPin) {
            if (MP3player.isPlaying()) {
              if (lastPlayed == i) {
                // if we're already playing the requested track, stop it
                MP3player.stopTrack();
                Serial.print("stopping track ");
                Serial.println(i - firstPin);
              } else {
                // if we're already playing a different track, stop that
                // one and play the newly requested one
                MP3player.stopTrack();
                MP3player.playTrack(i - firstPin);
                Serial.print("playing track ");
                Serial.println(i - firstPin);

                // don't forget to update lastPlayed - without it we don't
                // have a history
                lastPlayed = i;
              }
            } else {
              // if we're playing nothing, play the requested track
              // and update lastplayed
              MP3player.playTrack(i - firstPin);
              Serial.print("playing track ");
              Serial.println(i - firstPin);
              lastPlayed = i;
            }
          }
        } else {
          if (MPR121.isNewRelease(i)) {
            Serial.print("touch stopped on pin ");
            Serial.println(i);
            digitalWrite(LED_BUILTIN, LOW);
          }
        }
      }
    }
  }
}
