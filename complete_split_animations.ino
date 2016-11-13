/*
 512 LED 16x32 dimensional array, each pin independently brightness modulated

 code will "draw" one row of LEDs each interrupt cycle, so we'll need 16 interrupts to get though whole array
 ambient light sensor used to detect change in light level, triggering lights on/off  off is done via a fade out animation to add some smoothness
 added NES controller polling on timer2 interrupt
 on interrupt, latch button state into shift reg on controller, scan out and display button state on LEDs
 NES controller used now to select animations ( via select button ) and also to pause any animation ( via the start button )
 adding SPI interface to load shift register rather than call to shift out function

 ** ignore some of above, comment carry over from previous code

 12/23:  broke out red and blue led brightness as separate shifted bytes
 added streak to rotate animation
 12/26:  first attempt at 4x4 array!
 12/29:  finally got 4x4 rgb array working! woo!
 todo:  reverse shift reg order to cut down on wires  DONE
 1/09  got NES control working, added some color tools
 1/16: added serial input to set individual led w/ serial monitor
 1/22: testing mbi5168 led current sink
 1/24: code update to add defines for IO functions, started changes to brightness memory to get away from Darrah BAM method to update matrix
 -- i don't think i need 16 levels of brightness ( 16x16x16 = 4K color shared ), will try reduction down to 512, this should speed things up and allow me to increase
 	  number of rows in array from 4 to the 16 I eventually want to use

 2/10: added "painting" mode for light sense
 2/16: added shift reg for row control.  so shift row outputs via SPI now
 2/20: IMPORTANT.  reversed IR sensors.  pull-down resistors on analog input now instead of pull up, code changes to match ( calibrate function, etc.. )
 2/22: renamed to array_4x4_<datestamp>,  changed NES_move to paint w/ button press ability
 3/03: YAY NEW CAR!  added separate light sense calibration threshold for single color blue fade
 3/20: custom PCB!!!!  actually works!!! swapped row ordering to match pcb silkscreen.  still need variable scrub for memory size
 3.30: HOLY SHIT! FIRST 8X8 TEST!
 3.31: w00T!  got 16x16 2 board array working.  nothing fancy yet and perf. is kinda crappy.  will need better controller at some point.  cleaned up memory as well.  use flash to store lookup tables.
 4.1: got nes_move working w/ paint function!!
 4.3: got 3rd set of columns working ( board jumpers work too! ), added random_blink animation and fixed bugs in paint.
 4.4: got full 16x32 array running!!!  as expected performance does suffer for some modes.   work on improving code, saving ram, and adding new animations.  got bounce working better now, still need to add speed
 4.5: added next_animation function again to cycle though animations on button press ( need to get pushbutton to use this with ), started adding welcome spash screen, need to put into program memory
 4.6: added serial dump of nes_paint screen if debug is enabled to make saving drawings possible, completed splash screen greeting
 4.7: added edge flash on ball bounce strike
 4.9: fix to nes_paint for performance improvement, started pong, got basics of it down!
 4.13: added argument for splash screen to all for alternate text displayed ( for example for start of each different animation )
 4.15: added trails to bounce for balls < 4, not much else.
 4.22: added serial control w/ mouse input
 4.23: added x/y cursors to mouse input, modified serial communication string a bit
 4.26: added wave animation ( initial )
 5.23: TRANSITION TO DUE, GOT IT WORKING!!!
 5.26: got NES controller working @3.3V.  need to debug pong, something still wrong when game ends
 6.4: moved to complete, got rid of serial control, will improve/add animations here, added fade animation
 6.5: added color cycle to wave animation, added line sweeper animation ( need to debug more the drawing/clearing )
 6.16: added memory location for 8-bit analog read of IR sense data ( muxed ).  initially to test performance of 16 cycle read of 8 analog inputs ( as opposed to 11 cycles of 12 inputs )
 7.4: HAPPY 4th of JULY!  added fireworks, still needs some tweaking
 8.10: BOO! back from vacation.   small add of IR array enable output pin in preperation for getting IR sense functions working
 8.11: changed line sweeper to run 8 zones rather than full screen
 8.12: FIRST IR NODE WORKING!!!! just simple blink on/off right now and just 1 sense node running, but hey, it's a start
 8.14: WOOOOO! 1/2 IR nodes functional!!  it's taken FOREVER to get to this point but it's working!  issue with sense group1 sometimes picking up sense group 0's data though, still need to resolve but other than that
  WOOO HOOO!!!
 8.15: fixed sense group issue, added calibration and masking ability, moved IR sense to it's own interrupt with it's own enable/disable functions.  looking pretty
 8.18: all 128 ir nodes now working!!!!! holy crap!  need to add fade/delay times to turn off.  hopefully there is enough ram
 8.18: reduced mux selects back down to 4 pins w/ new wire, added light sense fade down, also added cool color cycling for light sense.
 8.26: added rain animation, including new fade_up function and gravitational freefall calculation for falling drops
 10.01: started added text scroll animation.  lots more to do
 10.02: lot more done!  text scrolling working well now.  just need to get character library completed...
 10.03: full alphanumeric character library coded, added 2nd type to text scroll on IR input ( may use for clock function or something along those lines )
 10.08: some tweaks to calibration to make it quicker
 10.09: added NES controls over wave function to tweak color, speed, height.  pretty slick now if I do say so :P
 10.10: added sprite_animation function, new link frames, changed pgm memory header file name
 10.11: added more frames to sprite animation, added ball avoid mode to light_sense
 10.13: added contra code easter egg to unlock tetris mode!
 10.16: added second NES controller, will need to add it to paint for starters to use
 10.17: fixed fireworks animation to fade differently and to support multiple bursts
 10.21: tweak to wave animation to add wave fill option.  awesome!
 10.23: changed text scrolling to handle string type to allow for integers ( millis, counters, etc.. ), added persistent mode to shape bouncing.
 10.24: added rfill function, random fill/unfill of array, also added start of stopwatch function, needs controller input, maybe add countdown timer too
 10.25: fixed up stopwatch function so it works properly now
 11.07: added chaser animation
 11.12: some tweaks to chaser
 11.15: reordered some animations, updated line sweeper to have no repeats on new color type
 11.20: added fade down mode for line sweep, so 3 types now, added second wandering led for chaser
 12.06: tried to modify calibration sequence to use 1 node a a time, it does not work very well. likely not enough ambient light around each sensor to do a good calibration, sticking to old method
 12.17: added draw row and col functions.  will make use of them in future
 01.05: happy new year 2015! adjusted NES pins to accommodate WIFI shield, fixed port swap, still need to figure out how to hook up hw SPI port for both wifi and panel control
 01.11: added wifi shield!  just check that it can scan networks so far..
 01.15: split blank and latch pins to 2 outputs each to hopefully allow better electrical distribution out to control boards
 01.21: starting to try to transition SPI for panel over to USART, it's been a nightmare but 1 control board is working.. more to come
 01.22: holy what the hell.  got USART SPI MASTER mode running for all 4 panels, CLK connection is VERY flaky but does work properly!  now to try to get the wifi running again, sweet!  that was easy!
        can now set color of table from smartphone app!
 01.24: updates to wifi client function to allow for animation selection, works from smartphone now.  awesome.  still buggy but does work
 02.09: updates to wave equation for 2d plotting, still needs some work
 02.20: working to move all the animations out of main file into its own source code
 02.22: moved all animation code over to separate source flie
 03.04: update to paint to add color chooser screen rather than rgb select, better option for picking color to paint with
 03.07: first pass snake game
 03.25: for some reason now the ir_on function to turn the ir sense interrupts back on and enable the IR array is now corrupting a bit of the sense_cal_on array.  this makes no sense to me currently but
        there is a workaround in place to simply save the state of the array before the function call and then restore it back.   band-aids work
 04.12: a few updates for phone app use
 05.01: added game of life animation
 05.17: start of maze generator / solver animation
 07.02: added flag sprite animation for july 4th
 07.15: started BT testing instead of wifi, removed wifi code for this branch
 07.19: added option change control for BT commands from Android, will expand for other animations
 08.08:	added pulldown on nes1 port for presence detect, if no controller is plugged, read 0
		also added bt ommunication back to phone to tell it current animation, can use this for other stuff as well eventually
 01.09: HNY added voter for upcoming shower
 07.04: updates to chaser
 
animation_number        animation
0                       nes_paint
1                       blink_rand_interval
2                       bounce
3                       rain animation / pong  NEED TO MOVE PONG TO IT'S OWN VALUE
4                       static color test ( clear_all w/ color argument )
4                       random fill, unfill of display, random or fixed color
5                       random led on fade off
6                       wave
7						line sweeper
8                       chaser
9                       ir sense
10                      text scroll
11                      "sprite" animate
12                      stopwatch
13						serial control
14                      it's a secret to everyone.  not really, it's just tetris
15                      snake
16						life
17						maze
18						voter
19						pong / breakout


TODO:  general code clean up, updated animations
 
 */


#include <SPI.h>              // SPI Library used to clock data out to the shift registers
#include <avr/pgmspace.h>     // for use of program memory to store static data
#include "animations.h"       // header for animation functions/source code

// control overall flow of pgm
byte cycle = 0;          	   // set to 1 to cycle though animations at 15s in interval, or 0 to stay on current animation until cycle button is pressed on table
int cycle_time = 7500;	       // time in ms to wait between switching to next animation in sequence
const byte debug = 0;          // set to 1 to get serial data out for debug
const byte bluetooth = 1;	   // set to 1 to enable BT shield
byte cal = 0;            	   // set to 0 to turn off calibration for other debug so we don't run it all the damn time, 1 = force cal always, 2 = check for dark room first
const byte shade_limit = 6;    // "grayscale" shades for each color
const byte code_array[14] = { 0, 0, 1, 1, 2, 3, 2, 3, 4, 5, 4, 5, 6, 7};
byte animation_sequence[18] = {0, 8, 99, 3, 5, 6, 7, 8, 9, 19, 15, 16, 17, 99, 14, 99, 0, 99};	// sequence of animations that pressing button or cycling will cycle through
byte controller_plugged = 0;   // global for whether or not controller is plugged into port

// static defines

//             blank pins  latch pins
// board pins  36,37       30,32
// PORTC pins  4,5
// PORTD pins              9,10

#define blank_high REG_PIOC_ODSR = REG_PIOC_ODSR | 0x00000030    // shift reg blank pin high ( DISABLE outputs )
#define blank_low  REG_PIOC_ODSR = REG_PIOC_ODSR & 0xFFFFFFCF    // shift reg blank pin low ( ENABLE outputs )
#define latch_high REG_PIOD_ODSR = REG_PIOD_ODSR | 0x00000600   // latch pin high to shift regs
#define latch_low  REG_PIOD_ODSR = REG_PIOD_ODSR &  0xFFFFF9FF   // latch pin low to shift regs

#define ledDataPin  75            // MOSI, board pin 75
#define shiftclkPin 76            // SPI clock, board pin 76

#define NES_clk1     26  //PORTD1                  
#define NES_latch1   25  //PORTD0                 
#define NES_data1    24  //PORTA15                

#define NES_clk0     29  //PORTD6       RED 
#define NES_latch0   28  //PORTD3       ORANGE 
#define NES_data0    27  //PORTD2       YELLOW 

// DUE register manipulation to set GPIO outputs,  should be faster than using arduino macros
#define NES_clk0_high    REG_PIOD_ODSR = REG_PIOD_ODSR | 0x00000040    
#define NES_clk0_low     REG_PIOD_ODSR = REG_PIOD_ODSR & 0xFFFFFFFBF
#define NES_latch0_high  REG_PIOD_ODSR = REG_PIOD_ODSR | 0x00000008
#define NES_latch0_low   REG_PIOD_ODSR = REG_PIOD_ODSR & 0xFFFFFFF7

#define NES_clk1_high    REG_PIOD_ODSR = REG_PIOD_ODSR | 0x00000002
#define NES_clk1_low     REG_PIOD_ODSR = REG_PIOD_ODSR & 0xFFFFFFFFD
#define NES_latch1_high  REG_PIOD_ODSR = REG_PIOD_ODSR | 0x00000001
#define NES_latch1_low   REG_PIOD_ODSR = REG_PIOD_ODSR & 0xFFFFFFFE


#define cycle_button 53         // the pin the input button1 is attached to, will cycle animations ( if interval has expired )
#define ir_array_enable 47      // pin used to turn on the IR sense array when in use
#define sense_select0 40        // output pins for IR sense mux selects
#define sense_select1 41
#define sense_select2 42
#define sense_select3 43

#define vote_boy_button 2
#define vote_girl_button 3
#define vote_results_button 4


// general variables used for LED control
// row 76543210
const byte row_cntl0[] = {
  B11111110,    // bytes to shift out to row control shift reg for rows 0:7
  B11111101,    // w/ PFET row drives now, 0 is active row, 1 is disabled row
  B11111011,
  B11110111,
  B11101111,
  B11011111,
  B10111111,
  B01111111,
  B11111111,    //8
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111
};

const byte row_cntl1[] = {
  B11111111,    // bytes to shift out to row control shift reg for rows 8:15
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111110,  //8
  B11111101,
  B11111011,
  B11110111,
  B11101111,
  B11011111,
  B10111111,
  B01111111
};

const byte rows = 16;             // row/col counts;
const byte cols = 32;
const int16_t ledCount = 512;     // how many LEDs we dealin' wif
int16_t shade_compare = 0;        // compare value for brightness testing

// new byte array to store single brightness value for a each pixel, the value will be used to access a lookup table to determine appropriate r.g.b values to send to shift regs
int16_t display_mem[512];			// main structure to hold LED array color state
byte led_state[512];
static int16_t *display_ptr0;       // pointer to display array for col0:7
static int16_t *display_ptr1;       // pointer to display array for col8:15
static int16_t *display_ptr2;       // pointer to display array for col16:23
static int16_t *display_ptr3;       // pointer to display array for col24:31

// lookup tables to store r,g,b data for pixel color/brightness ( can also just calc value on the fly if you want to save 1.5KB of RAM
// each led as 8 possible brightnesses ( including off ) so that gives us 8x8x8 = 512 shades
// NOTE:  these are stored in program memory to save SRAM space

PROGMEM prog_uchar lookup_red[512] =  {    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                           3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                           4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                                           5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                                           6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                           7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
                                      };

PROGMEM prog_uchar lookup_green[512] =  {  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,
                                           0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,
                                           0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,
                                           0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,
                                           0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,
                                           0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,
                                           0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,
                                           0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7
                                        };

PROGMEM prog_uchar lookup_blue[512] =  {   0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
                                           0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
                                           0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
                                           0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
                                           0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
                                           0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
                                           0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
                                           0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7
                                       };



// used to reading analog levels to adjust update interval / animations of main loop
byte buttonState = 1;                          // state of current button to cycle animations
byte lastbuttonState = 0;                      // state of last button check, used for state switching checking
byte buttonchanged = 0;                        // button edge detected
unsigned long previousMillis = 0;              // time of last animation update
unsigned long buttonPreviousMillis = 0;        // time of last button state switch
unsigned long nesButtonMillis = 0;             // delay to de-bounce nes buttons
unsigned long animationCycleMillis = 0;        // time of last animation cycle
byte animation_number = 0;                     // indicator for which animation to play
int16_t animation_interval = 100;              // delay before looping though current animation again
byte random_type = 0;                          // used to randomize the type field for certain animations

// store ir sense data for 128 sensors
volatile byte ir_sense_data[128];
byte ir_cal_on[128];                           // calibrated "on" state for each IR node
byte ir_cal_off[128];                          // calibrated "off" state for each IR node
byte ir_node_state[128];                       // current IR node state, needed for fades, delays, and other effects.
byte ir_fade_count[128];                       // timer for each node for fade/delay effects
volatile byte ir_group = 0;                    // to track current IR group
volatile byte ir_group_adder = 0;              // needed in interrupt to set appropriate IR node voltages read from array
unsigned long previousSenseMillis = 0;         // time of last sense state change
const int16_t fade_color_interval = 6000;      // interval on which to switch to new fade color
const byte fade_timer = 2;                     // number of light_sense animation cycles to wait between fading down LEDs
int16_t sense_color_on = random(1, 512);       // random color for light sense on, cycles on 10s interval
int16_t sense_color_off = 64;
byte ir_detected = 0;                          // flag to be used in other animations to indicate that IR data has been detected

// comment out below, used to time interrupt cycle for debug
/*
unsigned long interrupt_timer_start = 0;
unsigned long interrupt_timer_end = 0;
unsigned long interrupt_total = 0;
long interrupt_counter = 100000;
*/

// for BT communicatoin
String currentLine = "";

// variables used for interrupt routine to refresh led array
volatile byte row_count = 0;                    // row counter when refreshing array

// variable for interrupt routine to read NES controller state
volatile byte nes_state0 = B00000000;
volatile byte nes_state1 = B00000000;
volatile boolean l;

//variables for nes paint animation
int16_t color = 0;
int16_t nes_location = 0;
byte current_code[14] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
byte last_button = 99;
byte color_select = 0;

// variables used for blink_rand_interval() animation
byte blink_count = 55;

// variables used for random fade and firework animation
const byte fadeLeds = 49;
byte wait_interval[511];                         // interval for which to leave an LED off before turning it back on, randomize this guy at setup time
int16_t led_to_fade[fadeLeds];                   // array of 50 random LEDs to work with
int16_t fwait_min = 10;  //10                    // min time to leave led off for random blink animation ( in count of animation_interval cycles )
int16_t fwait_max = 40;  //80
byte fw_state[fadeLeds];                         // animation state for LEDs firework animation
int16_t fade_color[fadeLeds];
byte firework_count = 3;


// variables for bounce animation
byte balls = 3;
const byte ball_max = 40;
byte location_x[ball_max];
byte location_y[ball_max];
byte direction_x[ball_max];   // 1 = right, 0 = left
byte direction_y[ball_max];   // 1 = down, 0 = up
byte speed_x[ball_max];
byte speed_y[ball_max];
byte interval_counter = 0;
int16_t ball_color[ball_max];
byte shape[ball_max];
byte fade_step = 1;

//variables for static/rfill
byte fill_dir = 0;
int16_t rfill_array[512];
int16_t current_pos = 0;

// variables for pong
byte paddle_width = 4;
byte red_row[20];
byte orange_row[20];
byte yellow_row[20];
byte green_row[20];
byte blue_row[20];
int16_t score = 0;

//variables for wave
float angle = 0;
int16_t previous_loc[64];
float dx = 1.0;   //sets wavelength displayed on panel ( higher = smaller wavelength )
float dy = 1.0;
int16_t led_mask[512];
byte wave_rate = 32;
byte wave_height = 8;
int16_t wave_color[3] = {0, 7, 448}; // bit 0 = static or random cycling wave color, bits 1 and 2 = colors used for static wave color
byte fill_wave = 1;

//variables for line sweep
byte line_type[8] = {0, 0, 0, 0, 0, 0, 0, 0};
short cur_x[8] = {0, 0, 8, 8, 16, 16, 24, 24}; // these are shorts because of possible negative values!  leave them this way!
short cur_y[8] = {0, 8, 0, 8, 0, 8, 0, 8};

//variables for rain animation
byte drop_state[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte drop_pos_y[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte drop_wait_count[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte drop_stack_top[32] = {16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16};
int16_t drop_color[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const byte drop_wait_time = 35;
byte drop_fall_timer[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float gravity = 0.1;
byte drop_stack_limit[32] = {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5};

//variable for text scrolling
String scrollText = "Vote now:  boy or girl?";
String scrollTime = String(millis());
int16_t text_color = 6;
int16_t character_origin[140];  // grr. fixed length  140 is alot though.. right, twitter?

// vars for sprite animations
byte sprite_mode = 0;
unsigned long mode_time = 0;

// vars for stopwatch
long stopwatch_time = 0;
long stopwatch_start_time = 0;
long prev_stopwatch_time = 0;
byte stopwatch_running = 0;

// vars for chase
byte dir = 0;
byte time_chaser_cycle = 0;
byte chaser_cycle = 1;
byte last_x[3] = {0,0,0};
byte last_y[3] = {0,0,0};
byte dir_count[3] = {0,0,0};
byte dir_threshold[3] = {2,2,2};
byte x_mode[3] = {0,0,0};
byte y_mode[3] = {0,0,0};


// variables for tetris
byte partloc_x = partstart_x;       // x and y location of of current piece
byte partloc_y = partstart_y;
byte part_type = random(0, 7);      // type of current piece
byte next_part = random(0, 7);      // type of next piece
byte part_o = 0;                    // rotation of current piece
unsigned long previousUpdate = 0;
int16_t fall_rate = 500;                // interval in ms between part falls
byte tetris_stack[ledCount];        // state of current board, shows if a current location in the board has a piece in it or not ( for stacking )
byte pauseState = 0;                // is the game paused?
byte previousPauseButtonState = 1;  // the previous state of the pause button for edge detect
byte gameOver = 0;

// vars for snake
const byte snakeLength = 70;        // make snake length = 50 segments
int16_t snake1[snakeLength];        // array for each snake, value != 999 means segment is alive
int16_t snake2[snakeLength];
byte dir1 = 0;                      // snake direction
byte dir2 = 0;
byte dead1 = 0;
byte dead2 = 0;
byte players = 1;
int16_t food1 = 0;
int16_t food2 = 0;
byte foodmode = 1;

// vars for life
byte gamestate[512];				// state of the game board, each cell is alive(1) or dead(0)
int iteration = 0;					// current generation

// vars for maze
byte maze_nodes[105];				// board broken down into 105 nodes to create the maze in by removing walls between nodes, node state is either visited or not
byte maze_walls[188];				// there are 188 walls between all the nodes, wall state is either there or not
int16_t current_cell = 0;			// current cell being tracked for generation/solving algorithm
byte nodestack[105];				// array to use as lifo to track visited nodes as maze is travered
char stack_index = 0;				// ptr to top of nodestack ( array location of last "pushed" node )
byte startnode = 0;
byte endnode = 104;
byte visted_nodes[105];				// since not using recursion when solving maze ( for looping and displaying purposes ) need to store array of visited nodes during maze solve				
byte wall_update = 0;				// indicate to solver that we just want to update the state of the wall LED this run through the loop, makes sole animation smoother
byte next_cell = 0;					// to smoothen animation, need to store next node found 


// vars for vote, reused           previous variable used, make sure to init properly!
// int16_t boyCount 	   		// food1 field from snake
// int16_t girlCount	 		// food2 field from snake
// unsigned long buttonFilter 	// stopwatch_time from stopwatch 
// byte boyLastButtonState		// dead1 from snake
// byte girlLastButtonState		// dead2 from snake
// byte boyState				// dir1 from snake
// byte girlState				// dir2 from snake
// int16_t resultLoop			// current_cell from maze
// byte scoreLastButtonState	// foodmode from snake
// byte firsToMax				// next_cell from maze


 

// the setup routine runs once when you press reset:
void setup()  {

  if ( debug > 0 )  Serial.begin(38400);
  
  // setup SPI for panel control
  SPI.setBitOrder(MSBFIRST);            //Most Significant Bit First
  SPI.setDataMode(SPI_MODE0);           // Mode 0 Rising edge of data, keep clock low
  SPI.setClockDivider(8);               //Run the data in at 84MHz / 6 = 16MHz 
  //SPI.setClockDivider(21);
  
  if ( bluetooth == 1 ) BTconfig(debug);	// init BT board if using it

  // set up data direction registers for LED control
  pinMode(30, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
  pinMode(shiftclkPin, OUTPUT);
  pinMode(ledDataPin, OUTPUT);

  // set initial state for shift reg pins
  latch_low;
  blank_high;
  digitalWrite(shiftclkPin, LOW);
  digitalWrite(ledDataPin, LOW);

  // set up data direction registers for NES control
  pinMode(NES_clk0, OUTPUT);  // nes outputs manually set as well, none of this register manipulation stuff yet.
  pinMode(NES_latch0, OUTPUT);
  pinMode(NES_data0, INPUT);
  pinMode(NES_clk1, OUTPUT);  // nes outputs manually set as well, none of this register manipulation stuff yet.
  pinMode(NES_latch1, OUTPUT);
  pinMode(NES_data1, INPUT);

  
  NES_latch0_low;
  NES_clk0_low;
 
  // check if there is a controller plugged into port1 ( main NES port )
  // latch button state into shift reg
  NES_latch1_high;
  delayMicroseconds(5);  // need to add some dly, was getting empty readings from one of the controllers
  NES_latch1_low;

  // shift data input from NES controllers
  for ( byte i = 0; i < 8; i++) {
    bitWrite(nes_state1, i, digitalRead(NES_data1));
    NES_clk1_high;
    NES_clk1_low;
  }
  
  if ( nes_state1 > 0 ) controller_plugged = 1;


  // clear initial state of display memory and set up stuff for animations
  //init stuff for animations
  clear_all(0, ledCount, display_mem);

  //map display memory pointer to display memory
  display_ptr0 = display_mem;
  display_ptr1 = display_mem + 8;
  display_ptr2 = display_mem + 16;
  display_ptr3 = display_mem + 24;

  // set button to be used as input to cycle though animation sequences
  pinMode(cycle_button, INPUT);
  
  // define voter box buttons as inputs
  pinMode(vote_boy_button, INPUT);
  pinMode(vote_girl_button, INPUT);
  pinMode(vote_results_button, INPUT);

  // set pin mode for ir array enable
  pinMode(ir_array_enable, OUTPUT);
  digitalWrite(ir_array_enable, LOW);

  //pin modes for IR sense muxes
  analogReadResolution(8);
  pinMode(sense_select0, OUTPUT);
  pinMode(sense_select1, OUTPUT);
  pinMode(sense_select2, OUTPUT);
  pinMode(sense_select3, OUTPUT);

  //ir_group_adder = (ir_group >> 1) + 8*(ir_group & B00000001);
  digitalWrite(sense_select0, bitRead(ir_group, 3));    // sense mux MSB
  digitalWrite(sense_select1, bitRead(ir_group, 2));
  digitalWrite(sense_select2, bitRead(ir_group, 1));
  digitalWrite(sense_select3, bitRead(ir_group, 0));    // sense mux LSB

  /* useful table for understanding interrupt mapping to timer channels
  TC  	Chan   	NVIC "irq"   	IRQ handler function   	PMC id
  TC0	0		TC0_IRQn		TC0_Handler	     	 	ID_TC0  << current example
  TC0	1		TC1_IRQn		TC1_Handler	       	 	ID_TC1
  TC0	2		TC2_IRQn		TC2_Handler	       		ID_TC2
  TC1	0		TC3_IRQn		TC3_Handler	        	ID_TC3
  TC1	1		TC4_IRQn		TC4_Handler	        	ID_TC4
  TC1	2		TC5_IRQn		TC5_Handler	       	 	ID_TC5
  TC2	0		TC6_IRQn		TC6_Handler	        	ID_TC6
  TC2	1		TC7_IRQn		TC7_Handler	        	ID_TC7
  TC2	2		TC8_IRQn		TC8_Handler	        	ID_TC8
  */

  // *****************************************************************************************************************************
  // DUE timers are completely freaking different from mega timers.. this will be fun
  //
  // Will try to use timer channel0, counter 0
  //
  // TC0 channel 0 will be used to refresh LED states
  // will use a clock divide of 8 to get down to 10.5MHz (0.0952us period)
  // then count up 1920 ticks to get ~183us between refreshes (~5.5kHz update )

  // found code online, hopefully can muddle through it
  PMC->PMC_PCER0 = 1 << 27; // PMC: Enable Timer TC0,0 ( clock enable for peripheral ID 27 which is TC0,0

  // set up timer registers
  REG_TC0_WPMR = 0x54494D00; // enable write to registers
  //             33222222222211111111110000000000
  //             10987654321098765432109876543210
  REG_TC0_CMR0 = 0b00000000000000001100000000000001; // set channel mode register (see datasheet)

  // CMR0 bits set
  // 15 = WAVE: waveform mode selected
  // 14:13 = WAVSEL, up mode w/ trigger on RC compare ( important for counter matching )
  // 2:0 = TCCLKS, timer selection ( divide_by ) = 1  ( SYSCLK/8 )

  //REG_TC0_RC0=0x001B0000; 	// counter period, slow for voltage testing
  //REG_TC0_RC0=0x01F50000; 	// counter period, super slow for voltage testing
  REG_TC0_RC0 = 0x00000780; 	// counter period   780h = 1920  was 700
  REG_TC0_CCR0 = 0b101;  		// start counter
  REG_TC0_IER0 = 0b00010000; 	// enable interrupt on counter=rc
  REG_TC0_IDR0 = 0b11101111; 	// disable other interrupts

  NVIC_EnableIRQ(TC0_IRQn); // enable TC0 interrupts

  if ( debug == 1 )  Serial.println("Interrupts started! ");

  // ******************************************************************************************************************************
  // TC1 channel 0 will be used for controller input polling and response
  // will use a clock divide of 8 to get down to 10.5MHz (0.0952us period)
  // then count up 131k ticks to get us a 12.5ms ( roughly 80/s interval ) to poll the controller inputs
  // can make this longer if we want to but 80/s sounds good

  // found code online, hopefully can muddle through it
  PMC->PMC_PCER0 = 1 << 30; // PMC: Enable Timer TC1,0 ( clock enable for peripheral ID 30 which is TC1,0

  // set up timer registers
  REG_TC1_WPMR = 0x54494D00; // enable write to registers
  //             33222222222211111111110000000000
  //             10987654321098765432109876543210
  REG_TC1_CMR0 = 0b00000000000000001100000000000001; // set channel mode register (see datasheet)

  // CMR0 bits set
  // 15 = WAVE: waveform mode selected
  // 14:13 = WAVSEL, up mode w/ trigger on RC compare ( important for counter matching )
  // 2:0 = TCCLKS, timer selection ( divide_by ) = 1  ( SYSCLK/8 )
  // 2
  REG_TC1_RC0 = 0x00030000; 	// counter period
  REG_TC1_CCR0 = 0b101;  		// start counter
  REG_TC1_IER0 = 0b00010000; 	// enable interrupt on counter=rc
  REG_TC1_IDR0 = 0b11101111; 	// disable other interrupts

  NVIC_EnableIRQ(TC3_IRQn); 	// enable TC1 interrupts


  // **********************************************************************************************************************************
  // TC1 channel 1 will be used for IR array sensing
  // will use a clock divide of 8 to get down to 10.5MHz (0.0952us period)
  // then count up 6144 ticks to get us a 585us  ( roughly 1.7kHz ) to poll the controller inputs

  // found code online, hopefully can muddle through it
  PMC->PMC_PCER0 = 1 << 31; // PMC: Enable Timer TC1,1 ( clock enable for peripheral ID 31 which is TC1,1

  // set up timer registers
  REG_TC1_WPMR = 0x54494D00; // enable write to registers
  //             33222222222211111111110000000000
  //             10987654321098765432109876543210
  REG_TC1_CMR1 = 0b00000000000000001100000000000001; // set channel mode register (see datasheet)

  // CMR1 bits set
  // 15 = WAVE: waveform mode selected
  // 14:13 = WAVSEL, up mode w/ trigger on RC compare ( important for counter matching )
  // 2:0 = TCCLKS, timer selection ( divide_by ) = 1  ( SYSCLK/8 )

  REG_TC1_RC1 = 0x00001800; 	// counter period
  REG_TC1_CCR1 = 0b101;  		// start counter
  REG_TC1_IER1 = 0b00010000; 	// enable interrupt on counter=rc   // *HERE!
  REG_TC1_IDR1 = 0b11101111; 	// disable other interrupts

  NVIC_EnableIRQ(TC4_IRQn); // enable TC1 interrupts
  
  // lastly start up SPI
  SPI.begin();

  //calibrate IR sensor array
  calibrate(cal);

  //check controller connectivity.. how are you going to do this again?
  nes_on();

  // display welcome screen
  splash(0, display_mem);
  clear_all(0, ledCount, display_mem);
  
  /*
	// setup state properly to go right into voter mode    SJH*******
	animation_interval = 50;
	food1 = 0;
	food2 = 0;
	stopwatch_time = millis();
	dead1 = 0;
	dead2 = 0;
	dir1 = 0;
	dir2 = 0;
	current_cell = 0;
	foodmode = 0;
	next_cell = 0;
	for ( int16_t i = 0; i <= scrollText.length(); i++ ) {
		character_origin[i] = 130 + 6 * i;
	}
	for ( byte i = 0; i < fadeLeds; i++) {
		led_to_fade[i] = (32 * random(3, 13)) + random(3, 28); // constrain initial points so fireworks will be fully on panel
		fw_state[i] = 0;
		wait_interval[i] = fwait_min;
	}
  */


  if (debug == 6 ) {

    Serial.print("scrollText length: ");
    Serial.println( scrollText.length() );

    Serial.print("Nes controller0 state: ");
    Serial.println(nes_state0);

    Serial.print("PCER: ");
    Serial.println(PMC->PMC_PCSR0);

    analogReadResolution(10);
    Serial.print("ADC 10-bit (default) : ");
    Serial.print(analogRead(A0));

    // change the resolution to 12 bits and read A0
    analogReadResolution(12);
    Serial.print(", 12-bit : ");
    Serial.print(analogRead(A0));

    // change the resolution to 16 bits and read A0
    analogReadResolution(16);
    Serial.print(", 16-bit : ");
    Serial.print(analogRead(A0));

    // change the resolution to 8 bits and read A0
    analogReadResolution(8);
    Serial.print(", 8-bit : ");
    Serial.println(analogRead(A0));

    Serial.println("cal_on");
    for ( byte i = 0; i < 128; i++ ) {
      Serial.print(ir_cal_on[i]);
      Serial.print(", ");
    }

    Serial.println("");
    Serial.println("cal_off");
    for ( byte i = 0; i < 128; i++ ) {
      Serial.print(ir_cal_off[i]);
      Serial.print(", ");
    }

    Serial.println("");
    Serial.println("Sense data:");
    float sum = 0;
    for ( byte i = 0; i < 128; i++ ) {
      Serial.print(ir_sense_data[i]);
      Serial.print(", ");
      sum = sum + ir_sense_data[i];
    }

    Serial.println("");
    Serial.print("dark average: ");
    Serial.println(sum / 128.0);

  }

}


// the loop routine runs over and over again forever, well, forever is a bit of a stretch
void loop()  {

  // grab button state to see if we want to cycle to next animation in sequence
  buttonState = digitalRead(cycle_button);

  if (buttonState != lastbuttonState && ( millis() - buttonPreviousMillis > 600 )  ) { // button state has changed
    buttonchanged = 1;
    lastbuttonState = buttonState;
    buttonPreviousMillis = millis();
  }
  else {
    buttonchanged = 0;
  }

  // next check if the pause button is pressed on controller (note, only works for animations which nes state is active ) if so, pause animation by not calling the animation function until it is unpaused
  if ( bitRead(nes_state1, 3) == 0 && pauseState == 0 && previousPauseButtonState == 1 ) {   // button is pressed and current state is unpaused, pause the game
    pauseState = 1;
    previousPauseButtonState = 0;
  } else if ( bitRead(nes_state1, 3) == 0 && pauseState == 1 && previousPauseButtonState == 1 ) {  // button is pressed and current state is paused,  unpause the game
    pauseState = 0;
    previousPauseButtonState = 0;
  } else if ( bitRead(nes_state1, 3) == 1 && previousPauseButtonState == 0 ) {  // button is released
    previousPauseButtonState = 1;
  }
  
  // only certain animations can be paused with the start button, for non-pausable animations, clear the pause button back to zero, also clear gameOver for non-tetris modes
  if ( animation_sequence[animation_number] != 14 && animation_sequence[animation_number] != 15 ) pauseState = 0;
  if ( animation_sequence[animation_number] != 14 ) gameOver = 0;
  

  // user can start new tetris game by pressing start when current game is over
  if ( animation_sequence[animation_number] == 14 && gameOver == 1 && bitRead(nes_state1, 3) == 0 ) {
    clear_all(0, ledCount, display_mem);
    setup_tetris(ledCount, tetris_stack, &part_type, &next_part, display_mem, partstart_y, &gameOver, &previousPauseButtonState, &fall_rate, &score);
  }

  // if button is pressed, cycle to next animation
  if (buttonState == LOW && buttonchanged == 1 ) {
    //move to next animation
    next_animation(99);
    animationCycleMillis = millis();
  }

  // if cycle interval for animation has run it's course, scroll to next animation
  if ( cycle == 1 && millis() - animationCycleMillis > cycle_time ) {
    next_animation(99);
    animationCycleMillis = millis();
  }

  //call bluetooth client to process any bluetooth data
  if ( bluetooth == 1 ) bluetooth_client(debug);

  // main loop,  if animation interval has expired, make another call to selected animation function
  if ( millis() - previousMillis > animation_interval && pauseState == 0 && gameOver == 0 ) {

    previousMillis = millis();

    // call animation function for array
    switch (animation_sequence[animation_number]) {
		case 0:
			// "paint" function using NES controller
			nes_paint(&nes_state1, &last_button, display_mem, rfill_array, ledCount, &color, &nes_location, current_code, debug, code_array, &animation_number, &animation_interval, tetris_stack, &part_type, &next_part, partstart_y, &gameOver, &previousPauseButtonState, &fall_rate, &score, &color_select );
			break;
		case 1:
			// toggle each led in array to full brightness at random intervals for random amounts of time
			blink_rand_interval(0, &blink_count, ledCount, display_mem, wave_color, &sense_color_on, nes_state1, &animation_interval);
			break;
		case 2:
			// bounce
			bounce(random_type, display_mem, &balls, ball_max, location_x, location_y, direction_x, direction_y, speed_x, speed_y, &interval_counter, ball_color, shape, ledCount, nes_state1, &animation_interval, &fade_step );
			break;
		case 3:
			// rain
			rain(random_type, display_mem, nes_state1, drop_state, drop_pos_y, drop_wait_count, drop_color, drop_wait_time, drop_fall_timer, &gravity, drop_stack_top, drop_stack_limit, debug );
			break;
		case 4:
			// clear all ( random static color )
			//clear_all(512 + random_type);
			//clear_all(512);  // 514
			rfill(random_type, display_mem, ledCount, nes_state1, &fill_dir, rfill_array, &current_pos, wave_color, &sense_color_on, &animation_interval);
	
			break;
		case 5:
			// random fade
			// variables used for random fade and firework animation
			random_fade(random_type, display_mem, ledCount, fadeLeds, wait_interval, led_to_fade, fwait_min, fwait_max, fw_state, fade_color, firework_count, debug, &animation_interval );
			break;
		case 6:
			// wave animation
			wave(random_type, display_mem, nes_state1, &angle, previous_loc, &dx, &dy, led_mask, &wave_rate, &wave_height, wave_color, &fill_wave, &previousSenseMillis, debug);
			break;
		case 7:
			// random line sweep across array
			lines(random_type, display_mem, ledCount, line_type, cur_x, cur_y, rows, cols, debug );
			break;
		case 8:
			// led chaser
			chaser(random_type, ledCount, display_mem, nes_state1, &dir, &time_chaser_cycle, chaser_cycle, last_x, last_y, cur_x, cur_y, fade_color, &random_type, &animation_interval, &previousSenseMillis, fade_color_interval, dir_count, dir_threshold, x_mode, y_mode, debug);
			break;
		case 9:
			// light sense using IR array
			if ( random_type == 4 ) {
				light_sense(4, display_mem, ledCount, ball_color, location_x, location_y, ir_sense_data, ir_cal_on, ir_cal_off, ir_node_state, &sense_color_on, &sense_color_off, &previousSenseMillis, led_mask, &ir_detected, ir_fade_count, fade_timer, fade_color_interval, debug );
				wave(3, display_mem, nes_state1, &angle, previous_loc, &dx, &dy, led_mask, &wave_rate, &wave_height, wave_color, &fill_wave, &previousSenseMillis, debug);
			} else {
				light_sense(random_type, display_mem, ledCount, ball_color, location_x, location_y, ir_sense_data, ir_cal_on, ir_cal_off, ir_node_state, &sense_color_on, &sense_color_off, &previousSenseMillis, led_mask, &ir_detected, ir_fade_count, fade_timer, fade_color_interval, debug );
			}
			break;
		case 10:
			// scroll text message across screen
			//light_sense(2);
	
			scrollTime = String(millis());
			if ( random_type > 0 ) {
			//          type, text,    text_size,               color_type
			text_scroll(1, scrollText, scrollText.length() + 1, 0, display_mem, ledCount, text_color, character_origin, ir_detected, debug ); //random_type);
			} else {
			//          type, text,    text_size,             color_type
			text_scroll(2, scrollText, scrollText.length() + 1, 0, display_mem, ledCount, text_color, character_origin, ir_detected, debug ); //random_type);
			}
			break;
		case 11:
			// sprite animation w/ frames read from pgm memory
			sprite_animate(0, 200, display_mem, nes_state1, ledCount, &sprite_mode, &mode_time);
			break;
		case 12:
			// stopwatch function
			stopwatch(0, &stopwatch_time, &stopwatch_start_time, &prev_stopwatch_time, &stopwatch_running, display_mem, nes_state1, debug);
			break;
		case 13:
			// serial control over array via processing application
			serial_control(display_mem, ball_color, ledCount, nes_location);
			break;
		case 14:
			// call main tetris function.  always last animation nubmer
			tetris(&partloc_x, &partloc_y, &part_type, &next_part, &part_o, &previousUpdate, &fall_rate, tetris_stack, &pauseState, &previousPauseButtonState, &gameOver, &score, display_mem, ledCount, nes_state1, &nesButtonMillis, debug);
			break;
		case 15:
			snake(&players, &score, display_mem, ledCount, nes_state1, nes_state0, snake1, snake2, &dir1, &dir2, &dead1, &dead2, &food1, &food2, &foodmode, snakeLength, &animation_interval, debug);
			break;
		case 16:
			life(&random_type, &iteration, display_mem, ledCount, gamestate, nes_state1, &animation_interval, wave_color, debug);
			break;
		case 17: 
			// maze solver
			maze_solve(&current_cell, &endnode, maze_nodes, maze_walls, nodestack, &stack_index, display_mem, &wall_update, wave_color, nes_state1, &animation_interval, &mode_time, &random_type, &next_cell, debug );
			break;
		case 18:
			// boy/girl voter ** note makes use of variables from other animations
			voter(&food1, &food2, display_mem, nes_state1, &stopwatch_time, &dead1, &dead2, &foodmode, &dir1, &dir2, character_origin, &current_cell, &animation_interval, &next_cell, wait_interval, led_to_fade, fw_state, fade_color, debug );
			break;
		case 19:
			// pong / breakout
			pong( random_type, display_mem, ledCount, nes_state1, &nes_location, &paddle_width, &score, direction_x, direction_y, location_x, location_y, speed_x, speed_y, &interval_counter, ball_color, debug, red_row, orange_row, yellow_row, green_row, blue_row, &animation_interval );	
			break;
		

    }

    // comment out below, used to time interrupt cycle for testing
    /*if ( interrupt_counter < 5 ) {
        Serial.println(interrupt_total);
        interrupt_counter = 5000000;
      }*/

    
    if(debug == 2 ){
      Serial.print( animation_sequence[animation_number] );

      Serial.print( "NES controller0 state:  " );
      Serial.print(nes_state0);
      Serial.print( ",   NES controller1 state:  " );
      Serial.println(nes_state1);
	  
	  Serial.print( "tetris gameover: ");
	  Serial.println(gameOver);
	  
	  Serial.print( "controller plugged: ");
	  Serial.println(controller_plugged);
	  
	  /*
      Serial.print("ir_group: ");
      Serial.print(ir_group);
      Serial.print(" ");
      Serial.print("ir_group_adder: ");
      Serial.print(ir_group_adder);
      Serial.print("  ");
      Serial.print(ir_sense_data[79]);
      Serial.print(" ");
      Serial.print(ir_sense_data[95]);
      Serial.print(" ");
      Serial.print(ir_sense_data[111]);
      Serial.print(" ");
      Serial.println(ir_sense_data[127]);

      Serial.print(analogRead(A0));
      Serial.print(" ");
      Serial.print(analogRead(A1));
      Serial.print(" ");
      Serial.print(analogRead(A2));
      Serial.print(" ");
      Serial.println(analogRead(A3));
	  */

    }
  }
}

// **************************************************************************************************************************************
// *************************************  INTERRUPTS HERE **********************************************

/* useful table for nuserstanding interrupt mapping to timer channels
TC        	Chan   	NVIC "irq"   	IRQ handler function   	PMC id
TC0	0	TC0_IRQn	TC0_Handler	ID_TC0  << current example
TC0	1	TC1_IRQn	TC1_Handler	ID_TC1
TC0	2	TC2_IRQn	TC2_Handler	ID_TC2
TC1	0	TC3_IRQn	TC3_Handler	ID_TC3
TC1	1	TC4_IRQn	TC4_Handler	ID_TC4
TC1	2	TC5_IRQn	TC5_Handler	ID_TC5
TC2	0	TC6_IRQn	TC6_Handler	ID_TC6
TC2	1	TC7_IRQn	TC7_Handler	ID_TC7
TC2	2	TC8_IRQn	TC8_Handler	ID_TC8
*/

// interrupt routune to control LED state on timer interrupt
void TC0_Handler() {

  TC_GetStatus(TC0, 0);

  // comment out below, used to time interrupt cycle for testing
  //interrupt_counter--;
  //interrupt_timer_start = micros();

  int16_t pixel_shade0[8];  //temporary storage for shade comparisons
  int16_t pixel_shade1[8];
  int16_t pixel_shade2[8];
  int16_t pixel_shade3[8];

  byte shift_byte_r0 = B00000000; // bytes build up during interrupt to be shifted out to led driver shift regs
  byte shift_byte_g0 = B00000000;
  byte shift_byte_b0 = B00000000;
  byte shift_byte_r1 = B00000000;
  byte shift_byte_g1 = B00000000;
  byte shift_byte_b1 = B00000000;
  byte shift_byte_r2 = B00000000;
  byte shift_byte_g2 = B00000000;
  byte shift_byte_b2 = B00000000;
  byte shift_byte_r3 = B00000000;
  byte shift_byte_g3 = B00000000;
  byte shift_byte_b3 = B00000000;


  //use lookup table to determine rgb led brightness

  //assemble rgb shift bytes for each row / grayscale   8 columns controlled by each shift reg
  for ( byte i = 0; i < 8; i++ ) {

    //grab pixel color for current led in row
    pixel_shade0[i] = *display_ptr0;
    pixel_shade1[i] = *display_ptr1;
    pixel_shade2[i] = *display_ptr2;
    pixel_shade3[i] = *display_ptr3;

    //lookup shade RGB values from flash lookup table, store to shift_byte to be sent to led driver

    // col0:7
    if ( pgm_read_byte_near( &lookup_red[pixel_shade0[i]] )   > shade_compare ) {       // if current display_mem value for r/g/b component is greater than the current
		bitWrite(shift_byte_r0, i, 1);													// grayscale comparison, the led for this interrupt cycle is on
    }
    if ( pgm_read_byte_near( &lookup_green[pixel_shade0[i]] ) > shade_compare ) {
		bitWrite(shift_byte_g0, i, 1);
    }
    if ( pgm_read_byte_near( &lookup_blue[pixel_shade0[i]] )  > shade_compare ) {
      	bitWrite(shift_byte_b0, i, 1);
    }

    // col8:15
    if ( pgm_read_byte_near( &lookup_red[pixel_shade1[i]] )   > shade_compare ) {
		bitWrite(shift_byte_r1, i, 1);
    }
    if ( pgm_read_byte_near( &lookup_green[pixel_shade1[i]] ) > shade_compare ) {
		bitWrite(shift_byte_g1, i, 1);
    }
    if ( pgm_read_byte_near( &lookup_blue[pixel_shade1[i]] )  > shade_compare ) {
		bitWrite(shift_byte_b1, i, 1);
    }

    // col16:23
    if ( pgm_read_byte_near( &lookup_red[pixel_shade2[i]] )   > shade_compare ) {
      bitWrite(shift_byte_r2, i, 1);
    }
    if ( pgm_read_byte_near( &lookup_green[pixel_shade2[i]] ) > shade_compare ) {
      bitWrite(shift_byte_g2, i, 1);
    }
    if ( pgm_read_byte_near( &lookup_blue[pixel_shade2[i]] )  > shade_compare ) {
      bitWrite(shift_byte_b2, i, 1);
    }

    // col24:31
    if ( pgm_read_byte_near( &lookup_red[pixel_shade3[i]] )   > shade_compare ) {
      bitWrite(shift_byte_r3, i, 1);
    }
    if ( pgm_read_byte_near( &lookup_green[pixel_shade3[i]] ) > shade_compare ) {
      bitWrite(shift_byte_g3, i, 1);
    }
    if ( pgm_read_byte_near( &lookup_blue[pixel_shade3[i]] )  > shade_compare ) {
      bitWrite(shift_byte_b3, i, 1);
    }

    display_ptr0++;
    display_ptr1++;
    display_ptr2++;
    display_ptr3++;

  }

  // moved blanking of reg outputs after all the shade calculation/lookup stuff.  hopefully this means the LEDs will stay on longer during each interrupt sequence
  blank_high;
  delayMicroseconds(20);  // needed to add a bit of dly here. was getting some ghosting between rows
  
  
  // now send rgb bytes for current row out to shift regs, send out data for column 24:31 first, then 16:23, followed by cols 8:15 / rows 8:15 first, followed by 0:7 since driver boards are daisy-chained
  
  SPI.transfer(shift_byte_r3,SPI_CONTINUE);
  SPI.transfer(shift_byte_g3,SPI_CONTINUE);
  SPI.transfer(shift_byte_b3,SPI_CONTINUE);
    
  SPI.transfer(shift_byte_r2,SPI_CONTINUE);
  SPI.transfer(shift_byte_g2,SPI_CONTINUE);
  SPI.transfer(shift_byte_b2,SPI_CONTINUE);
    
  SPI.transfer(shift_byte_r1,SPI_CONTINUE);
  SPI.transfer(shift_byte_g1,SPI_CONTINUE);
  SPI.transfer(shift_byte_b1,SPI_CONTINUE);
  SPI.transfer(row_cntl1[row_count],SPI_CONTINUE);
    
  SPI.transfer(shift_byte_r0,SPI_CONTINUE);
  SPI.transfer(shift_byte_g0,SPI_CONTINUE);
  SPI.transfer(shift_byte_b0,SPI_CONTINUE);
  SPI.transfer(row_cntl0[row_count]);
  
  

  // latch shift reg data into output latches and enable register outputs
  latch_high;
  latch_low;
  blank_low;

  // update row count so next interrupt, we refresh next row, wrap at top to bottom
  // once we complete all rows, incr shade_compare to update for next brightness level
  // once we complete all shades, start whole loop over again

  if ( row_count == rows - 1 ) {
    row_count = 0;
    display_ptr0 = display_mem;
    display_ptr1 = display_mem + 8;
    display_ptr2 = display_mem + 16;
    display_ptr3 = display_mem + 24;
    shade_compare++;
    if ( shade_compare > shade_limit ) {
      shade_compare = 0;
    }
  }
  else {
    row_count++;
    display_ptr0 = display_ptr0 + 24;
    display_ptr1 = display_ptr1 + 24;
    display_ptr2 = display_ptr2 + 24;
    display_ptr3 = display_ptr3 + 24;
  }

  // comment out below.  used to time interrupt cycle for testing
  //interrupt_timer_end = micros();
  //interrupt_total = interrupt_total + ( interrupt_timer_end - interrupt_timer_start );
}


// interrupt routine to read data from attached NES controllers
void TC3_Handler() {

  TC_GetStatus(TC1, 0);
  //long dummy=REG_TC1_SR0; // vital - reading this clears some flag
  // otherwise you get infinite interrupts

  // latch button state into shift reg
  NES_latch0_high;
  NES_latch1_high;
  delayMicroseconds(5);  // need to add some dly, was getting empty readings from one of the controllers
  NES_latch0_low;
  NES_latch1_low;

  // shift data input from NES controllers
  for ( byte i = 0; i < 8; i++) {
    bitWrite(nes_state0, i, digitalRead(NES_data0));
    bitWrite(nes_state1, i, digitalRead(NES_data1));
    NES_clk0_high;
    NES_clk1_high;
    NES_clk0_low;
    NES_clk1_low;
  }
  
  if (nes_state1 == 0 ) nes_state1 = 255;

}

// interrupt routine to read IR array
void TC4_Handler() {

  TC_GetStatus(TC1, 1);
  //long dummy=REG_TC1_SR0; // vital - reading this clears some flag
  // otherwise you get infinite interrupts

  // now read analog inputs and store sense data into sense array
  ir_sense_data[64 + ir_group_adder] = analogRead(A0);
  ir_sense_data[80 + ir_group_adder] = analogRead(A1);
  ir_sense_data[96 + ir_group_adder] = analogRead(A2);
  ir_sense_data[112 + ir_group_adder] = analogRead(A3);

  ir_sense_data[0 + ir_group_adder] = analogRead(A6);
  ir_sense_data[16 + ir_group_adder] = analogRead(A7);
  ir_sense_data[32 + ir_group_adder] = analogRead(A4);
  ir_sense_data[48 + ir_group_adder] = analogRead(A5);

  // update sensor group and mux selects for next time through
  if (ir_group < 15 ) {
    ir_group++;
  } else {
    ir_group = 0;
  }

  ir_group_adder = (ir_group >> 1) + 8 * (ir_group & B00000001);

  // update mux controls for next round ( 4 bit mux control )
  digitalWrite(sense_select0, bitRead(ir_group, 3));    // sense mux MSB
  digitalWrite(sense_select1, bitRead(ir_group, 2));
  digitalWrite(sense_select2, bitRead(ir_group, 1));
  digitalWrite(sense_select3, bitRead(ir_group, 0));    // sense mux LSB

}


// **************************************************************************************************************************************
// **********************************    helpers     ******************************************

// set up parameters for next animation and move animation number
void next_animation(byte switchto) {
	
  byte random_shape = 0;
  byte temp_color = 0;
  //char *animation_name = "test name is long";
  String animation_name = "test name is long";
  char nameToSend[] = "test name is long";

  if ( switchto == 99 ) {
    // increment through the animation sequence array, if we hit the end of the array, reset
    animation_number++;
    if (animation_sequence[animation_number] == 99 ) {
      animation_number = 0;
    }
  } else {
    // jump to requested animation number
    animation_sequence[16] = switchto;
    animation_number = 16;
  }

  // now set up for the next animation based on the sequence array
  clear_all(0, ledCount, display_mem);
  nes_off();
  ir_off();
  //Serial.end();
  
  byte color_mode;

  switch (animation_sequence[animation_number]) {

	case 0:

		// nes_paint
		nes_location = 0;
		color = 0;
		nes_on();
		pauseState = 0;
		gameOver = 0;
		animation_interval = 100;
		clear_all(0, ledCount, display_mem);
		animation_name = "nes paint";
		break;
	
	case 1:
		
		// blink_rand_interval
		nes_on();
		blink_count = random(10, 70);
		animation_interval = random(20, 70);
		animation_name = "random blink";
		break;

    case 2:
      
		// bounce
		nes_on();
		random_type = random(0, 4);
		random_shape = random(0, 3);
		for ( byte i = 0; i < ball_max; i++ ) {
			direction_x[i] = random(0, 2);
			direction_y[i] = random(0, 2);
			location_x[i] = random(1, 30);
			location_y[i] = random(2, 15);
			speed_x[i] = B00000001 << random(0, 3);
			//speed_y[i] = speed_x[i];
			speed_y[i] = B00000001 << random(0, 3);
			//ball_color[i] = (7*random(0,2)) + (56*random(0,2))+(448*random(0,2));
			ball_color[i] = random(0, 512);
			shape[i] = random_shape;
		}
		animation_interval = 30;
		animation_name = "bounce";
		break;

    case 3:

		// rain
		
		nes_on();
		temp_color = random(1, 512);
		random_type = random(0, 2);
		color_mode = random(0,2);
		for ( byte i = 0; i < 32; i++ ) {
			drop_wait_count[i] = random(1, drop_wait_time);
			if ( color_mode == 0 ) {
			drop_color[i] = temp_color;
			} else {
			drop_color[i] = random(1, 512);
			}
			drop_state[i] = 0;
			drop_pos_y[i] = 0;
			drop_fall_timer[i] = 0;
			drop_stack_top[i] = 16;
			drop_stack_limit[i] = random(3,14);
			gravity = (random(1, 11) / 10.0);
		}
	
		animation_interval = 30;
		animation_name = "rain";
		break;
	
	case 4:

		// rfill
		nes_on();
		current_pos = 0;
		rfill_init(0, ledCount, rfill_array);
		rfill_init(1, ledCount, rfill_array);
		fill_dir = 1;
		random_type = random(0, 3);
		wave_color[0] = random(0,2);
		animation_interval = 5;
		animation_name = "fill";
		break;

    case 5:

		// random_fade
		nes_off();
		clear_all(0, ledCount, display_mem);
		nes_location = 0;
		color = 0;
		randomSeed(analogRead(0));
		random_type = random(0, 2);
		firework_count = random(1, 6);
		for ( byte i = 0; i < fadeLeds; i++) {
			led_to_fade[i] = (32 * random(3, 13)) + random(3, 28); // constrain initial points so fireworks will be fully on panel
			fw_state[i] = 0;	 //random(3, 13) for full panel
			wait_interval[i] = fwait_min;
		}
		animation_interval = 50;  //100
		animation_name = "random fade";
		if ( random_type == 1 ) animation_name = "fireworks";
		break;

    case 6:

		// wave
		for ( int16_t i = 0; i < ledCount; i++ ) {
			led_mask[i] = 0x0FFF;
		}
		random_type = random(0, 7);
		//random_type = 2;
		wave_color[0] = random(0, 3);
		previousSenseMillis = millis();
		if ( random_type == 2 ) wave_color[1] = random(0,2);
		nes_location = 0;
		nes_on();
		color = 0;
		angle = 0;
		animation_interval = 50;
		animation_name = "wave";
		break;

    case 7:

		//line_sweep
		random_type = random(0, 4);
	
		for ( byte i = 0; i < 8; i++) {
			line_type[i] = 0;
			cur_x[i] = 8 * (i / 2);
		}
	
		animation_interval = 30;
	
		if (random_type == 3 ) {
			animation_interval = random(30, 150);
			cur_x[0] = random(1, 6);
		}
		animation_name = "lines";
		break;

    case 8:

		// chaser
		nes_on();
		random_type = random(0, 10);
		previousSenseMillis = millis();
		for ( byte i=0; i<3; i++) {
			cur_x[i] = 0;
			cur_y[i] = 0;
			dir_count[i] = 0;
			dir_threshold[i] = random(2, 7);
			x_mode[i] = 0;
			y_mode[i] = 0;
			fade_color[i] = random(1, 512);	
		}
		animation_interval = 10;
		animation_name = "chaser";
		break;

    case 9:

		// light_sense
		byte temp_cal_on[128];            // temp storage for ir_ca_on array getting corrupted
		for ( byte i=0; i<128; i++ ) {
			temp_cal_on[i] = ir_cal_on[i];
		}
		
		for ( int16_t i = 0; i < ledCount; i++ ) {
			led_mask[i] = 0;
			if ( i < 128 ) {
				ir_sense_data[i] = 0;
			}
		}
		random_type = random(0, 5);
		//random_type = 4;
		if ( random_type == 4 ) {
			dx = 0.1;
			wave_rate = 200;
		}
		location_x[0] = 2 * random(1, 15);
		location_y[0] = 2 * random(1, 7) + 1;
		
		ir_on();
		previousSenseMillis = millis();
		animation_interval = 15;
		
		// correct the ir_cal_on array getting corrupted in/after the ir_on() call ( seriously, how the fuck is that happening?? )
		for ( byte i=0; i<128; i++ ) {
			ir_cal_on[i] = temp_cal_on[i];
		}
		animation_name = "sense";
		break;

    case 10:

		// text_scroll
		scrollTime = String(millis());
		random_type = random(1, 3);
		random_type = 1;
	
		if ( random_type > 0 ) {
			for ( int16_t i = 0; i <= scrollText.length(); i++ ) {
			character_origin[i] = 130 + 6 * i;
			}
		} else {
			for ( int16_t i = 0; i <= scrollTime.length(); i++ ) {
			character_origin[i] = 130 + 6 * i;
			}
		}
	
		// uncomment when using text scroll w/ light sensing
		// ir_on();
	
		//text_color = random(1,512);
		animation_interval = 25;
		animation_name = "text scroll";
		break;
	
    case 11:

		// sprite animate
		mode_time = millis();
		nes_on();
		animation_interval = 100;
		animation_name = "sprite";
		break;

    case 12:

		// stopwatch
		stopwatch_time = 0;
		nes_on();
		animation_interval = 50;
		animation_name = "stopwatch";
		break;

    case 13:

		// serial contrl
		nes_location = 0;
		Serial.begin(19200);
		animation_interval = 10;
		break;

    case 14:

		// in easter egg tetris, set up led state for nes_paint
		nes_location = 0;
		color = 0;
		nes_on();
		pauseState = 0;
		gameOver = 0;
		animation_interval = 100;
		animation_name = "tetris!";
		break;
      
	case 15:

		// snake
		nes_on();
		pauseState = 0;
		previousPauseButtonState == 1;
		gameOver = 0;
		for ( byte i=0; i < snakeLength; i++ ) {
			snake1[i] = 999;
			snake2[i] = 999;
		}
		
		// set initial snake positions
		snake1[0] = 263;  
		snake1[1] = 262;
		snake1[2] = 261;
		snake1[3] = 260;
				
		dir1 = 2;
		dir2 = 1;
		dead1 = 0;
		dead2 = 0;
		players = 1;
		
		if ( players == 2 ) {
			snake2[0] = 312;  
			snake2[1] = 313;
			snake2[2] = 314;
			snake2[3] = 315;
		}
		
		foodmode = 0;  // one pellet mode
		newFood(snake1, snake2, snakeLength, &food1, &food2, foodmode, 0 );
		animation_interval = 50;
		animation_name = "snake";
		break;
		
	case 16:

		// life
		nes_on();
		random_type = random(0, 3);
		animation_interval = 50;
		wave_color[0] = random(1,512);
		init_life(gamestate, &iteration, ledCount, wave_color);
		animation_name = "life";
		break;
		
	case 17:
	
		// maze
		nes_on();
		animation_interval = 20;
		//wave_color[0] = random(1,512);
		wave_color[0] = 3;
		wave_color[1] = random(1,512);
		init_maze(maze_nodes, maze_walls, nodestack, &stack_index, ledCount, display_mem, &current_cell, wave_color, &random_type, &wall_update, debug);
		animation_name = "maze";
		break;
		
	case 18:
		
		// voter
		nes_on();
		animation_interval = 50;
		animation_name = "voter";
		food1 = 0;
		food2 = 0;
		stopwatch_time = millis();
		dead1 = 0;
		dead2 = 0;
		dir1 = 0;
		dir2 = 0;
		current_cell = 0;
		foodmode = 0;
		next_cell = 0;
		for ( int16_t i = 0; i <= scrollText.length(); i++ ) {
			character_origin[i] = 130 + 6 * i;
		}
		for ( byte i = 0; i < fadeLeds; i++) {
			led_to_fade[i] = (32 * random(3, 13)) + random(3, 28); // constrain initial points so fireworks will be fully on panel
			fw_state[i] = 0;
			wait_interval[i] = fwait_min;
		}
		break;	
		
	case 19:
	
		// pong / breakout ... need to fix this
		nes_on();
		nes_location = 494;   // starting paddle location
		random_type = 1;
		direction_x[0] = random(0, 2);
		direction_y[0] = random(1, 2);
		location_x[0] = random(0, 32);
		location_y[0] = random(0, 3);
		speed_x[0] = B00000001 << random(0, 3);
		speed_y[0] = speed_x[0];
		
		if ( random_type == 1 ) {
			for ( byte i = 0; i < 20; i++ ) {
				red_row[i] = 1; 
				orange_row[i] = 1;
				yellow_row[i] = 1;
				green_row[i] = 1;
				blue_row[i] = 1;
			}
			
			location_x[0] = random(6, 25);
			location_y[0] = 8;
			
			speed_x[0] = B00000001 << random(2, 3);
			speed_y[0] = speed_x[0];
			
		}
		
		//speed_y[i] = B00000001 << random(0,3);
		//ball_color[i] = (7*random(0,2)) + (56*random(0,2))+(448*random(0,2));
		ball_color[0] = random(0, 512);
		interval_counter = 0;
		
		animation_interval = 30;
		animation_name = "pong";
		break;
	
	}
	
	// at end, transmit the current animation number over the BT connection
	// would also like to transmit animation interval and animation type back to phone, need to figure out how
	if (bluetooth == 1 ) {
		
		animation_name += "&";
		animation_name += String(animation_sequence[animation_number]);
		animation_name += "&";
		animation_name += String(random_type);
		animation_name += "&";
		animation_name += String(animation_interval);
		animation_name.toCharArray(nameToSend, animation_name.length()+1);
		Serial1.write(nameToSend);
		//Serial1.write("test send");
		
		if (debug > 0 ) Serial.println(nameToSend);
	}
	
	
}

// calibration function for IR array
void calibrate(byte caltype) {

  ir_on();

  //first check if sense data suggests that we're in a dark room, if so, just load default dark room settings to save time
  if ( caltype == 2 ) {
    float sum = 0;
    delay(1000);
    for ( byte i = 0; i < 128; i++ ) {
      sum = sum + ir_sense_data[i];
    }

    // if ir data average is less than a predetermined threshold, just load dark thresholds for each node from flash, otherwise, proceed with calibration
    caltype = 1;
    if ( sum / 128.0 < 20 ) caltype = 2;

  }


  // if sense data shows we're in a lighter environment, run through calibration procedure for each ir node
  if ( caltype == 1 ) {
    unsigned long cal_prev_millis = 0;

	
    for ( int16_t j = 0; j < 512; j++ ) {

      // set board to current led color/brightness
      clear_all(j, ledCount, display_mem);

      // wait a bit for sense interrupts to read full array state
      cal_prev_millis = millis();
      while ( millis() - cal_prev_millis < 50 ) {
        // do nothing, just wait for IR array to get updated w/ values from current ambient LED state
      }

      // threshold is simple max of read IR sense value across all LED brightness, this also takes ambient light into account
      for ( byte i = 0; i < 128; i++ ) {
        ir_cal_on[i] = max(ir_sense_data[i], ir_cal_on[i]);
      }

    }


    // second calibratoin type: instead of turning on all LEDs at once, turn on only the 4 for each node to full brightness immediately
    // doesn't really work.
    /*
    int term0 = 0;
    for ( byte i=0; i<=127; i++ ) {

      clear_all(0, ledCount, display_mem);
      term0 = (64*(i/16)) + (2*(i%16));

      display_mem[term0] = 511;
      display_mem[term0 + 1] = 511;
      display_mem[term0 + 32] = 511;
      display_mem[term0 + 33] = 511;

      // wait a bit for sense interrupts to read full array state
      cal_prev_millis = millis();
      while( millis() - cal_prev_millis < 100 ) {
        // do nothing, just wait for IR array to get updated w/ values from current ambient LED state
      }

      // threshold is simple max of read IR sense value across all LED brightness, this also takes ambient light into account
      ir_cal_on[i] = max(ir_sense_data[i], ir_cal_on[i]);

    }*/
	
	/*

    // do the same thing now in reverse
    for ( byte i=127; i>0; i-- ) {

      clear_all(0, ledCount, display_mem);
      term0 = (64*(i/16)) + (2*(i%16));

      display_mem[term0] = 511;
      display_mem[term0 + 1] = 511;
      display_mem[term0 + 32] = 511;
      display_mem[term0 + 33] = 511;

      // wait a bit for sense interrupts to read full array state
      cal_prev_millis = millis();
      while( millis() - cal_prev_millis < 100 ) {
        // do nothing, just wait for IR array to get updated w/ values from current ambient LED state
      }

      // threshold is simple max of read IR sense value across all LED brightness, this also takes ambient light into account
      ir_cal_on[i] = max(ir_sense_data[i], ir_cal_on[i]);

    } */
    

    //set on threshold 5% higher than max ambient value found for each node
    for ( byte i = 0; i < 128; i++ ) {

      // manually tune calibrated values.. need to make more sensitive w/o false hits
      if ( i > 31 && i < 48 ) {
        //ir_cal_on[i] = ir_cal_on[i] * 1.20;
        ir_cal_off[i] = ir_cal_on[i] * 0.90;
      } else if ( i < 16 ) {
        //ir_cal_on[i] = ir_cal_on[i] * 1.10;
        ir_cal_off[i] = ir_cal_on[i] * 0.90;
      } else if ( i > 63 && i < 80 ) {
        //ir_cal_on[i] = (ir_cal_on[i]) * 1.25;  // row 4 is sensitive, raise his threshold further
        ir_cal_off[i] = ir_cal_on[i] * 0.95;
      } else {
        //ir_cal_on[i] = (ir_cal_on[i] + 1) * 1.20; // 20% above max ambient level is threshold
        ir_cal_off[i] = ir_cal_on[i] * 0.90; // 10% below on threshold for some hysterisis
      }

      ir_node_state[i] = 0;

    }

  } else if (caltype == 0 ) {
    for ( byte i = 0; i < 128; i++ ) {
      ir_cal_on[i] = 255;
    }

  } else if (caltype == 2 ) {        // set ir thresholds from previous dark room calibrated values
    for ( byte i = 0; i < 128; i++ ) {
      ir_cal_on[i] = pgm_read_byte_near(&dark_cal_on[i]);
      ir_cal_off[i] = pgm_read_byte_near(&dark_cal_off[i]);
    }
  }

  ir_off();
  clear_all(0, ledCount, display_mem);

}

void ir_on() {
	digitalWrite(ir_array_enable, HIGH);
	REG_TC1_IER1 = 0b00010000; // enable interrupt on counter=rc
	REG_TC1_IDR1 = 0b11101111; // disable other interrupts
}

void ir_off() {
  digitalWrite(ir_array_enable, LOW);
  REG_TC1_IER1 = 0b00000000; // enable interrupt on counter=rc
  REG_TC1_IDR1 = 0b11111111; // disable other interrupts
}

void nes_on() {
	
	if ( controller_plugged == 1 ) {
		REG_TC1_IER0 = 0b00010000; // enable interrupt on counter=rc
		REG_TC1_IDR0 = 0b11101111; // disable other interrupts
	}
}

void nes_off() {
	REG_TC1_IER0 = 0b00000000; // enable interrupt on counter=rc
	REG_TC1_IDR0 = 0b11111111; // disable other interrupts
}

// bluetooth client, this will be used to process request from bluetooth for panel control
void bluetooth_client(byte debug) {
	
	// android application operates by sending bluetooth serial data with different tokens the micro controller will parse out and use
	// current tokens:
	// L* = simple clear of all LEDs
	// C* = color request, set current color of all LEDs on table to provided value
	// A* = animation request, set table animation to provided animation number
	// I* = interval request, set table animation interval
	// T* = animation cycle time, used to tell table to either cycle animations based on a timer or only cycle on table button push
	// S* = update animation options (same as T*) and then RESET table
	// R* = random type for animation, used to vary settings within a particular animation
	// O1* = option1.  generic changable option.  the actual function of this option will depend on which animation is currently active
	
	char data;
	byte lineComplete = 0;
	
	while( Serial1.available() > 0 )
	{
		data = (char)Serial1.read();
		currentLine += data;
   
	}
	
	if ( data == '*' ) {
		if ( debug == 1 ) Serial.println(currentLine);
		lineComplete = 1;
	}
	
	// once we got a full command from the serial port, parse it and respond accordingly
	if ( lineComplete == 1 ) {
	
		// Check to see what the request was for the current line
		
		// clear LEDs request
        if (currentLine.endsWith("L*")) {  // turn off all LEDs on /L request
          clear_all(0, ledCount, display_mem);
          if ( debug == 1 ) {
            Serial.print("current line: ");
            Serial.println(currentLine);
          }
        }
		
		// set color request
		if (currentLine.endsWith("C*")) {

          // grab the index of the brightness token in the string
          byte index_s = currentLine.lastIndexOf(":");
          byte index_e = currentLine.lastIndexOf("C*");
          char colorstring[4];
          String brightnessString = currentLine.substring(index_s + 1, index_e);
          int16_t len = brightnessString.length();
          brightnessString.toCharArray(colorstring, len + 1);
          clear_all(atoi(colorstring), ledCount, display_mem);

          if ( debug == 1 ) {
            Serial.println(currentLine);
            Serial.print("string index of token is: ");
            Serial.println(currentLine.lastIndexOf(":"));
            Serial.print("string index of end token is: ");
            Serial.println(currentLine.lastIndexOf("*C"));
            Serial.print("brightnessString: ");
            Serial.println(brightnessString);
            Serial.print("Colorstring: ");
            Serial.println(colorstring);
            Serial.print("Color set to: ");
            Serial.println(atoi(colorstring));
          }
        }
		
		// to select current animation
        if (currentLine.endsWith("A*")) {

			// grab the index of the brightness token in the string
			byte index_s = currentLine.lastIndexOf(":");
			byte index_e = currentLine.lastIndexOf("A*");
			char animation[3];
			String animationString = currentLine.substring(index_s + 1, index_e);
			//char *colorstring;
			int16_t len = animationString.length();
			animationString.toCharArray(animation, len + 1);
			next_animation(atoi(animation));


			if ( debug == 1 ) {
				Serial.println(currentLine);
				Serial.print("string index of token is: ");
				Serial.println(currentLine.lastIndexOf(":"));
				Serial.print("string index of end token is: ");
				Serial.println(currentLine.lastIndexOf("*A"));
				Serial.print("animationString: ");
				Serial.println(animationString);
				Serial.print("animation: ");
				Serial.println(animation);
			}
        }
		
		// to set animation interval  format will be ":<interval>*I"
		if (currentLine.endsWith("I*")) {
			
			// grab the index of the token in the string
			byte index_s = currentLine.lastIndexOf(":");
			byte index_e = currentLine.lastIndexOf("I*");
			char interval[4];
			String intervalString = currentLine.substring(index_s + 1, index_e);
			//char *colorstring;
			int16_t len = intervalString.length();
			intervalString.toCharArray(interval, len + 1);
			animation_interval = atoi(interval);
			
			if ( debug == 1 ) {
				Serial.println(currentLine);
				Serial.print("string index of token is: ");
				Serial.println(currentLine.lastIndexOf(":"));
				Serial.print("string index of end token is: ");
				Serial.println(currentLine.lastIndexOf("*I"));
				Serial.print("inervalString: ");
				Serial.println(intervalString);
				Serial.print("interval: ");
				Serial.println(interval);
			}
			
		}
		
		// to set animation cycling properties
		if (currentLine.endsWith("T*")) {
			
			// format for string will be ":<TRUE/FALSE>&<cycle time in ms>&<TRUE/FALSE>*T"
			char interval[6];
			
			
			// grab the index of the token in the string and the cal boolean
			byte index_s = currentLine.lastIndexOf("&");
			byte index_e = currentLine.lastIndexOf("T*");
			String animationString = currentLine.substring(index_s + 1, index_e);
			cal = (1 ? animationString == "true" : 0);
			
			// next grab cycle time from string
			index_s = currentLine.indexOf("&");
			index_e = currentLine.lastIndexOf("&");
			if ( index_e > index_s ) {
				String animationString = currentLine.substring(index_s + 1, index_e);
				int16_t len = animationString.length();
				animationString.toCharArray(interval, len + 1);
				cycle_time = atoi(interval);
			}
			
			
			//last, parse out state of check box to determine if animations cycling is enabled or not
			index_s = currentLine.lastIndexOf(":");
			index_e = currentLine.indexOf("&");
			animationString = currentLine.substring(index_s + 1, index_e);
			if ( animationString == "true" ) {
				cycle = 1;
			} else {
				cycle = 0;
			}
			
			if ( debug == 1 ) {
				Serial.println(currentLine);
				Serial.print("option string index of start token is: ");
				Serial.println(currentLine.lastIndexOf(":"));
				Serial.print("option string index of end token is: ");
				Serial.println(currentLine.lastIndexOf("*T"));
				Serial.print("option cal: ");
				Serial.println(cal);
				Serial.print("option interval: ");
				Serial.println(interval);
				Serial.print("option cycle state: ");
				Serial.println(cycle);
			}
			
		}
		
		// to set animation cycling properties AND reset the table
		if (currentLine.endsWith("S*")) {
			
			// format for string will be ":<TRUE/FALSE>&<cycle time in ms>&<TRUE/FALSE>*T"
			char interval[6];
			
			// grab the index of the token in the string and the cal boolean
			byte index_s = currentLine.lastIndexOf("&");
			byte index_e = currentLine.lastIndexOf("S*");
			String animationString = currentLine.substring(index_s + 1, index_e);
			cal = (1 ? animationString == "true" : 0);
			
			if ( debug ) {
				Serial.print("animationString: ");
				Serial.println(animationString);
			}
			
			// next grab cycle time from string
			index_s = currentLine.indexOf("&");
			index_e = currentLine.lastIndexOf("&");
			if ( index_e > index_s ) {
				String animationString = currentLine.substring(index_s + 1, index_e);
				int16_t len = animationString.length();
				animationString.toCharArray(interval, len + 1);
				cycle_time = atoi(interval);
			}
					
			//last, parse out state of check box to determine if animations cycling is enabled or not
			index_s = currentLine.lastIndexOf(":");
			index_e = currentLine.indexOf("&");
			animationString = currentLine.substring(index_s + 1, index_e);
			if ( animationString == "true" ) {
				cycle = 1;
			} else {
				cycle = 0;
			}

			// clean some stuff up before issuing reset via setup function
			buttonState = 1;                      
			lastbuttonState = 0;                    
			buttonchanged = 0;               
			previousMillis = 0;              
			buttonPreviousMillis = 0;        
			nesButtonMillis = 0;             
			animationCycleMillis = 0;        
			animation_number = 0;            
			animation_interval = 100;        
			random_type = 0; 
			clear_all(0, ledCount, display_mem);
			
			// calibrate again if selected
			calibrate(cal);
			
			nes_on();
			
			splash(0, display_mem);
			clear_all(0, ledCount, display_mem);
			
		}
		
		
		// to set animation type  format will be ":<type>*R"
		if (currentLine.endsWith("R*")) {
			
			// grab the index of the token in the string
			byte index_s = currentLine.lastIndexOf(":");
			byte index_e = currentLine.lastIndexOf("R*");
			char type[4];
			String typeString = currentLine.substring(index_s + 1, index_e);
			//char *colorstring;
			int16_t len = typeString.length();
			typeString.toCharArray(type, len + 1);
			random_type = atoi(type);
			
			if ( debug == 1 ) {
				Serial.println(currentLine);
				Serial.print("string index of token is: ");
				Serial.println(currentLine.lastIndexOf(":"));
				Serial.print("string index of end token is: ");
				Serial.println(currentLine.lastIndexOf("*I"));
				Serial.print("typeString: ");
				Serial.println(typeString);
				Serial.print("random type: ");
				Serial.println(type);
			}
			
		}
		
		// to set generic animation option1 ( varies by current animation, slider based to single value passed  
		if (currentLine.endsWith("O1*")) {
			
			// grab the index of the brightness token in the string
			byte index_s = currentLine.lastIndexOf(":");
			byte index_e = currentLine.lastIndexOf("O1*");
			char dataval[4];
			String datavalString = currentLine.substring(index_s + 1, index_e);
			//char *colorstring;
			int16_t len = datavalString.length();
			datavalString.toCharArray(dataval, len + 1);
			
			// only one implemented w/ option1 value so far
			switch ( animation_sequence[animation_number]) {
				
				case 1: 
					blink_count = atoi(dataval);
					if ( debug == 1 ) Serial.println("set led count");
					break;
				
				case 3:
					gravity = (float)atof(dataval);
					if ( debug == 1 ) Serial.println("set gravity");
					break;
					
			}
				
			if ( debug == 1 ) {
				Serial.println(currentLine);
				Serial.print("option1 string index of token is: ");
				Serial.println(currentLine.lastIndexOf(":"));
				Serial.print("option1 string index of end token is: ");
				Serial.println(currentLine.lastIndexOf("*I"));
				Serial.print("option1 datavalString: ");
				Serial.println(datavalString);
				Serial.print("option1 dataval: ");
				Serial.println(dataval);
			}
			
		}

		
	
		// lastly clear the currentLine for next command
		currentLine = "";
	}

  /*
  WiFiClient client = server.available();   // listen for incoming clients

  if ( debug == 1 && server.status() == 0 ) {
    //server_uptime = millis() - server_uptime;
    Serial.println("server status = 0!");
    Serial.println(millis());
  }

  if (client) {                             // if you get a client,
    if (debug == 1 ) Serial.println("got client!");

    String currentLine = "";                // make a String to hold incoming data from the client
    char c;
    //requestTimer = millis();              // time how long it takes to get the GET web request to actually set the LED

    while (client.connected()) {            // loop while the client's connected

      //while(client.available()) {
      if (client.available()) {               // if there's bytes to read from the client,

        c = client.read();                    // read a byte, then
        if (c == '\n') {                      // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:

          if (currentLine.length() == 0) {
            // break out of the while loop:
            client.println("HTTP/1.0 200 OK");
            client.println("Content-type:text/html");
		    // The HTTP response ends with another blank line:
            client.println();
            break;
          } else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see what the request was for the current line
        if (currentLine.endsWith("/L")) {  // turn off all LEDs on /L request
          clear_all(0, ledCount, display_mem);
          if ( debug == 1 ) {
            Serial.print("current line: ");
            Serial.println(currentLine);
          }
        }

        // for a color request, need to parse out the brightness value from the get string
        if (currentLine.endsWith("*C")) {

          // grab the index of the brightness token in the string
          byte index_s = currentLine.lastIndexOf(":");
          byte index_e = currentLine.lastIndexOf("*C");
          char colorstring[4];
          String brightnessString = currentLine.substring(index_s + 1, index_e);
          int16_t len = brightnessString.length();
          brightnessString.toCharArray(colorstring, len + 1);
          clear_all(atoi(colorstring), ledCount, display_mem);

          if ( debug == 1 ) {
            Serial.println(currentLine);
            Serial.print("string index of token is: ");
            Serial.println(currentLine.lastIndexOf(":"));
            Serial.print("string index of end token is: ");
            Serial.println(currentLine.lastIndexOf("*C"));
            Serial.print("brightnessString: ");
            Serial.println(brightnessString);
            Serial.print("Colorstring: ");
            Serial.println(colorstring);
            Serial.print("Color set to: ");
            Serial.println(atoi(colorstring));
          }
        }

        // to select current animation
        if (currentLine.endsWith("*A")) {

			// grab the index of the brightness token in the string
			byte index_s = currentLine.lastIndexOf(":");
			byte index_e = currentLine.lastIndexOf("*A");
			char animation[3];
			String animationString = currentLine.substring(index_s + 1, index_e);
			//char *colorstring;
			int16_t len = animationString.length();
			animationString.toCharArray(animation, len + 1);
			next_animation(atoi(animation));


			if ( debug == 1 ) {
				Serial.println(currentLine);
				Serial.print("string index of token is: ");
				Serial.println(currentLine.lastIndexOf(":"));
				Serial.print("string index of end token is: ");
				Serial.println(currentLine.lastIndexOf("*A"));
				Serial.print("animationString: ");
				Serial.println(animationString);
				Serial.print("animation: ");
				Serial.println(animation);
			}
        }
		
		// to set animation interval
		if (currentLine.endsWith("*I")) {
			
			// grab the index of the brightness token in the string
			byte index_s = currentLine.lastIndexOf(":");
			byte index_e = currentLine.lastIndexOf("*I");
			char interval[4];
			String intervalString = currentLine.substring(index_s + 1, index_e);
			//char *colorstring;
			int16_t len = intervalString.length();
			intervalString.toCharArray(interval, len + 1);
			animation_interval = atoi(interval);
			
			if ( debug == 1 ) {
				Serial.println(currentLine);
				Serial.print("string index of token is: ");
				Serial.println(currentLine.lastIndexOf(":"));
				Serial.print("string index of end token is: ");
				Serial.println(currentLine.lastIndexOf("*I"));
				Serial.print("inervalString: ");
				Serial.println(intervalString);
				Serial.print("interval: ");
				Serial.println(interval);
			}
			
		}
		
		// to set animation cycling properties
		if (currentLine.endsWith("*T")) {
			
			// format for string will be /T<TRUE/FALSE>*<cycle time in ms>*T
			
			// grab the index of the brightness token in the string
			byte index_s = currentLine.lastIndexOf("&");
			byte index_e = currentLine.lastIndexOf("*T");
			char interval[6];
			String animationString = currentLine.substring(index_s + 1, index_e);
			//char *colorstring;
			int16_t len = animationString.length();
			animationString.toCharArray(interval, len + 1);
			cycle_time = atoi(interval);
			
			// also parse out state of check box to determine if animations cycling is enabled or not
			index_s = currentLine.lastIndexOf(":");
			index_e = currentLine.lastIndexOf("&");
			animationString = currentLine.substring(index_s + 1, index_e);
			if ( animationString == "TRUE" ) {
				cycle = 1;
			} else {
				cycle = 0;
			}
			
		}
      }
    }

    // close the connection:
    if ( debug == 1 ) {
      requestTimer = millis();              // time how long it takes to get the GET web request to actually set the LED
    }

    client.stop();

    if ( debug == 1 ) {
      Serial.print("request timer: ");
      Serial.println(millis() - requestTimer);
      Serial.println("client disonnected");
    }
  } */
  
}

// setup function for USART0 SPI to run panel instead of main hardware SPI which now runs the wifi shield
// need this because panel update is interrupt driven SPI in interrupt will fuck up any current communication with wifi shield
void USART0config() {

  //PMC->PMC_PCER0 = 1<<17;         // turn on clocks for peripheral for USART0

  USART0->US_WPMR = 0x55534100;   // Unlock the USART Mode register

  // reset and disable receiver and transmitter
  USART0->US_CR = US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS ;

  USART0->US_MR = 0x000409CE;     // Set Mode to CLK0=1, 8_BIT, SPI_MASTER,
  USART0->US_BRGR = 0x000E;       // Clock Divider (SCK = 6MHz) 
  
  // Enable receiver and transmitter
  USART0->US_CR = US_CR_RXEN | US_CR_TXEN ;

  // set up the usart IOs
  PIOA->PIO_WPMR = 0x50494F00;    // Unlock PIOA Write Protect Mode Register
  PIOB->PIO_WPMR = 0x50494F00;    // Unlock PIOB Write Protect Mode Register

  PIOB->PIO_ABSR |= (0u << 25);   // CS: Assign B25 I/O to the Peripheral A function
  PIOB->PIO_PDR |= (1u << 25);    // CS: Disable PIO control, enable peripheral control

  PIOA->PIO_ABSR |= (1u << 17);   // SCK: Assign A17 I/O to the Peripheral B function
  PIOA->PIO_PDR |= (1u << 17);    // SCK: Disable PIO control, enable peripheral control
  PIOA->PIO_ABSR |= (0u << 10);   // MISO: Assign PA10 I/O to the Peripheral A function
  PIOA->PIO_PDR |= (1u << 10);    // MISO: Disable PIO control, enable peripheral control
  PIOA->PIO_ABSR |= (0u << 11);   // MOSI: Assign A11 I/O to the Peripheral A function
  PIOA->PIO_PDR |= (1u << 11);    // MOSI: Disable PIO control, enable peripheral control
}

void USART0status() {

  if ( debug == 1 ) {
    Serial.print("PIOA_ABSR \t");
    Serial.println(PIOA->PIO_ABSR, HEX);
    Serial.print("PIOA_PDR \t");
    Serial.println(PIOA->PIO_PDR, HEX);
    Serial.print("PIOB_ABSR \t");
    Serial.println(PIOB->PIO_ABSR, HEX);
    Serial.print("PIOB_PDR \t");
    Serial.println(PIOB->PIO_PDR, HEX);
    Serial.print("US_MR   \t");
    Serial.println(USART0->US_MR, HEX);
    Serial.print("US_CR   \t");
    Serial.println(USART0->US_CR, HEX);
    Serial.print("US_CSR  \t");
    Serial.println(USART0->US_CSR, HEX);
    Serial.print("US_CSR shift (txrdy)  \t");
    Serial.println(USART0->US_CSR & 0x00000002, HEX);
    Serial.print("US_CSR shift (txempty)  \t");
    Serial.println(USART0->US_CSR & 0x00000200, HEX);
    Serial.print("US_BRGR (HEX)\t");
    Serial.println(USART0->US_BRGR, HEX);
    Serial.print("US_BRGR (DEC)\t");
    Serial.println(USART0->US_BRGR, DEC);
    Serial.print("US_RTOR \t");
    Serial.println(USART0->US_RTOR, HEX);
    Serial.print("US_TTGR \t");
    Serial.println(USART0->US_TTGR, HEX);
    Serial.print("US_FIDI \t");
    Serial.println(USART0->US_FIDI, HEX);
    Serial.print("US_NER  \t");
    Serial.println(USART0->US_NER, HEX);
    Serial.print("US_IF   \t");
    Serial.println(USART0->US_IF, HEX);
    Serial.print("US_MAN  \t");
    Serial.println(USART0->US_MAN, HEX);
    Serial.print("US_LINMR \t");
    Serial.println(USART0->US_LINMR, HEX);
    Serial.print("US_LINIR \t");
    Serial.println(USART0->US_LINIR, HEX);
    Serial.print("US_WPSR \t");
    Serial.println(USART0->US_WPSR, HEX);
    Serial.print("PMC_PCSR0 \t");
    Serial.println(PMC->PMC_PCSR0, HEX);

  }
}


// setup for BT module, hopefully less complicated than the wifi since it's just being treated as a serial connection
void BTconfig(byte debug)
{

  if (debug == 1) {
	Serial.println("Config BT....");
  }
	
  // serial1 (USART0) used for BT communication, initialize it and set up connection to BT module
  Serial1.begin(38400); 
  
  Serial1.print("\r\n+STWMOD=0\r\n");  // set to slave
  Serial1.print("\r\n");
  delay(2000);
  //Serial1.print(\r\n+STNA=led_table\r\n");   // set name BT name to "led_table"  
  Serial1.print("\r\n+INQ=1\r\n");  // set to inquiry
  
  if (Serial1.available() > 0 ) {
	  int data = Serial1.read();
	  if( data > 0 && debug == 1 ) {
		  Serial.println("data from bt!");
		  Serial.println(data);
		  Serial.print((char)data);
	  }
  }
  
  //bluetooth.print("\r\n+STAUTO=1\r\n");  // set to auto pair  Serial.begin(38400);

}



