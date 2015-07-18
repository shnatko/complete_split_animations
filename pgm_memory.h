/*
	Header file for defining text alphabet
	
	Contains LED definitions for each letter..pain in the ass to code..
        
        10.08.14: also added "dark room" calibration values so I can skip doing a calibration if in a dark room and just load the values from flash, saves time
        10.09.14: added sprite frames stored in pgm space.  consider renaming header file to something that makes more sense

*/

#ifndef pgm_memory_h
#define pgm_memory_h

#include <avr/pgmspace.h>     // for use of program memory to store static data
#include <Arduino.h>

// grrr the compiler doesn't know what a byte is...
typedef unsigned char byte;

extern const word text_lookup[71][20] PROGMEM;

// also store default calibration values for IR array in PRGMEM to use as a default when you don't want to run cal.
extern const byte dark_cal_on[128] PROGMEM;
extern const byte dark_cal_off[128] PROGMEM;
          
extern const word link_down_1[512] PROGMEM;
extern const word link_down_2[512] PROGMEM;
extern const word link_right_1[512] PROGMEM;
extern const word link_right_2[512] PROGMEM;
extern const word link_left_1[512] PROGMEM;
extern const word link_left_2[512] PROGMEM;
extern const word link_up_1[512] PROGMEM;
extern const word link_up_2[512] PROGMEM;
extern const word flag1[512] PROGMEM;
extern const word flag2[512] PROGMEM;
extern const word flag3[512] PROGMEM;


// UGLY but big fucking lookup table to find index for LED array to draw a given letter/punctuation
extern byte letter_lookup(char );

 //text_lookup[letter_a][] = {97,98,99,128,132,160,164,192,196,225,226,227};

#endif
