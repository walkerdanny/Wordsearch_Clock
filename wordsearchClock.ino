/*
Wordsearch Clock by Danny Walker - March 2017

For schematics and other details, visit http://danny.makesthings.work
and https://github.com/walkerdanny

Released under a CC-BY-NC-SA

As usual, this software is provided with absolutely no warranty.
Don't yell at me if you blow up your USB port etc.

*/

// Include some libraries for the LEDs and the RTC
#include "FastLED.h"
#include <Wire.h>
#include <DS3232RTC.h>
#include <TimeLib.h>

// Define some useful macros
#define NUM_LEDS 100 // Number of LEDs in the grid
#define DATA_PIN 12 // Pin the LED data line is attached to
#define EW 10 // The number of LEDs in the fade effect
#define MAX_BRIGHTNESS 200
// #define MIN_BRIGHTNESS 10 // Max and min brightness used for if the optional brightness pot is added

byte fadeVals [EW]; // Array in which to store the brightness values needed for the fading effect

//const int brightnessPin = A0; // Analog pin the brightness pot is attached to
const int huePin  = A1; // Analog pin the colour adjust pin is attached to
byte colorMapped; // Global variable to store the processed colour value

// Variables used to index the animation array
byte start = 0;
byte last =0;

// Animation variables
byte toAnimate [24]; // 24 is the maximum number of LEDs that are ever going to be animated (for the sentence ITISTWENTYFIVEPASTELEVEN)
byte animCounter=0; // Stores the actually used number of LEDs in th animation, used to account for sentences shorter than 24 characters
boolean animate = true; // Allows the animation to be skipped for one cycle while the clock refreshes

// Time variables used to check if the time has changed
uint8_t currHour = 0;
uint8_t currMinute = 0;
uint8_t prevHour = 1;
uint8_t prevMinute = 1;
uint8_t procHour = 0;
uint8_t procMinute = 0;

// Timers for clock refreshes and animation
long prevMillis = -3000;
const int timeOut = 3000;

long animMillis = -100;
const int animTimeout = 100;

boolean oclock = false; // Used to catch the edge case of it being something oclock

CRGB leds[NUM_LEDS]; // The FastLED array

void setup(){
  delay(3000); // Delay for sanity checks

  Serial.begin(9600);

  // Tell the arduino to sync with the RTC
  setSyncProvider(RTC.get);
  setSyncInterval(5);

  if(timeStatus() != timeSet){
    Serial.println("Unable to sync with RTC");
  } else {
    Serial.println("Synced with RTC");
  }

  // Set up the LEDs
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(MAX_BRIGHTNESS);

  // Set all the LEDs to off
  for(int i = 0;i<NUM_LEDS;i++) leds[i] = (0,0,0);

  setupFadeColours();
  updateClock();
  refreshText();
}

void loop(){

  if(millis()-prevMillis > timeOut){ // If the clock hasn't been read in a while
    prevMillis = millis();
    animate = false; // Probably not needed but it works
    updateClock();
    animate = true; // Probably not needed but it works
    if(currHour != prevHour || currMinute != prevMinute){ // If the clock has changed
      animate = false; // Probably not needed but it works
      refreshText();
      // Dump the time over serial
      Serial.print("Clock changed from ");
      Serial.print(prevHour);
      Serial.print(":");
      Serial.print(prevMinute);
      Serial.print(" to ");
      Serial.print(currHour);
      Serial.print(":");
      Serial.println(currMinute);
      animate = true; // Probably not needed but it works
      last = 0;
    }
  }

  // If the clock has changed then set all LEDs to black and go to the next loop
  if(!animate){
    for(int i=0;i<NUM_LEDS;i++){
      leds[i] = (0,0,0);
    }
  }

  if(millis() - animMillis > animTimeout && animate){ // If it's been long enough since the last animation cycle
    adjustBrightness();
    adjustHue();

    // Set the start position
    if(last>=EW && last < animCounter-1){ // If the last LED is greater than the effect width or less than the animation LED array length
      start = last - EW +1;
    } else if (last<EW && last != animCounter-1){ // If the last LED is less than the effect width from the start, then start at 0
      start = 0;
    }

    if(last == animCounter-1) start++; // If the last LED of the effect is at the last of the animation, bump the start posiion along too so the effect goes "off screen"

    for(int i = 0;i<NUM_LEDS;i++) leds[i] = (0,0,0); // Reset all colours to black before changing them to the fade values

    // // Debugging stuff
    // Serial.print(animCounter);
    // Serial.print(", ");
    // Serial.print(start);
    // Serial.print(", ");
    // Serial.println(last);

    for(int i = start; i<=last; i++){ // Loop from start to last and add in the "fading" led values
      if(last>EW){
        if(toAnimate[i] != 255){
          //Serial.print(toAnimate[i]);
          //Serial.print(",");
          leds[toAnimate[i]] = CHSV(colorMapped,255,fadeVals[i-start]);
        }
      } else {
        if(toAnimate[i] != 255){
          //Serial.print(toAnimate[i]);
          //Serial.print(",");
          leds[toAnimate[i]] = CHSV(colorMapped,255,fadeVals[last-i]);
        }
      }
    }
    //Serial.println();

    if(start == last && start == animCounter-1) last = 0; // Reset when you hit the last

    if(last < animCounter-1) last++; // Move the effect end along one place if it's not at the last LED

    FastLED.show(); // Show the LEDs!

    animMillis = millis();
  }
}



void setupFadeColours(){
    // Set up the fading effect values, sorry this looks totally gross
  // If the effect width is an even number
  if(EW%2 ==0){
    for(int i=0;i<EW/2;i++){
      byte r = (i+1)*(255/(EW/2));
      fadeVals[i] = r;
    }
    for(int i = EW/2;i<EW;i++){
      byte r = ((EW)-i)*(255/(EW/2));
      fadeVals[i] = r;
    }
  // If the effect width is an odd number
  } else {
    for(int i=0;i<=EW/2;i++){
      byte r = (i+1)*(255/(((EW-1)/2)+1));
      fadeVals[i] = r;
    }
    for(int i = EW/2;i<EW;i++){
      byte r = ((EW)-i)*(255/(((EW-1)/2)+1));
      fadeVals[i] = r;
    }
  }

  for(int i = 0; i<EW;i++){
    Serial.println(fadeVals[i]);
    fadeVals[i] = dim8_raw(fadeVals[i]); // Useful function that accounts for how the human eye perceives brightness
  }
}


void adjustBrightness(){
  // Uncomment the code below if you're using the optional brightness pot

  //int rawIn = analogRead(brightnessPin);
  //int brightnessMapped = map(rawIn,0,1023,0,255);
  //Serial.print("Brightness is ");
  //Serial.println(brightnessMapped);
  //FastLED.setBrightness(constrain(brightnessMapped,MIN_BRIGHTNESS,MAX_BRIGHTNESS));

  FastLED.setBrightness(MAX_BRIGHTNESS); // Comment this out if you use the optional pot
}


void adjustHue(){
  int rawIn = analogRead(huePin);
  colorMapped = map(rawIn,0,1023,0,255); // Adjust the range to suit the hue range
}


void updateClock(){
  prevHour = currHour;
  prevMinute = currMinute;

  currHour = hour();
  currMinute = minute();
}


void refreshText(){
  // Save the current minute and hour for processing below
  procHour = currHour;
  procMinute = currMinute;

  // Reset the animation array
  for(int i=0;i<sizeof(toAnimate);i++){
    toAnimate[i] = 255;
  }

  // Put "it" and "is" in their places in the array
  toAnimate[0] = 94;
  toAnimate[1] = 95;
  toAnimate[2] = 3;
  toAnimate[3] = 4;

  animCounter = 4; // There are currently 4 LEDs to light up [ITIS]

  // Adjust for 24 hour clock!
  if (procHour>12) {
    procHour-=12;
  } else if (procHour==0) {
    procHour = 12;
  }

  // Adjust for the case of being [some minutes] to the next twelve hour block
  if (procMinute >= 35 && procHour != 12) {
    procHour++;
  } else if (procMinute>=35 && procHour == 12) {
    procHour = 1;
  }

  // Check minutes and add
  uint8_t m5mins =  currMinute / 5; // divide the number of minutes by 5 so we can use switch... case

  Serial.println("Matching minutes");

  // Match the minutes... the cases here are essentially the solutions to the Wordsearch
  switch(m5mins){
    case 0:
      //It's between 0 and 5 minutes past
      oclock = true;

      break;

    case 1:
      // It's between 5 and 10 minutes past

      // Five
      toAnimate[animCounter] = 90;
      toAnimate[animCounter+1] = 91;
      toAnimate[animCounter+2] = 92;
      toAnimate[animCounter+3] = 93;
      animCounter +=4;

      // PAST
      toAnimate[animCounter] = 78;
      toAnimate[animCounter+1] = 68;
      toAnimate[animCounter+2] = 58;
      toAnimate[animCounter+3] = 48;
      animCounter+=4;

      break;

    case 2:
      // It's between 10 and 15 minutes past
      // TEN
      toAnimate[animCounter] = 35;
      toAnimate[animCounter+1] = 24;
      toAnimate[animCounter+2] = 13;
      animCounter+=3;

      // PAST
      toAnimate[animCounter] = 78;
      toAnimate[animCounter+1] = 68;
      toAnimate[animCounter+2] = 58;
      toAnimate[animCounter+3] = 48;
      animCounter+=4;

      break;

    case 3:
      // It's between 15 and 20 minutes past
      //QUARTER
      toAnimate[animCounter] = 0;
      toAnimate[animCounter+1] = 10;
      toAnimate[animCounter+2] = 20;
      toAnimate[animCounter+3] = 30;
      toAnimate[animCounter+4] = 40;
      toAnimate[animCounter+5] = 50;
      toAnimate[animCounter+6] = 60;

      animCounter+=7;

      // PAST
      toAnimate[animCounter] = 78;
      toAnimate[animCounter+1] = 68;
      toAnimate[animCounter+2] = 58;
      toAnimate[animCounter+3] = 48;
      animCounter+=4;

      break;

    case 4:
      // It's between twenty and twenty five past
      // TWENTY

      toAnimate[animCounter] = 56;
      toAnimate[animCounter+1] = 46;
      toAnimate[animCounter+2] = 36;
      toAnimate[animCounter+3] = 26;
      toAnimate[animCounter+4] = 16;
      toAnimate[animCounter+5] = 6;

      animCounter+=6;


      // PAST
      toAnimate[animCounter] = 78;
      toAnimate[animCounter+1] = 68;
      toAnimate[animCounter+2] = 58;
      toAnimate[animCounter+3] = 48;
      animCounter+=4;

      break;

    case 5:
      // TWENTY
      toAnimate[animCounter] = 56;
      toAnimate[animCounter+1] = 46;
      toAnimate[animCounter+2] = 36;
      toAnimate[animCounter+3] = 26;
      toAnimate[animCounter+4] = 16;
      toAnimate[animCounter+5] = 6;

      animCounter+=6;


      // FIVE
      toAnimate[animCounter] = 90;
      toAnimate[animCounter+1] = 91;
      toAnimate[animCounter+2] = 92;
      toAnimate[animCounter+3] = 93;
      animCounter +=4;

      // PAST
      toAnimate[animCounter] = 78;
      toAnimate[animCounter+1] = 68;
      toAnimate[animCounter+2] = 58;
      toAnimate[animCounter+3] = 48;
      animCounter+=4;

      break;

    case 6:
      // HALF
      toAnimate[animCounter] = 31;
      toAnimate[animCounter+1] = 21;
      toAnimate[animCounter+2] = 11;
      toAnimate[animCounter+3] = 1;
      animCounter+=4;

      //PAST
      toAnimate[animCounter] = 78;
      toAnimate[animCounter+1] = 68;
      toAnimate[animCounter+2] = 58;
      toAnimate[animCounter+3] = 48;
      animCounter+=4;

      break;

    case 7:
      // TWENTY
      toAnimate[animCounter] = 56;
      toAnimate[animCounter+1] = 46;
      toAnimate[animCounter+2] = 36;
      toAnimate[animCounter+3] = 26;
      toAnimate[animCounter+4] = 16;
      toAnimate[animCounter+5] = 6;

      animCounter+=6;


      // FIVE
      toAnimate[animCounter] = 90;
      toAnimate[animCounter+1] = 91;
      toAnimate[animCounter+2] = 92;
      toAnimate[animCounter+3] = 93;
      animCounter +=4;

      // TO
      toAnimate[animCounter] = 61;
      toAnimate[animCounter+1] = 62;
      animCounter +=2;

      break;

    case 8:
      // TWENTY
      toAnimate[animCounter] = 56;
      toAnimate[animCounter+1] = 46;
      toAnimate[animCounter+2] = 36;
      toAnimate[animCounter+3] = 26;
      toAnimate[animCounter+4] = 16;
      toAnimate[animCounter+5] = 6;

      animCounter+=6;

      // TO
      toAnimate[animCounter] = 61;
      toAnimate[animCounter+1] = 62;
      animCounter +=2;

      break;

    case 9:
      // QUARTER
      toAnimate[animCounter] = 0;
      toAnimate[animCounter+1] = 10;
      toAnimate[animCounter+2] = 20;
      toAnimate[animCounter+3] = 30;
      toAnimate[animCounter+4] = 40;
      toAnimate[animCounter+5] = 50;
      toAnimate[animCounter+6] = 60;

      animCounter+=7;

      // TO
      toAnimate[animCounter] = 61;
      toAnimate[animCounter+1] = 62;
      animCounter +=2;

      break;

    case 10:
      // TEN
      toAnimate[animCounter] = 35;
      toAnimate[animCounter+1] = 24;
      toAnimate[animCounter+2] = 13;
      animCounter+=3;

      // TO
      toAnimate[animCounter] = 61;
      toAnimate[animCounter+1] = 62;
      animCounter +=2;

      break;

    case 11:
      // FIVE
      toAnimate[animCounter] = 90;
      toAnimate[animCounter+1] = 91;
      toAnimate[animCounter+2] = 92;
      toAnimate[animCounter+3] = 93;
      animCounter +=4;

      // TO
      toAnimate[animCounter] = 61;
      toAnimate[animCounter+1] = 62;
      animCounter +=2;

      break;

    default:
    // Don't do anything?
    break;
  }

  // Now do the same for hours
  Serial.println("Matching hours");

  switch(procHour) {
    case 1:
      //println("ONE");
      toAnimate[animCounter] = 23;
      toAnimate[animCounter+1] = 34;
      toAnimate[animCounter+2] = 45;

      animCounter +=3;

      break;
    case 2:
      //println("TWO");
      toAnimate[animCounter] = 57;
      toAnimate[animCounter+1] = 47;
      toAnimate[animCounter+2] = 37;

      animCounter +=3;
      break;
    case 3:
      //println("THREE");
      toAnimate[animCounter] = 87;
      toAnimate[animCounter+1] = 76;
      toAnimate[animCounter+2] = 65;
      toAnimate[animCounter+3] = 54;
      toAnimate[animCounter+4] = 43;

      animCounter +=5;
      break;
    case 4:
      //println("FOUR");
      toAnimate[animCounter] = 9;
      toAnimate[animCounter+1] = 19;
      toAnimate[animCounter+2] = 29;
      toAnimate[animCounter+3] = 39;

      animCounter += 4;
      break;
    case 5:
      //println("FIVE");
      toAnimate[animCounter] = 90;
      toAnimate[animCounter+1] = 91;
      toAnimate[animCounter+2] = 92;
      toAnimate[animCounter+3] = 93;

      animCounter += 4;
      break;
    case 6:
      //println("SIX");
      toAnimate[animCounter] = 79;
      toAnimate[animCounter+1] = 89;
      toAnimate[animCounter+2] = 99;

      animCounter +=3;
      break;
    case 7:
      //println("SEVEN");
      toAnimate[animCounter] = 53;
      toAnimate[animCounter+1] = 64;
      toAnimate[animCounter+2] = 75;
      toAnimate[animCounter+3] = 86;
      toAnimate[animCounter+4] = 97;

      animCounter +=5;
      break;
    case 8:
      //println("EIGHT");
      toAnimate[animCounter] = 70;
      toAnimate[animCounter+1] = 71;
      toAnimate[animCounter+2] = 72;
      toAnimate[animCounter+3] = 73;
      toAnimate[animCounter+4] = 74;

      animCounter +=5;
      break;
    case 9:
      //println("NINE");
      toAnimate[animCounter] = 8;
      toAnimate[animCounter+1] = 18;
      toAnimate[animCounter+2] = 28;
      toAnimate[animCounter+3] = 38;

      animCounter += 4;
      break;
    case 10:
      //println("TEN");
      toAnimate[animCounter] = 35;
      toAnimate[animCounter+1] = 24;
      toAnimate[animCounter+2] = 13;

      animCounter +=3;

      break;
    case 11:
      //println("ELEVEN");
      toAnimate[animCounter] = 80;
      toAnimate[animCounter+1] = 81;
      toAnimate[animCounter+2] = 82;
      toAnimate[animCounter+3] = 83;
      toAnimate[animCounter+4] = 84;
      toAnimate[animCounter+5] = 85;


      animCounter +=6;

      break;
    case 12:
      //println("TWELVE");
      toAnimate[animCounter] = 2;
      toAnimate[animCounter+1] = 12;
      toAnimate[animCounter+2] = 22;
      toAnimate[animCounter+3] = 32;
      toAnimate[animCounter+4] = 42;
      toAnimate[animCounter+5] = 52;


      animCounter +=6;

      break;
    default:
    // Do nothing?
    break;
  }

  // Catch if it's something oclock!
  if (oclock) {
    toAnimate[animCounter] = 33;
    toAnimate[animCounter+1] = 44;
    toAnimate[animCounter+2] = 55;
    toAnimate[animCounter+3] = 66;
    toAnimate[animCounter+4] = 77;
    toAnimate[animCounter+5] = 88;


    animCounter +=6;

    oclock = false; // don't do this next time round!
  }
}
