
/*******************************************************************************

Glorified calibration test of bare conductive book project.

  sensitivity range in setup_touchpad()

mpr121 (touchboard) docs : https://github.com/BareConductive/mpr121
sfe mp3 shield docs : http://mpflaga.github.io/Sparkfun-MP3-Player-Shield-Arduino-Library/class_s_f_e_m_p3_shield.html

XXX possibly useful from skimming
     https://github.com/BareConductive/mpr121/blob/public/MPR121/MPR121.h

   setTouchThreshold(pin,val) / setReleaseThreshold(pin,val) // per-pin thresholds
   getFilteredData(pin) / getBaselineData(pin) // raw data

*******************************************************************************/

// compiler error handling
#include "Compiler_Errors.h"

// touch includes
#include <MPR121.h>
#include <Wire.h>

// mp3 includes
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <SFEMP3Shield.h>

// mp3 shield
SFEMP3Shield MP3player;
// sd card instantiation
SdFat sd;

int lastPlayed = 0; // track which track we last played

// touch behaviour definitions
#define firstPin 0
#define lastPin 11

// define LED_BUILTIN for older versions of Arduino
#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

// tweakables
uint8_t volume = 0; // 0-255 : 10 is audible, higher numbers are quieter??

void setup() {
  Serial.begin(57600);

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Bare Conductive Proximity MP3 player");
  
  setup_sdcard();
  setup_touchpad(40,20); // args = touch, release thresholds
  setup_mp3player(volume); // arg = volume

}

// if this fails, we crash (hardware is broken, get a new board)
void setup_sdcard() {
  if (!sd.begin(SD_SEL, SPI_HALF_SPEED)) {
    Serial.println("Error starting SD card");
    sd.initErrorHalt();
  }
}

// touch_threshold == reading to define a 'touch' (default = 40);
// release_threshold == reading to define a 'release' (default = 20)
// touch_threshold MUST be < release_threshold
void setup_touchpad(unsigned char touch_threshold, unsigned char release_threshold) {
  // correct setup for our bareconductive board
  if (!MPR121.begin(0x5C)) Serial.println("error starting MPR121");
  MPR121.setInterruptPin(4);

  MPR121.setTouchThreshold(touch_threshold);
  MPR121.setReleaseThreshold(release_threshold);
}

// set left and right channel volume to 'volume'
// value is * -1/2dB level
void setup_mp3player(uint8_t volume) {
  byte result = MP3player.begin();
  MP3player.setVolume(volume);

  if (result != 0) {
    Serial.print("Error starting mp3 player, code: ");
    Serial.println(result);
  }
}

// the main attraction

void loop() {
  if (MPR121.touchStatusChanged()) {

    MPR121.updateTouchData();

    // only make an action if we have one or fewer pins touched
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
      Serial.println("\n+++++ end debug +++++");
      /////////////////////////////////////////////////

      // check for pin up on each pin

      for (int i = 0; i < 12; i++) { // Check which electrodes were pressed
        if (MPR121.isNewRelease(i)) {

          Serial.print("release on pin ");
          Serial.println(i);

          digitalWrite(LED_BUILTIN, LOW);

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
          if (MPR121.isNewTouch(i)) {
            Serial.print("touch on pin ");
            Serial.println(i);
            digitalWrite(LED_BUILTIN, HIGH);
          }
        }
      }
    }
  }
}
