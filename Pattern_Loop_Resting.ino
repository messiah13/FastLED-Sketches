#include "FastLED.h"

FASTLED_USING_NAMESPACE

// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define NUM_LEDS            150
#define FRAMES_PER_SECOND   240
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#define maxsteps 16              // Case statement wouldn't allow a variable.

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType    currentBlending;    // NOBLEND or LINEARBLEND

int        myhue =   0;
int     thisdelay = 600;          // Standard delay value.
int center = 0;                   // Center of the current ripple.
int step = -1;                    // -1 is the initializing step.uint8_t max_bright = 128;

uint8_t max_bright = 255;         // Overall brightness definition. It can be changed on the fly.
uint8_t thisbeat =  16;           // Beats per minute for first part of dot. 23
uint8_t thatbeat =  20;           // Combined the above with this one. 28
uint8_t thisfade =  32;           // How quickly does it fade? Lower = slower fade rate.
uint8_t  thisbri = 255;           // Brightness of a sequence.
uint8_t myfade = 255;             // Starting brightness.
uint8_t fadeval =22;              // 8 bit, 1 = slow, 255 = fast
uint8_t colour;                   // Ripple colour is randomized.

// have 3 independent CRGBs - 2 for the sources and one for the output
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];
CRGB leds3[NUM_LEDS];

void setup()
{
  
  FastLED.addLeds<NEOPIXEL, 4>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, 5>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, 7>(leds, NUM_LEDS);
  
  delay(1000);                        // Soft startup to ease the flow of electrons.
  Serial.begin(9600);
  
  currentBlending = LINEARBLEND;
  FastLED.setBrightness(max_bright);
  set_max_power_in_volts_and_milliamps(5, 500);     // FastLED Power management set at 5V, 500mA.
 

} // Setup

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { ease, sinelon, ripple  };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = CRGB::DodgerBlue; // "base color" used by many of the patterns
  
void loop()
{
  gPatterns[gCurrentPatternNumber](); // Call the current pattern function once, updating the 'leds' array
  FastLED.show();  // send the 'leds' array out to the actual LED strip
  FastLED.delay(1000/FRAMES_PER_SECOND); // insert a delay to keep the framerate modest
  // do some periodic updates
  EVERY_N_MILLISECONDS( 200 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 32 ) { nextPattern(); } // change patterns periodically
} // loop()

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
} // nextPattern()

void ease() {

  static uint8_t easeOutVal = 0;
  static uint8_t easeInVal  = 0;
  static uint8_t lerpVal    = 0;

  easeOutVal = ease8InOutQuad(easeInVal); // Start with easeInVal at 0 and then go to 255 for full easing.
  easeInVal++;
  lerpVal = lerp8by8(0, NUM_LEDS, easeOutVal);  // Map it to the number of LED's you have.
  leds[lerpVal] = gHue;
  fadeToBlackBy(leds, NUM_LEDS, fadeval);       // 8 bit, 1 = slow fade, 255 = fast fade
  
} // ease()

void sinelon() {    // a colored dot sweeping back and forth, with fading trails
  
  EVERY_N_MILLISECONDS(100) {
    uint8_t maxChanges = 24; 
   nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);// AWESOME palette blending capability.
  }

  EVERY_N_SECONDS(5) {              // Change the target palette to a random one every 5 seconds.
    static uint8_t baseC = CRGB::DodgerBlue;// You can use this as a baseline colour if you want similar hues in the next line.
    targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
  }

  EVERY_N_MILLISECONDS(thisdelay) { // FastLED based non-blocking delay to update/display the sequence.
    sinelon();                      // Call our sequence.
  }
  
  fadeToBlackBy( leds, NUM_LEDS, thisfade);
  int pos1 = beatsin16(thisbeat,0,NUM_LEDS);
  int pos2 = beatsin16(thatbeat,0,NUM_LEDS);

  leds[(pos1+pos2)/2] +=  ColorFromPalette(currentPalette, myhue++, thisbri, currentBlending);

} // sinelon()

void ripple() {
 
  EVERY_N_MILLISECONDS(500) {
    uint8_t maxChanges = 24; 
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges); //AWESOME palette blending capability.
  }

  EVERY_N_SECONDS(3) {
   targetPalette = CRGBPalette16(CHSV(random8(), 255, 32), CHSV(random8(), random8(64)+192, 255), CHSV(random8(), 255, 32), CHSV(random8(), 255, 255)); 
  }

  EVERY_N_MILLISECONDS(thisdelay) {  // FastLED based non-blocking delay to update/display the sequence.
    ripple();
  }
    FastLED.show();
    fadeToBlackBy(leds, NUM_LEDS, fadeval);                    // 8 bit, 1 = slow, 255 = fast
  
  switch (step) {

    case -1:                                                 // Initialize ripple variables.
      center = random(NUM_LEDS);
      colour = random8();
      step = 0;
      break;

    case 0:
      leds[center] = ColorFromPalette(currentPalette, colour, myfade, currentBlending);
      
      step ++;
      break;

    case maxsteps:                                           // At the end of the ripples.
      step = -1;
      break;

    default:                                                 // Middle of the ripples.
      leds[(center + step + NUM_LEDS) % NUM_LEDS] += ColorFromPalette(currentPalette, colour, myfade/step*2, currentBlending);       // Simple wrap from Marc Miller
      leds[(center - step + NUM_LEDS) % NUM_LEDS] += ColorFromPalette(currentPalette, colour, myfade/step*2, currentBlending);
      step ++;                                               // Next step.
      break;  
  } // switch step
} // ripple()

//void aanimation() {

//  animationA();                                         // render the first animation into leds2   
//  animationB();                                         // render the second animation into leds3

//  uint8_t ratio = beatsin8(2);                          // Alternate between 0 and 255 every minute
  
//  for (int i = 0; i < NUM_LEDS; i++) {                  // mix the 2 arrays together
//    leds[i] = blend( leds2[i], leds3[i], ratio );
//  }
//
//} // loop()



void animationA() {                                   // running red stripe.

  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t red = (millis() / 10) + (i * 12);         // speed, length
    if (red > 128) red = 0;
    leds2[i] = CRGB(red, 0, 0);
  }
} // animationA()



void animationB() {                                   // running blue stripe in opposite direction.
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t blue = (millis() / 5) - (i * 12);         // speed, length
    if (blue > 128) blue = 0;
    leds3[i] = CRGB(0, 0, blue);
  }
} // animationB()
