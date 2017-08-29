#include "FastLED.h"


/* //////////////////////////////////////////////////
 * //making sense of the Parallax PIR sensor's output
 * //////////////////////////////////////////////////
 *
 * Switches a LED according to the state of the sensors output pin.
 * Determines the beginning and end of continuous motion sequences.
 *
 * @author: Kristian Gohlke / krigoo (_) gmail (_) com / http://krx.at
 * @date:   3. September 2006 
 *
 * kr1 (cleft) 2006 
 * released under a creative commons "Attribution-NonCommercial-ShareAlike 2.0" license
 * http://creativecommons.org/licenses/by-nc-sa/2.0/de/
 *
 * The Parallax PIR Sensor is an easy to use digital infrared motion sensor module. 
 * (http://www.parallax.com/detail.asp?product_id=555-28027)
 *
 * The sensor's output pin goes to HIGH if motion is present.
 * However, even if motion is present it goes to LOW from time to time, 
 * which might give the impression no motion is present. 
 * This program deals with this issue by ignoring LOW-phases shorter than a given time, 
 * assuming continuous motion is present during these phases.
 *  
 *
 * easing
 *
 * By: Andrew Tuline
 *
 * Date: August, 2015
 *
 * This boring display demonstrates the easing capability of FastLED. 
 * The Blue LED starts out slow, speeds up and then slows down when it gets to the end.
 * It uses uint8_t variables and may not work correctly on longer strands . . 
 * as noted by GitHub user fahrvergnuunen.
 * easeOutVal = ease8InOutQuad(easeInVal);      // Start with easeInVal at 0 and then go to 255 for the full easing.
 * ledNum = lerp8by8(0, NUM_LEDS, easeOutVal);  // Map it to the number of LED's you have.
 *
 *
 * sinelon
 *
 * By: Mark Kriegsman
 *
 * Modified by: Andrew Tuline
 *
 * Date: February 2015
 *
 * This uses the built in beat in FastLED to move a dot back and forth. 
 * In this case, it uses two beats added together for more randomness.
 * 
 */
 

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif


long unsigned int lowIn;  //the time when the sensor outputs a low impulse       
long unsigned int pause = 1000;  //the amount of milliseconds the sensor has to be low before we assume all motion has stopped
boolean lockLow = true;
boolean takeLowTime; 
int pirPin = 3;                   //the digital pin connected to the PIR sensor's output
int ledPin = 13;
int calibrationTime = 30;       // (10-60 secs according to the datasheet)

#define NUM_LEDS 150
#define FRAMES_PER_SECOND 2400

// Global variables can be changed on the fly.
struct CRGB leds[NUM_LEDS];     // Initialize our LED array.
int thisdelay = 600;
int        gHue = 0;            // rotating "base color" used by many of the pattern
int     myhue =   0;

uint8_t max_bright = 255;       // Overall brightness definition.
uint8_t  thisbri = 255;         // Brightness of a sequence.
uint8_t thisbeat =  16;         // Beats per minute for first part of dot. 23
uint8_t thatbeat =  20;         // Combined the above with this one. 28
uint8_t thisfade =  32;         // How quickly does it fade? Lower = slower fade rate.

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType  currentBlending;    // NOBLEND or LINEARBLEND

void setup() {
 
  FastLED.addLeds<NEOPIXEL, 4>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, 5>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, 7>(leds, NUM_LEDS);
 
  FastLED.setBrightness(max_bright);
  set_max_power_in_volts_and_milliamps(5, 500);       // FastLED Power management set at 5V, 500mA. 

    Serial.begin(9600);
    pinMode(pirPin, INPUT);
    pinMode(ledPin, OUTPUT);
    digitalWrite(pirPin, LOW);

  //give the sensor some time to calibrate
     Serial.print("calibrating sensor ");
     for(int i = 0; i < calibrationTime; i++){
        Serial.print(".");
       delay(1000);
      }
      Serial.println(" done");  
       Serial.println("SENSOR ACTIVE");
       delay(50);
}

void loop(){
  
  if(digitalRead(pirPin) == HIGH)
    digitalWrite(ledPin, HIGH);   //the led visualizes the sensors output pin state

  FastLED.delay(1000/FRAMES_PER_SECOND);  // insert a delay to keep the framerate modest 
  EVERY_N_MILLISECONDS(thisdelay) {       // FastLED based non-blocking delay to update/display the sequence.
    ease();
  }
  
  else
  
  if(digitalRead(pirPin) == LOW)       
       digitalWrite(ledPin, LOW);  //the led visualizes the sensors output pin state
 { Serial.println(" motion detected! ");
  EVERY_N_MILLISECONDS(thisdelay);
  
  EVERY_N_MILLISECONDS(100) {
    uint8_t maxChanges = 24; 
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);// AWESOME palette blending capability.
  }

  EVERY_N_SECONDS(5) {                      // Change the target palette to a random one every 5 seconds.
    static uint8_t baseC = CRGB::DodgerBlue;// You can use this as a baseline colour if you want similar hues in the next line.
    targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
  }

  EVERY_N_MILLISECONDS(thisdelay) {         // FastLED based non-blocking delay to update/display the sequence.
    sinelon();                              // Call our sequence.
  }

  FastLED.show();
 }
} // loop()

void ease() {

  static uint8_t easeOutVal = 0;
  static uint8_t easeInVal  = 0;
  static uint8_t lerpVal    = 0;

  easeOutVal = ease8InOutQuad(easeInVal);        // Start with easeInVal at 0 and then go to 255 for the full easing.
  easeInVal++;

  lerpVal = lerp8by8(0, NUM_LEDS, easeOutVal);   // Map it to the number of LED's you have.

  leds[lerpVal] = CRGB::DodgerBlue;
  fadeToBlackBy(leds, NUM_LEDS, 16);             // 8 bit, 1 = slow fade, 255 = fast fade
  
} // ease()

void sinelon() {                                 // a colored dot sweeping back and forth, with fading trails
  
  fadeToBlackBy( leds, NUM_LEDS, thisfade);
  int pos1 = beatsin16(thisbeat,0,NUM_LEDS);
  int pos2 = beatsin16(thatbeat,0,NUM_LEDS);

  leds[(pos1+pos2)/2] += ColorFromPalette(currentPalette, myhue++, thisbri, currentBlending);

} // sinelon()




