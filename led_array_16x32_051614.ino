/*
 
 512 LED 16x32 dimensional array, each pin independantly brightness modulated w/ bam method from Kevin Darrah videos (thanks!)
 
 code will "draw" one row's LED each interrupt cycle, so we'll need 16 interrupts to get though whole array
 ambient light sensor used to detect change in light level, triggering lights on/off  off is done via a fade out animation to add some smoothness
 the code for the light_sense animation accounts for the middle colum of LEDs to be shared between both ambient light sensors
 it operates such that either sensor catchinh shadow will enable the LEDs and will only fade them off once both sensors catch light again
 light_sense() coded to be generic for any NxM sized array, figures out sensor positions automatically and iterates though them, just need to change
 row/col variables and resize some arrays..  extra credit: work on sensor "sharing"
 added NES controller polling on timer2 interrupt
 on interrupt, latch button state into shift reg on contoller, scan out and display button state on LEDs
 NES controller used now to select animations ( via select button ) and also to pause any animation ( via the start button )
 adding SPI interface to load shitf register rather than call to shift out function
 12/23:  broke out red and blue led brightness as sperate shifted bytes
 added streak to rotate animation
 12/26:  first attempt at 4x4 array!
 12/29:  finally got 4x4 rgb array working! woo!
 todo:  reverse shift reg order to cut down on wires  DONE
 1/09 got NES control working, added some color tools
 1/16: added serial input to set individual led w/ serial monitor
 1/22: testing mbi5168 led current sink driver
 1/24: code update to add defines for IO functions, stated changes to brightness memory to get away from Darrah BAM method to update matrix
 -- i don't think i need 16 levels of brightness ( 16x16x16 = 4K color shared ), will try reduction down to 512, this should speed things up and allow me to increase
 	  number of rows in array from 4 to the 16 I eventually want to use
 
 2/10: added "painting" mode for light sense
 2/16: added shift reg for row control.  so shift row outputs via SPI now
 2/20: IMPORTTANT.  revered IR sensors.  pull-down resitors on analog input now instead of pull up, code changes to match ( calibrate function, etc.. )
 2/22: renamed to array_4x4_<datestamp>,  changed NES_move to paint w/ button press ability
 3/03: YAY NEW CAR!  added separate light sense calibration threshold for single color blue fade
 3/20: custom PCB!!!!  actually works!!!  w00t!!  swapped row ordering to match pcb silkscreen.  still need variable scrub for memory size 
 3.30: HOLY SHIT! FIRST 8X8 TEST!
 3.31: w00T!  got 16x16 2 board array working.  nothing fancy yet and perf. is kinda crappy.  will need better controller at some point.  cleaned up memory as well.  use flash to store lookup tables.
 4.1: w00T 2!  got nes_move working w/ paint function!!
 4.3: got 3rd set of columns working ( board jumpers work too! ), added random_blink animation and fixed bugs in paint.
 4.4: got full 16x32 array running!!!  as expected performance does suffer for some modes.   work on improving code, saving ram, and adding new animations.  got bounce working better now, still need to add speed
 4.5: added next_animation function again to cycle though animations on button press ( need to get pushbutton to use this with ), started adding welcome spash screen, need to put into program memory
 4.6: added serial dump of nes_paint screen if debug is enabled to make saving drawings possible, completed splash screen greeting 
 4.7: added edge flash on ball bounce strike
 4.9: fix to nes_paint for performance improvement, started pong, got basics of it down!
 4.13: added agrument for splash screen to all for alternate text displayed ( for example for start of each different animation )
 4.15: added trails to bounce for balls < 4, not much else.
 4.22: added serial control w/ mouse input
 4.23: added x/y cursors to mouse input, modified serial communication string a bit
 4.26: added wave animation ( initial )
 
 
animation_number        animation
0                       nes_paint
1                       blink_rand_interval  
2                       bounce
3                       pong game
4                       static color test ( clear_all w/ color argument ) 
5                       serial control test w/ processing
6                       wave
 
 */

#include <SPI.h>              // SPI Library used to clock data out to the shift registers
#include <avr/pgmspace.h>     // for use of program memory to store static data

// debug
const byte debug = 0;          // set to 1 to get serial data out for debug
const byte cal = 0;            // set to 0 to turn off calibration for other debug so we don't run it all the damn time   
const byte shade_limit = 6;    // "grayscale" shades for each color

// static defines 
#define blankPin     4         // PORTH4, board pin 7
#define shiftclkPin 52         // SPI clock, board pin 52
//#define latchPin     3	       // PORTD3, board pin 18
#define latchPin     0         // PORTG0, board pin 41
#define ledDataPin  51         // MOSI, board pin 51

#define blank_high   PORTH |=  (1 << blankPin)    // disables shift reg blank ( ENABLE outputs )
#define blank_low    PORTH &= ~(1 << blankPin)    // enable shift reg blank ( DISABLE outputs )
#define latch_high   PORTG |=  (1 << latchPin)    // enable latch to shift regs 
#define latch_low    PORTG &= ~(1 << latchPin)   // disable latch to shift regs  

#define NES_clk0     6	       // RED PORTB6, board pin 12
#define NES_latch0   5         // ORANGE PORTB5, board pin 11
#define NES_data0    10        // YELLOW PORTB4, board pin 10

//#define NES_clk1     0;      // PORTG0, board pin 41
//#define NES_latch1   6       // PORTL6, board pin 43
//#define NES_data1    44      // PORTL4, board pin 44

#define NES_clk0_high    PORTB |=  (1 << NES_clk0)
#define NES_clk0_low     PORTB &= ~(1 << NES_clk0)
#define NES_latch0_high  PORTB |=  (1 << NES_latch0)
#define NES_latch0_low   PORTB &= ~(1 << NES_latch0)
//#define NES_clk1_high    PORTG |=  (1 << NES_clk0)
//#define NES_clk1_low     PORTG &= ~(1 << NES_clk0)
//#define NES_latch1_high  PORTL |=  (1 << NES_latch0)
//#define NES_latch1_low   PORTL &= ~(1 << NES_latch0)

#define paintBluePin 2
#define paintRedPin 3
#define paintGreenPin 4

#define cycle_button 26                   // the pin the input button1 is attached to, will cycle animations ( if interval has expired )


// general variables used for LED control 
//               row 76543210
const byte row_cntl0[] = { 
                    B00000001,    // bytes to shift out to row control shift reg for rows 0:7
                    B00000010,
                    B00000100,
                    B00001000,
                    B00010000,
                    B00100000,
                    B01000000,
                    B10000000,
                    B00000000,
                    B00000000,
                    B00000000,
                    B00000000,
                    B00000000,
                    B00000000,
                    B00000000,
                    B00000000 };
                    
const byte row_cntl1[] = { 
                    B00000000,    // bytes to shift out to row control shift reg for rows 8:15
                    B00000000,
                    B00000000,
                    B00000000,
                    B00000000,
                    B00000000,
                    B00000000,
                    B00000000,
                    B00000001,
                    B00000010,
                    B00000100,
                    B00001000,
                    B00010000,
                    B00100000,
                    B01000000,
                    B10000000 };
                    
byte rows = 16;                     // row/col counts;
byte cols = 32;
int ledCount = 512;                // how many LEDs we dealin' wif
int shade_compare = 0;		  // compare value for brightness testing

// new byte array to store single brightness value for a each pixel, the value will be used to access a lookup table to determine appropriate r.g.b values to send to shift regs
int display_mem[512];		// main structure to hold LED array color state
byte led_state[512];
static int *display_ptr0;     	// pointer to display array for col0:7
static int *display_ptr1;       // pointer to display array for col8:15
static int *display_ptr2;       // pointer to display array for col16:23
static int *display_ptr3;       // pointer to display array for col24:31

// lookup tables to store r,g,b data for pixel color/brightness ( can also just calc value on the fly if you want to save 1.5KB of RAM
// each led as 8 possible brightnesses ( including off ) so that gives us 8x8x8 = 512 shades
// NOTE:  these are stored in program memory to save SRAM space

PROGMEM prog_uchar lookup_red[512] =  {    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                           2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
                                           3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
                                           4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                                           5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                                           6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                                           7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };
                                         
PROGMEM prog_uchar lookup_green[512] =  {  0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,
                                           0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,
                                           0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,
                                           0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,
                                           0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,
                                           0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,
                                           0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,
                                           0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7 }; 

PROGMEM prog_uchar lookup_blue[512] =  {   0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,
                                           0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,
                                           0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,
                                           0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,
                                           0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,
                                           0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,
                                           0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,
                                           0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7 };




// used to reading analog levels to adjust update interval / animations of main loop
byte buttonState = 0;                          // state of current button to cycle animations
byte lastbuttonState = 0;                      // state of last button check, used for state switching checking
byte buttonchanged = 0;                        // button edge detected
unsigned long previousMillis = 0;              // time of last annimation update
unsigned long buttonPreviousMillis = 0;        // time of last button state switch
unsigned long animationCycleMillis = 0;        // time of last animation cycle
byte animation_number = 0;                     // indicator for which animation to play
int animation_interval = 100;                  // delay before looping though current animation again

// comment out below, used to time interrupt cycle for debug
/*
unsigned long interrupt_timer_start = 0;
unsigned long interrupt_timer_end = 0;
unsigned long interrupt_total = 0;
long interrupt_counter = 100000;
*/

// variables used for interrupt routine to refresh led array 
volatile int row_count = 3;                    // row counter when refreshing array

// variable for interrupt routine to read NES controller state
volatile byte nes_state0 = B11111111;
volatile byte nes_state1 = B11111111;

//variables for nes paint animation
byte color = 0;
byte nes_red = 0;
byte nes_blue = 0;
byte nes_green = 0;
int nes_location = 0;

// variables used for blink_rand_interval() animation
/*
byte blink_interval[384];                 // interval for which to leave an LED on, randomize this guy at setup time
byte blink_min = 1;                       // min time to leave led on for random blink animation ( in count of animation_interval cycles )
byte blink_max = 2;                       // max time to leave led on for random blink animation ( in count of animation_interval cycles )
byte wait_interval[384];                   // interval for which to leave an LED off before turning it back on, randomize this guy at setup time
byte wait_min = 1;                        // min time to leave led off for random blink animation ( in count of animation_interval cycles )
byte wait_max = 2; 
*/

// variables for bounce animation
const byte balls = 7;
byte location_x[balls]; 
byte location_y[balls]; 
byte direction_x[balls];   // 1 = right, 0 = left
byte direction_y[balls];   // 1 = down, 0 = up
byte speed_x[balls];
byte speed_y[balls];
byte interval_counter = 0;
int ball_color[balls];

// variables for pong
byte paddle_width = 4;
byte score = 0;

//varialbes for serial
int previous_pos = 0;

//variable for wave
float angle = 0;
int previous_loc[64];
const float dx = 1.5*(TWO_PI / 32);
long cycles = 0;

// the setup routine runs once when you press reset:
void setup()  { 

  // setup SPI  
  SPI.setBitOrder(MSBFIRST);            //Most Significant Bit First
  SPI.setDataMode(SPI_MODE0);           // Mode 0 Rising edge of data, keep clock low
  SPI.setClockDivider(SPI_CLOCK_DIV2);  //Run the data in at 16MHz/2 - 8MHz

  // set up data direction registers for LED control
  DDRH = DDRH | B00010000;  // PORTH4 is output
  DDRD = DDRD | B00001000;  // PORTD3 is output
  pinMode(shiftclkPin, OUTPUT);
  pinMode(ledDataPin, OUTPUT);

  // set initial state for shifr reg pins
  digitalWrite(shiftclkPin, LOW);
  digitalWrite(ledDataPin, LOW);
  latch_low;
  blank_high;

  // set up data direction registers for NES control
  DDRB = DDRB | B01100000;  // PORTB 5:6 are outputs
  DDRG = DDRG | B00000001;  // PORTG 0 is output
  DDRL = DDRL | B01000000;  // PORTL 6 is output 
  pinMode(NES_data0, INPUT);
  //pinMode(NES_data1, INPUT);

  // set initial state for NES controller bits
  NES_clk0_low;
  NES_latch0_low;
  //NES_clk1_low;
  //NES_latch1_low;

  pinMode(paintRedPin, OUTPUT);
  pinMode(paintGreenPin, OUTPUT);
  pinMode(paintBluePin, OUTPUT);

  digitalWrite(paintRedPin, HIGH); 
  digitalWrite(paintGreenPin, HIGH);
  digitalWrite(paintBluePin, HIGH);
  
  // clear initial state of display memory and set up stuff for animations
  //init stuff for animations
  clear_all(0);

  //map display memory pointer to display memory
  display_ptr0 = display_mem;
  display_ptr1 = display_mem + 8;
  display_ptr2 = display_mem + 16;
  display_ptr3 = display_mem + 24;

  // set button to be used as input to cycle though animation sequences
  pinMode(cycle_button, INPUT);

  // setup interrupt
  noInterrupts();// kill interrupts until everybody is set up

  // *************************************************************
  // Timer 1 will be used to refresh LED states
  // will use a clock divide of 64 to get down to 250kHz (4us period)
  // then count up 70 ticks to get 280us between refreshes (3.57kHz update )

  // COM1[A-C](1:0) = 00   no IO in use on interrupt
  // WGM1(3:0) = 0100 = CTC clear counter on counter match
  // CS1(2:0) = 011 = div by 64 on 16MHz clock
  // OCIE1A = 1 output compare interrupt enable 1 A set to enable interrupt

  TCCR1A = B00000000;      //Register A all 0's since we're not toggling IO pins
  TCCR1B = B00001011;      //bit 3 set to place in CTC mode, will call an interrupt on a counter match, bits 2:0 = 011 to get /64 clk divide
  // slow down for debug
  //TCCR1B = B00001101;    // *debug* bit 3 set to place in CTC mode, will call an interrupt on a counter match
  TIMSK1 = B00000010;      //bit 1 set to call the interrupt on an OCR1A match
  //TIMSK1 = B00000000;      //bit 1 set to call the interrupt on an OCR1A match

  OCR1AH = B00000000;      // timer 16 is 16 bit counter so compare reg has a high and low byte
  //slow down for debug
  //OCR1AH = B00000011;    // *debug* timer 16 is 16 bit counter so compare reg has a high and low byte
  OCR1AL = 50;
  //OCR1AL = 50;

  //our clock runs at 250kHz, which is 1/250kHz = 4us
  //with OCR1A set to 30, this means the interrupt will be called every (30+1)x4us=124us, 
  // which gives a multiplex frequency of about 8kHz



  // *************************************************************
  // Timer3 will be used for controller input polling and response
  // will use a clock divide of 1024 to get down to 15.6kHz (0.064ms period)
  // then count up 260 ticks on the 0.064ms clock to get us a 16.667ms ( roughtly 60/s interval ) to poll the controller inputs
  // can make this longer if we want to but 60/s sounds good

  // COM3[A-C](1:0} = 00
  // WGM3(3:0) = 0100 = CTC ( clear counter on match ) mode
  // CS3(2:0)  = 101 =  div by 1024 on 16MHz clock
  // OCIE3A = 1 = output compare interrupt enable 3 A set to enable interrupt


  TCCR3A = B00000000;  // all zeros, again, no IO on interrupt
  TCCR3B = B00001101;  // bit3 = 1 for CTC WGM mode,  bits 2:0 = 101 to get / 1024 clk divide
  TIMSK3 = B00000010;  // bit1 = 1 to enable interrupt on timer match
  //TIMSK3 = B00000000;  // bit1 = 1 to enable interrupt on timer match


  // 260 in bin = 100000100
  // 300bin = 00000001 00101100
  // 500bin = 00000010 00001000
  
  // 
  OCR3AH = B00000010;  // upper timer compare 3 bits
  OCR3AL = B00001000;  // lower timer compare 3 bits

  // lastly, enable shift register outputs and re-enable interrupts
  digitalWrite(blankPin, LOW);
  SPI.begin();        //start up the SPI library
  
  //interrupt_timer_start = millis();
  interrupts();       //let the show begin, this lets the multiplexing start
  
  // display welcome screen
  splash(0);
  
  // nes_paint is first animation, set up initial state for it
  for(int i=0; i<ledCount; i++) {
    if (i == 0){
      led_state[i] = 1;
    } else {
      display_mem[i] = 0;
      led_state[i] = 0;
    }
    // also set up previous_loc array initial state
    if ( i < 32 ) {
      previous_loc[2*i] = 0;
      previous_loc[(2*i)+1] = 0;
    }
  }

  if(debug == 1 ){
    Serial.begin(19600);
  }
  
  Serial.begin(19600);
  
} 


// the loop routine runs over and over again forever, allows user to modify led brightness w/ analog input
void loop()  { 

  // grab button state to see if we want to cycle to next animation in sequence
  buttonState = digitalRead(cycle_button);

  if (buttonState != lastbuttonState && ( millis() - buttonPreviousMillis > 600 )  ){  // button state has changed
    buttonchanged = 1;
    lastbuttonState = buttonState;
    buttonPreviousMillis = millis();      
  } 
  else {
    buttonchanged = 0;
  }
  
   // if button is pressed, cycle to next animation
  /*
  if (buttonState == HIGH && buttonchanged == 1 ) {
    //move to next animation
    next_animation();
  } 
  */
  
  // if cycle interval for animation has run it's course, scroll to next animation
  if ( millis() - animationCycleMillis > 15000 ) {
    next_animation();
    animationCycleMillis = millis();
  }
  
  // main loop,  if animation interval has expired, make another call to selected animation function
  if ( millis() - previousMillis > animation_interval ) {

    previousMillis = millis();
    
    // call animation function for array 
    switch (animation_number) {
      case 0:
        // "paint" function using NES controller
        nes_paint();
        break;
      case 1:
        // toggle each led in array to full brightness at random intervals for random amounts of time
        blink_rand_interval();
        break;
      case 2:
        // bounce
        bounce(1);
        break;
      case 3:
        // pong
        pong();
        break;
      case 4:
        // clear all ( random static color )
        clear_all(8);
        break;
      case 5:
        serial_control();
        break;
      case 6:
        wave();
        break;
    }

    // comment out below, used to time interrupt cycle for testing  
    /*if ( interrupt_counter < 5 ) {
        Serial.println(interrupt_total);
        interrupt_counter = 5000000;
      } */
      
  }

}

// **************************************************************************************************************************************
// ********************** animations stored here  *********************************

// #################################################################
// 0: move lit LED around based on NES control D pad buttons, left,right,up,down
void nes_paint(){
  
  // grab current NES control button state 
  byte up,down,left,right,a,b,sel,start = 0;   
  String paint_string = ""; 
    
  up = bitRead(nes_state0, 4);  
  down = bitRead(nes_state0, 5);
  left = bitRead(nes_state0, 6);
  right = bitRead(nes_state0, 7);
  a = bitRead(nes_state0, 0);
  b = bitRead(nes_state0, 1);
  sel = bitRead(nes_state0, 2);
  start = bitRead(nes_state0, 3);
  
  
  // start button used to clear array
  if ( start == 0 ) {
    
    for ( int i=0; i < ledCount; i++ ) {  
      if (display_mem[i] > 0) {
        paint_string = String("display_mem[");
        paint_string = String(paint_string + i + "] = " + display_mem[i] + ";" );
        Serial.println(paint_string);
      }
      display_mem[i] = 0;    
    }
  }
    
  // select button used to cycle though color choice w/ indicator, green, blue, clear
  
  if ( sel == 0 ) { 
  
    // toggle paint color between red, green, blue, white and clear
    if ( color < 4 ) {
      color++;
      nes_red = 0;
      nes_green = 0;
      nes_blue = 0;
    } else {
      color = 0;
      nes_red = 0;
      nes_green = 0;
      nes_blue = 0;
    }
    
    // illuminate infor LED based on which color to "paint" with  
    switch (color) {
      case 0:
        digitalWrite(paintRedPin, LOW);
        digitalWrite(paintGreenPin, HIGH);
        digitalWrite(paintBluePin, HIGH);
        break;
      case 1:
        digitalWrite(paintRedPin, HIGH);
        digitalWrite(paintGreenPin, LOW);
        digitalWrite(paintBluePin, HIGH);
        break;
      case 2:
        digitalWrite(paintRedPin, HIGH);
        digitalWrite(paintGreenPin, HIGH);
        digitalWrite(paintBluePin, LOW);
        break;
      case 3:
        digitalWrite(paintRedPin, LOW);
        digitalWrite(paintGreenPin, LOW);
        digitalWrite(paintBluePin, LOW);
        break;
      case 4:
        digitalWrite(paintRedPin, HIGH);
        digitalWrite(paintGreenPin, HIGH);
        digitalWrite(paintBluePin, HIGH);
        break;
    }
  
  }
  
  // first need to grab color state of cursor location, this is so I have the correct starting rgb point for raising and lowering each LED
  nes_red = display_mem[nes_location] / 64;
  nes_green = (display_mem[nes_location] % 64) / 8;
  nes_blue = display_mem[nes_location] % 8;
  
  // a button will paint selected LED up by current color
  if ( a == 0 ){
    // paint LED up based on currently selected color
    switch (color) {
      case 0:
        if(nes_red < 7 ){
          nes_red++;
          display_mem[nes_location] = display_mem[nes_location] + 64;
        }
        break;
      case 1: 
        if(nes_green < 7 ){
          nes_green++;
          display_mem[nes_location] = display_mem[nes_location] + 8;
        }
        break;
      case 2: 
        if(nes_blue < 7 ){
          nes_blue++;
          display_mem[nes_location] = display_mem[nes_location] + 1;
        }
        break;
      case 3:
        if(nes_red < 7 ){
          nes_red++;
          display_mem[nes_location] = display_mem[nes_location] + 64;
        }
        if(nes_green < 7 ){
          nes_green++;
          display_mem[nes_location] = display_mem[nes_location] + 8;
        }
        if(nes_blue < 7 ){
          nes_blue++;
          display_mem[nes_location] = display_mem[nes_location] + 1;
        }
        break;
      case 4:
        nes_red = 0;
        nes_green = 0;
        nes_blue = 0;
        display_mem[nes_location] = 0;
        break;
    }
  } 
  
  // b button will paint selected LED down by current color
  if ( b == 0 ) {
    // paint LED down based on currently selected color
    switch (color) {
      case 0:
        if(nes_red > 0 ){ 
          nes_red--;
          display_mem[nes_location] = display_mem[nes_location] - 64;
        }
        break;
      case 1: 
        if(nes_green > 0 ){
          nes_green--; 
          display_mem[nes_location] = display_mem[nes_location] - 8;
        }
        break;
      case 2: 
        if(nes_blue > 0 ){ 
          nes_blue--;
          display_mem[nes_location] = display_mem[nes_location] - 1;
        }
        break;
      case 3:
        if(nes_red > 0 ){ 
          nes_red--;
          display_mem[nes_location] = display_mem[nes_location] - 64;
        }
        if(nes_green > 0 ){
          nes_green--; 
          display_mem[nes_location] = display_mem[nes_location] - 8;
        }
        if(nes_blue > 0 ){ 
          nes_blue--;
          display_mem[nes_location] = display_mem[nes_location] - 1;
        }
        break; 
      case 4:
        nes_red = 0;
        nes_green = 0;
        nes_blue = 0;
        display_mem[nes_location] = 0;
        break;
    } 
  }
  
  // LED cursor position is being moved, calculate new position
  if( up == 0 ){
    // move led up if not in top row already, no wrap
    if (nes_location>31) {
      nes_location = nes_location - 32;
      cursor_blink(nes_location);
    }
  } else if ( down == 0 ) {
    // move led down if not in bottom row already, no wrap
    if (nes_location<480){
      nes_location = nes_location + 32;
      cursor_blink(nes_location);
    }
  } else if ( left == 0 ) {
    if (nes_location%32 > 0) {         // move left, no wrap
      nes_location = nes_location - 1;
      cursor_blink(nes_location);
    }
  } else if ( right == 0 ) {
    if (nes_location%32 < 31) {          // move right, no wrap
      nes_location = nes_location + 1;
      cursor_blink(nes_location);
   }
  }
  
  
  //debug info
  /*
  if (debug == 1) {
    Serial.println(nes_state0);
  }
  */
  
}


// #################################################################
// 1: animation to randomly turn on a LED for a random amount of time
void blink_rand_interval(){
  
  clear_all(0);
  for ( byte i = 0; i<60; i++ ) {
  
    //grab an led to update
    int led_to_update = random(0,ledCount);
  
    //if led is currently on, turn it off
    if( display_mem[led_to_update] > 0 ) {
      display_mem[led_to_update] = 0;      // turn LED off
    } else {
      display_mem[led_to_update] = random(0,512);
      //display_mem[led_to_update] = 511;
    }
  }
  
  
  /*  old wait interval code, don't need for large array since already slow 
  // if led is currently on, decrement it's blink counter and check if it should be shut off
  if (led_state[led_to_update] == 1 ) {
    blink_interval[led_to_update]--;
    
    if (blink_interval[led_to_update] <= 0 ) {
      // turn led off now, re-randomize blink time for next time though
      blink_interval[led_to_update] = random(blink_min, blink_max);
      display_mem[led_to_update] = 0;
      led_state[led_to_update] = 0;
    }
  
  // if led is currently off, decrement it's wait counter and check if it should be turned on
  } else {
    wait_interval[led_to_update]--;
    
    if (wait_interval[led_to_update] <=0) {
      // turn on led now, re-randomize wait time for next time through
      wait_interval[led_to_update] = random(wait_min, wait_max);
      
      // time to turn on, random color choice              
      display_mem[led_to_update] = (random(0,8)*64) + (random(0,8)*8) + random(0,8);
      led_state[led_to_update] = 1;
    }
  } */

}

// #################################################################
// 2: bouncing balls
void bounce(byte type) {
  
  byte flash_top = 0;
  byte flash_bottom = 0;
  byte flash_left = 0;
  byte flash_right = 0;
  int edge_flash_color = 7;
  byte left_edge = 0;
  byte right_edge = 31;
  byte top = 0;
  byte bottom = 15;
  
  //if type is 1, we're using bigger balls, adjust edges accordingly
  if ( type == 1 ) {
    left_edge = 1;
    right_edge = 29;
    top = 2;
    bottom = 14;
  }
  
  // fade down previous led position when ball count <=5
  if ( balls <= 5 && type == 0 ) {
    for( int j=0; j<ledCount; j++) {
      if ( display_mem[j] > 0 ) {
        //decrement red
        if ( display_mem[j] / 64  > 0 ) display_mem[j] = display_mem[j] - 64;
    
        //decrement green
        if ( (display_mem[j] % 64) / 8 > 0 ) display_mem[j] = display_mem[j] - 8;
    
        //decrement blue
        if ( display_mem[j] % 8 > 0 ) display_mem[j]--;
      }
    }  
  }
  
  for (int i=0; i<balls; i++ ) {
    
     //clear last frame ( no fade ) for ball count > 2
     if (balls > 5 || type > 0 ) {
       display_mem[location_x[i]+(32*location_y[i])] = 0;
       
       if ( type == 1 ) {
         int ball_loc = location_x[i]+(32*location_y[i]);
         display_mem[ball_loc-1] = 0;
         display_mem[ball_loc+1] = 0;
         display_mem[ball_loc-33] = 0;
         display_mem[ball_loc-32] = 0;
         display_mem[ball_loc-31] = 0;
         
         display_mem[ball_loc+1] = 0;
         display_mem[ball_loc+2] = 0;
         display_mem[ball_loc+32] = 0;
         display_mem[ball_loc+33] = 0;
         display_mem[ball_loc-31] = 0;
         display_mem[ball_loc-30] = 0;
         display_mem[ball_loc-64] = 0;
         display_mem[ball_loc-63] = 0;
         
       }
     } 

        
    // calculate new X position if interval counter is multiple of ball's speed rating ( high speed rating means lower update interval in this case )
    if ( interval_counter % speed_x[i] == 0 ) {
      
      if (location_x[i] == right_edge ) {    // at right edge, change direction left
        location_x[i]--;
        direction_x[i] = 0;
        flash_right = 1;
      } else if ( location_x[i] == left_edge ) {     // at left edge, change direction right
        location_x[i]++;
        direction_x[i] = 1;
        flash_left = 1;
      } else if ( location_x[i] < right_edge && direction_x[i] == 1) {
        location_x[i]++;
      }  else if ( location_x[i] > left_edge && direction_x[i] == 0 ) {
        location_x[i]--;
      }
    }
  
    // calculate new Y position, same deal with the speed_y setting
    if ( interval_counter % speed_y[i] == 0 ) {
      if( location_y[i] < bottom && direction_y[i] == 1) {
        location_y[i]++;
      }  else if ( location_y[i] > top && direction_y[i] == 0 ) {
        location_y[i]--;
      } else if ( location_y[i] == bottom ) {    // at bottom, change direction up
        location_y[i]--;
        direction_y[i] = 0;
        flash_bottom = 1;
      } else if ( location_y[i] == top  ) {    // at top, change direction down
        location_y[i]++;
        direction_y[i] = 1;
        flash_top = 1;
      }
    }
    
    
    // flash whole edge of matrix when ball strikes it,  can tweak to only have portion of edge near ball flash when ball strikes it ( use location_x/y and flash adjacent leds
    /*
    for( byte i=0; i<32; i++) {
      if (flash_top == 1) {
        display_mem[i] = edge_flash_color;
      } else {
        display_mem[i] = 0;
      }
      
      if (flash_bottom == 1) {
        display_mem[i+480] = edge_flash_color;
      } else {
        display_mem[i+480] = 0;
      }
      
      // check left/right edges also
      if ( i < 16 ) {
        
        if ( flash_left == 1 ) {
          display_mem[i*32] = edge_flash_color;
        } else {
          display_mem[i*32] = 0;
        }
        
        if ( flash_right == 1 ) {
          display_mem[(i*32) + 31] = edge_flash_color;
        } else {
          display_mem[(i*32) + 31] = 0;
        }
        
      } else {
        // do nothing
      }
    }
    */
    
    if ( type == 1) {
      //draw 12-pixel ball based on current location
      int ball_loc = location_x[i]+(32*location_y[i]);
      //draw "center" of ball
      /*
      display_mem[ball_loc] = ball_color[i];
      display_mem[ball_loc+1] = ball_color[i];
      display_mem[ball_loc-32] = ball_color[i];
      display_mem[ball_loc-31] = ball_color[i];
      */
      
      //draw out edge of ball
      display_mem[ball_loc-1] = ball_color[i];
      display_mem[ball_loc-33] = ball_color[i];
      display_mem[ball_loc+2] = ball_color[i];
      display_mem[ball_loc+32] = ball_color[i];
      display_mem[ball_loc+33] = ball_color[i];
      display_mem[ball_loc-30] = ball_color[i];
      display_mem[ball_loc-64] = ball_color[i];
      display_mem[ball_loc-63] = ball_color[i];
        
           
    } else {  
      display_mem[location_x[i]+(32*location_y[i])] = ball_color[i];
    }
   
  }
  
   // increment interval counter, used to determine whether or not to update an led's position based on it's speed setting
  if ( interval_counter < 16 ) {
    interval_counter++;
  } else {
    interval_counter = 0;
  }
     
}


// #################################################################
// 3: pong..  start w/ simply moving paddle around

void pong() {
  
  // grab current NES control button state 
  byte left,right, flash, start, select = 0;
  
  left = bitRead(nes_state0, 6);
  right = bitRead(nes_state0, 7);
  start = bitRead(nes_state0, 3);
  select = bitRead(nes_state0, 2);
  
  //re-init game when user presses start
  if ( start == 0 ) {
    clear_all(0);
    nes_location = 494;   // starting paddle location
    direction_x[0] = random(1,2);
    direction_y[0] = random(1,2);
    location_x[0] = random(0,32);
    location_y[0] = random(0, 3);
    speed_x[0] = B00000001 << random(0,3);
    speed_y[0] = speed_x[0];
    //speed_y[0] = B00000001 << random(0,3);
    //ball_color[i] = (7*random(0,2)) + (56*random(0,2))+(448*random(0,2)); 
    ball_color[0] = random(0, 512);
    interval_counter = 0;
    flash = 0;
    score = 0;
  }
  
  //change paddle width when user presses select  ( make this a level thing later )
  if ( select == 0 ) {
  
    if (paddle_width < 8 ) {
      paddle_width++;
    } else {
      paddle_width = 2;
    }
  }
  
  
  // check direction buttons and move paddle to new location
  if ( left == 0 ) {
    if (nes_location > 480 && nes_location > 479) {         // move left
      nes_location = nes_location - 1;
    }
  } else if ( right == 0 ) {
    if (nes_location < (512 - paddle_width) ) {             // move right
      nes_location = nes_location + 1;
    }
  }
  
  // calculate new position for ball
  
  //clear last frame
  display_mem[location_x[0]+(32*location_y[0])] = 0;
  
  // calculate new X position if interval counter is multiple of ball's speed rating ( high speed rating means lower update interval in this case )
  if ( interval_counter % speed_x[0] == 0 ) {
    if (location_x[0] == 31 ) {    // at right edge, change direction left
      location_x[0]--;
      direction_x[0] = 0;
    } else if ( location_x[0] == 0 ) {     // at left edge, change direction right
      location_x[0]++;
      direction_x[0] = 1;
    } else if ( location_x[0] < 31 && direction_x[0] == 1) {
      location_x[0]++;
    }  else if ( location_x[0] > 0 && direction_x[0] == 0 ) {
      location_x[0]--;
    }
  }
  
  // calculate new Y position, same deal with the speed_y setting, y position only "reflects" from bottom if ball strikes paddle
  if ( interval_counter % speed_y[0] == 0 ) {
    if( location_y[0] < 15 && direction_y[0] == 1) {
      location_y[0]++;
    }  else if ( location_y[0] > 0 && direction_y[0] == 0 ) {
      location_y[0]--;
    } else if ( location_y[0] == 15 ) {    // at bottom, change direction up if x location is within paddle range
      
      // check that ball location is on paddle
      if ( (location_x[0]+480) >= nes_location && (location_x[0]+480) < (nes_location+paddle_width) ) {
        location_y[0]--;
        direction_y[0] = 0;
        score++;
      } else {
        location_y[0] = 16;
        flash = 1;
        if (debug == 1) { 
          Serial.print(location_x[0]);
          Serial.print("  ");
          Serial.println(nes_location);
        }
      }
    } else if ( location_y[0] == 0  ) {    // at top, change direction down
      location_y[0]++;
      direction_y[0] = 1;
    }
  }
   
  // draw ball
  display_mem[location_x[0]+(32*location_y[0])] = ball_color[0];
    
  // show score at top if ball escaped
  if (flash == 1) {
    for ( byte i=0; i<score; i++ ) {
      display_mem[i] = 56;
    }
    display_mem[location_x[0]] = 448;
  }
   
  // draw paddle now
  draw_paddle();
  
   // lastly increment interval counter, used to determine whether or not to update an led's position based on it's speed setting
  if ( interval_counter < 16 ) {
    interval_counter++;
  } else {
    interval_counter = 0;
  }
     
}


// #################################################################
// 4: wave animation
void wave() {
  
  float x = angle;  // this is the current "base" angle to calucate the sin off of
  int new_loc0;      // value basically stores the height of the wave
  int new_loc1;      // value basically stores the height of the wave
  
  // loop though each column in the matrix, compute the wave height and draw it
  for (int i = 0; i < 32; i++) {
    new_loc0 = 8 + (sin(x)*8);
    new_loc1 = 8 + (sin(-1*x)*8);
    //new_loc1 = -1 * new_loc0;
    x+=dx;
    display_mem[previous_loc[2*i]] = 0;
    display_mem[previous_loc[(2*i)+1]] = 0;
    
    //display_mem[32*new_loc + i] = random(0, 512);
    display_mem[32*new_loc0 + i] = 56;
    display_mem[32*new_loc1 + i] = 7;

    previous_loc[2*i] = (32*new_loc0) + i;
    previous_loc[(2*i)+1] = (32*new_loc1) + i;
  }
  
  //int new_loc = 8 + (7*sin(angle));
  angle += 0.43;
}

// #################################################################
// 5: manual control over each led brightness via serial communication from pc
void serial_control(){
  
  char serial_data = 0;
  
  // if there is serial data in the buffer, grab it and interpret
  while(Serial.available() > 0 ) {
    display_mem[511] = 0;
    serial_data = Serial.read();  //returns input as ascii character
   
    //corresponds to left mouse click
    if ( serial_data == 'A' ) {  // since serial port read byte as ascii, compare to ascii
      ball_color[0] = 7;
      ball_color[1] = 448;
      clear_all(0);
     
    // right mouse click    
    } else if (serial_data == 'B') {
      ball_color[0] = 448;
      ball_color[1] = 7;
      clear_all(0);
  
    // mouse position string
    } else if (serial_data == 'P' ) {
    
      int pos = 600;
       
      // grab position integer and update display based on current mouse position   
      while (Serial.available() > 0) {
        pos = Serial.parseInt();
        
        if (pos < 512) {
          display_mem[previous_pos] = 0;     // clear previous location
               
          // draw x and y cursors across whole screen with single led in center for position     
          for (int i=0; i<32; i++) {
            display_mem[(32*(previous_pos/32))+i] = 0;
            
            if (i < 16 ) {
              display_mem[(32*i)+(previous_pos%32)] = 0;
              display_mem[(32*i)+(pos%32)] = ball_color[1];
            }
            display_mem[(32*(pos/32)) + i ] = ball_color[1];
          }
         
          // now set current location color and save location as previous location
          display_mem[pos] = ball_color[0];
          previous_pos = pos;  
        }
      }
    }
  }

  display_mem[511] = 43;  // for serial communication debug

}

// **************************************************************************************************************************************
// *************************************  INTERRUPTS HERE **********************************************

// interrupt routune to control LED state on timer interrupt
ISR(TIMER1_COMPA_vect) {
  
  // comment out below, used to time interrupt cycle for testing
  //interrupt_counter--;
  //interrupt_timer_start = micros();
  
  unsigned int pixel_shade0[8];  //temporary storeage for shade comparisons
  unsigned int pixel_shade1[8];
  unsigned int pixel_shade2[8];
  unsigned int pixel_shade3[8];
  
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
  

  //instead of all that BAM counter stuff, use lookup table to determine rgb led brightness

  //assemble rgb shift bytes for each row / grayscale
  for ( int i=0; i<8; i++ ) {

    //grab pixel color for current led in row
    pixel_shade0[i] = *display_ptr0;
    pixel_shade1[i] = *display_ptr1;
    pixel_shade2[i] = *display_ptr2;
    pixel_shade3[i] = *display_ptr3;

    // constrain value to 0:511 for functions where we are fading down and may go negative
    if (animation_number == 2 ) {
    
      if (pixel_shade0[i] < 0 )  { 
        pixel_shade0[i] = 0; 
      }
      if (pixel_shade1[i] < 0 )  { 
        pixel_shade1[i] = 0; 
      }
      if (pixel_shade2[i] < 0 )  { 
        pixel_shade1[i] = 0; 
      }
      if (pixel_shade3[i] < 0 )  { 
        pixel_shade1[i] = 0; 
      }
    }

    //lookup shade RGB values from flash lookup table, store to shift_byte to be send to led driver
    
    // col0:7
    if ( pgm_read_byte_near( &lookup_red[pixel_shade0[i]] )   > shade_compare ) { 
//    if ( (pixel_shade0[i] >> 6 ) > shade_compare ) {
      bitWrite(shift_byte_r0, i, 1); 
    }
    if ( pgm_read_byte_near( &lookup_green[pixel_shade0[i]] ) > shade_compare ) { 
      bitWrite(shift_byte_g0, i, 1); 
    }
    if ( pgm_read_byte_near( &lookup_blue[pixel_shade0[i]] )  > shade_compare ) {
//    if ( (pixel_shade0[i] & 7) > shade_compare ) { 
      bitWrite(shift_byte_b0, i, 1); 
    }
    
    // col8:15
    if ( pgm_read_byte_near( &lookup_red[pixel_shade1[i]] )   > shade_compare ) { 
//    if ( (pixel_shade1[i] >> 6 ) > shade_compare ) {  
      bitWrite(shift_byte_r1, i, 1); 
    }
    if ( pgm_read_byte_near( &lookup_green[pixel_shade1[i]] ) > shade_compare ) { 
      bitWrite(shift_byte_g1, i, 1); 
    }
    if ( pgm_read_byte_near( &lookup_blue[pixel_shade1[i]] )  > shade_compare ) { 
//    if ( (pixel_shade1[i] & 7) > shade_compare ) {  
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
  

  // now send rgb bytes for current row out to shift regs, send out data for column 16:23 first, followed by cols 8:15 / rows 8:15 first, followed by 0:7 since driver boards are daisy-chained
  SPI.transfer(shift_byte_r3);
  SPI.transfer(shift_byte_g3);
  SPI.transfer(shift_byte_b3);
    
  SPI.transfer(shift_byte_r2);
  SPI.transfer(shift_byte_g2);
  SPI.transfer(shift_byte_b2);

  SPI.transfer(shift_byte_r1);
  SPI.transfer(shift_byte_g1);
  SPI.transfer(shift_byte_b1);
  SPI.transfer(row_cntl1[row_count]);
  
  SPI.transfer(shift_byte_r0);
  SPI.transfer(shift_byte_g0);
  SPI.transfer(shift_byte_b0);
  SPI.transfer(row_cntl0[row_count]);

  //once done, toggle latch pin to capture data to output of shift reg
  latch_high;
  //delayMicroseconds(100);
  latch_low;
  //delayMicroseconds(5);  // wait a bit before moving pointers

  // turn on shift reg outputs
  blank_low;
   
  // update row count so next interrupt, we refresh next row, wrap at top to bottom
  // once we complete all rows, incr shade_compare to update for next brightness level
  // once we complete all shades, start whole loop over again

  if( row_count == rows-1 ) {
    //delayMicroseconds(100);  // wait a bit before moving pointers
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
ISR(TIMER3_COMPA_vect) {

  // latch button state into shift reg
  NES_latch0_high;
  delayMicroseconds(50);
  NES_latch0_low;


  // shift data input from NES controllers
  for( int i=0; i<8; i++) {
    bitWrite(nes_state0, i, digitalRead(NES_data0)); 
    NES_clk0_high;
    NES_clk0_low;

    //bitWrite(nes_state1, i, digitalRead(NES_data1)); 
    //digitalWrite(NES_clk1, HIGH);
    //digitalWrite(NES_clk1, LOW);
  }


}



// **************************************************************************************************************************************
// **********************************    helpers     ******************************************

// set brightness of all LEDs to zero
void clear_all(int color) {
  
  int test_color = 6;
  
  switch (color) {
    case 512:
      for(int i=0; i<ledCount; i++) {
        display_mem[i] = 0;
      }
      // r/g/b/w column pattern for trim resistor tuning
     
      display_mem[0] = test_color;
      display_mem[7] = test_color;
      display_mem[8] = test_color;
      display_mem[15] = test_color;
      display_mem[16] = test_color;
      display_mem[23] = test_color;
      display_mem[24] = test_color;
      display_mem[31] =  test_color;
      display_mem[32] =  test_color;
      display_mem[39] =  test_color;
      display_mem[40] =  test_color;
      display_mem[47] =  test_color;
      display_mem[48] =  test_color;
      display_mem[55] =  test_color;
      display_mem[56] =  test_color;
      display_mem[63] =  test_color;
      display_mem[64] =  test_color;
      display_mem[71] =  test_color;
      display_mem[72] =  test_color;
      display_mem[79] =  test_color;
      display_mem[80] =  test_color;
      display_mem[87] =  test_color;
      display_mem[88] =  test_color;
      display_mem[95] =  test_color;
      display_mem[96] =  test_color;
      display_mem[103] =  test_color;
      display_mem[104] =  test_color;
      display_mem[111] =  test_color;
      display_mem[112] =  test_color;
      display_mem[119] =  test_color;
      display_mem[120] =  test_color;
      display_mem[127] =  test_color;
      display_mem[128] =  test_color;
      display_mem[135] =  test_color;
      display_mem[136] =  test_color;
      display_mem[143] =  test_color;
      display_mem[144] =  test_color;
      display_mem[151] =  test_color;
      display_mem[152] =  test_color;
      display_mem[159] =  test_color;
      display_mem[160] =  test_color;
      display_mem[167] =  test_color;
      display_mem[168] =  test_color;
      display_mem[175] =  test_color;
      display_mem[176] =  test_color;
      display_mem[183] =  test_color;
      display_mem[184] =  test_color;
      display_mem[191] =  test_color;
      display_mem[192] =  test_color;
      display_mem[199] =  test_color;
      display_mem[200] =  test_color;
      display_mem[207] =  test_color;
      display_mem[208] =  test_color;
      display_mem[215] =  test_color;
      display_mem[216] =  test_color;
      display_mem[223] =  test_color;
      display_mem[224] =  test_color;
      display_mem[231] =  test_color;
      display_mem[232] =  test_color;
      display_mem[239] =  test_color;
      display_mem[240] =  test_color;
      display_mem[247] =  test_color;
      display_mem[248] =  test_color;
      display_mem[255] =  test_color;
      display_mem[256] =  test_color;
      display_mem[263] =  test_color;
      display_mem[264] =  test_color;
      display_mem[271] =  test_color;
      display_mem[272] =  test_color;
      display_mem[279] =  test_color;
      display_mem[280] =  test_color;
      display_mem[287] =  test_color;
      display_mem[288] =  test_color;
      display_mem[295] =  test_color;
      display_mem[296] =  test_color;
      display_mem[303] =  test_color;
      display_mem[304] =  test_color;
      display_mem[311] =  test_color;
      display_mem[312] =  test_color;
      display_mem[319] =  test_color;
      display_mem[320] =  test_color;
      display_mem[327] =  test_color;
      display_mem[328] =  test_color;
      display_mem[335] =  test_color;
      display_mem[336] =  test_color;
      display_mem[343] =  test_color;
      display_mem[344] =  test_color;
      display_mem[351] =  test_color;
      display_mem[352] =  test_color;
      display_mem[359] =  test_color;
      display_mem[360] =  test_color;
      display_mem[367] =  test_color;
      display_mem[368] =  test_color;
      display_mem[375] =  test_color;
      display_mem[376] =  test_color;
      display_mem[383] =  test_color;
      display_mem[384] =  test_color;
      display_mem[391] =  test_color;
      display_mem[392] =  test_color;
      display_mem[399] =  test_color;
      display_mem[400] =  test_color;
      display_mem[407] =  test_color;
      display_mem[408] =  test_color;
      display_mem[415] =  test_color;
      display_mem[416] =  test_color;
      display_mem[423] =  test_color;
      display_mem[424] =  test_color;
      display_mem[431] =  test_color;
      display_mem[432] =  test_color;
      display_mem[439] =  test_color;
      display_mem[440] =  test_color;
      display_mem[447] =  test_color;
      display_mem[448] =  test_color;
      display_mem[455] =  test_color;
      display_mem[456] =  test_color;
      display_mem[463] =  test_color;
      display_mem[464] =  test_color;
      display_mem[471] =  test_color;
      display_mem[472] =  test_color;
      display_mem[479] =  test_color;
      display_mem[480] =  test_color;
      display_mem[487] =  test_color;
      display_mem[488] =  test_color;
      display_mem[495] =  test_color;
      display_mem[496] =  test_color;
      display_mem[503] =  test_color;
      display_mem[504] =  test_color;
      display_mem[511] =  test_color;         
      break;
      
    case 513:      
      // randomize colors for matrix
      for(int i=0; i<ledCount; i++) {
        display_mem[i] = random(0, 512);
      }
      break;
      
    case 514:
      // color = array index
      for(int i=0; i<ledCount; i++) {
        display_mem[i] = i;
      }
      break; 
     
    case 515:
      //rgb checkerboard
      for(int i=0; i<ledCount; i++ ) {
        if (i%3 == 0 ){
          display_mem[i] = 448;
        } else if ( i%3 == 1 ) {
          display_mem[i] = 56;
        } else if (i%3 ==2) {
          display_mem[i] = 7;
        }
      }
      break; 
    
    default:
      // set all LEDs to color passed to function
      
      //first constrain color to 0:511 range
      color = constrain(color, 0, 511);
      
      for(int i=0; i<ledCount; i++) {
        display_mem[i] = color;
      }
    }
}


// blink a selected location white for .1 seconds
void cursor_blink(int location) {

  // save off currrent state so we can load it back when done
  int save_state = display_mem[location];

  display_mem[location] = 511;
  delay(50);
  display_mem[location] = 0;
  delay(50);
  display_mem[location] = save_state;

}

// set up paramaters for next animation and move animation number
void next_animation() {
  
  if (animation_number == 0 ) {
   
    // in nes_paint
    // need to set up led state before swapping over to blink animation
    clear_all(0);
    TIMSK3 = B00000000;
    animation_interval = 10;
    animation_number = 1;
  
  } else if (animation_number == 1) {
  
    // in rand blink
    // set up led state for bounce
    clear_all(0);
    //randomSeed(analogRead(0));
    for ( byte i = 0; i<balls; i++ ) {
      direction_x[i] = random(0,2);
      direction_y[i] = random(0,2);
      location_x[i] = random(1,30);
      location_y[i] = random(2, 15);
      speed_x[i] = B00000001 << random(0,3);
      //speed_y[i] = speed_x[i];
      speed_y[i] = B00000001 << random(0,3);
      //ball_color[i] = (7*random(0,2)) + (56*random(0,2))+(448*random(0,2)); 
      ball_color[i] = random(0, 512);
      /*
      if( i%2 == 1 ) { 
        ball_color[i] = 7;
      } else {
        ball_color[i] = 448;
      }
      */
    }
    TIMSK3 = B00000010;
    animation_interval = 30;
    animation_number = 2;
    
  } else if (animation_number == 2 ) {
  
    // in bounce, set up led state for pong
    clear_all(0);
    TIMSK3 = B00000010;
    nes_location = 494;   // starting paddle location
    direction_x[0] = random(1,2);
    direction_y[0] = random(1,2);
    location_x[0] = random(0,32);
    location_y[0] = random(0, 3);
    speed_x[0] = B00000001 << random(0,3);
    speed_y[0] = speed_x[0];
    //speed_y[i] = B00000001 << random(0,3);
    //ball_color[i] = (7*random(0,2)) + (56*random(0,2))+(448*random(0,2)); 
    ball_color[0] = random(0, 512);
    interval_counter = 0;
    animation_interval = 25;
    animation_number = 3;
    
  } else if ( animation_number == 3) {
    
    // in pong, set up for static
    
    TIMSK3 = B00000000;
    clear_all(0);
    animation_interval = 500;
    animation_number = 4;
    
    
    //set up led state for nes_paint
    /*
    clear_all(0);
    nes_location = 0;
    color = 0;
    TIMSK3 = B00000010;
    animation_interval = 100;
    animation_number = 0;
    */
    
  } else if (animation_number == 4 ) {
  
    // in static test set up led state for serial control
    clear_all(0);
    nes_location = 0;
    color = 0;
    TIMSK3 = B00000000;
    animation_interval = 10;
    animation_number = 5;
  
  } else if (animation_number == 5 ) {
  
    // in serial control set up led state wave
    clear_all(0);
    nes_location = 0;
    color = 0;
    angle = 0;
    TIMSK3 = B00000000;
    animation_interval = 10;
    animation_number = 6;
  
  } else if (animation_number == 6 ) {
  
    // in wave set up led state for nes_paint
    clear_all(0);
    nes_location = 0;
    color = 0;
    TIMSK3 = B00000010;
    animation_interval = 100;
    animation_number = 0;
  
  }
    
}

// draw paddle for pong
void draw_paddle() {
  
  int paddle_color = 448;
  
  //clear bottom row and draw current paddle location
  for( int i=480; i<512; i++) {
    
    if ( i >= nes_location && i < ( nes_location + paddle_width ) ) {
      display_mem[i] = paddle_color;
    } else {
      display_mem[i] = 0;
    }
    
  }
}


// display hello message at startup/reset
void splash(byte screen) {
  
  unsigned long splash_start = millis();
  int splash_color = 63;
  int black = 0;
  int pink = 64+7;
  int yellow = (64*6) + (8*2);
  int red = 64*4;
  int white = 511;
  
  switch (screen) {
    
    case 0:
    
    // print "hello!!" to screen
    display_mem[66] = splash_color;
    display_mem[77] = splash_color;
    display_mem[80] = splash_color;
    display_mem[90] = splash_color;
    display_mem[92] = splash_color;
    display_mem[98] = splash_color;
    display_mem[109] = splash_color;
    display_mem[112] = splash_color;
    display_mem[122] = splash_color;
    display_mem[124] = splash_color;
    display_mem[130] = splash_color;
    display_mem[141] = splash_color;
    display_mem[144] = splash_color;
    display_mem[154] = splash_color;
    display_mem[156] = splash_color;
    display_mem[162] = splash_color;
    display_mem[173] = splash_color;
    display_mem[176] = splash_color;
    display_mem[186] = splash_color;
    display_mem[188] = splash_color;
    display_mem[194] = splash_color;
    display_mem[195] = splash_color;
    display_mem[196] = splash_color;
    display_mem[200] = splash_color;
    display_mem[201] = splash_color;
    display_mem[202] = splash_color;
    display_mem[205] = splash_color;
    display_mem[208] = splash_color;
    display_mem[212] = splash_color;
    display_mem[213] = splash_color;
    display_mem[214] = splash_color;
    display_mem[218] = splash_color;
    display_mem[220] = splash_color;
    display_mem[226] = splash_color;
    display_mem[229] = splash_color;
    display_mem[231] = splash_color;
    display_mem[235] = splash_color;
    display_mem[237] = splash_color;
    display_mem[240] = splash_color;
    display_mem[243] = splash_color;
    display_mem[247] = splash_color;
    display_mem[250] = splash_color;
    display_mem[252] = splash_color;
    display_mem[258] = splash_color;
    display_mem[261] = splash_color;
    display_mem[263] = splash_color;
    display_mem[264] = splash_color;
    display_mem[265] = splash_color;
    display_mem[266] = splash_color;
    display_mem[269] = splash_color;
    display_mem[272] = splash_color;
    display_mem[275] = splash_color;
    display_mem[279] = splash_color;
    display_mem[282] = splash_color;
    display_mem[284] = splash_color;
    display_mem[290] = splash_color;
    display_mem[293] = splash_color;
    display_mem[295] = splash_color;
    display_mem[301] = splash_color;
    display_mem[304] = splash_color;
    display_mem[307] = splash_color;
    display_mem[311] = splash_color;
    display_mem[314] = splash_color;
    display_mem[316] = splash_color;
    display_mem[322] = splash_color;
    display_mem[325] = splash_color;
    display_mem[327] = splash_color;
    display_mem[331] = splash_color;
    display_mem[333] = splash_color;
    display_mem[336] = splash_color;
    display_mem[339] = splash_color;
    display_mem[343] = splash_color;
    display_mem[354] = splash_color;
    display_mem[357] = splash_color;
    display_mem[360] = splash_color;
    display_mem[361] = splash_color;
    display_mem[362] = splash_color;
    display_mem[366] = splash_color;
    display_mem[369] = splash_color;
    display_mem[372] = splash_color;
    display_mem[373] = splash_color;
    display_mem[374] = splash_color;
    display_mem[378] = splash_color;
    display_mem[380] = splash_color;
    break;
    
  case 1:
     // display animation name for paint
     display_mem[357] = splash_color;
     break;
     
  case 2:
     // cake!
     //outline first ( black )
    display_mem[11] = black;
    display_mem[12] = black;
    display_mem[13] = black;
    display_mem[14] = black;
    display_mem[42] = black;
    display_mem[43] = black;
    display_mem[46] = black;
    display_mem[47] = black;
    display_mem[74] = black;
    display_mem[79] = black;
    display_mem[105] = black;
    display_mem[106] = black;
    display_mem[111] = black;
    display_mem[112] = black;
    display_mem[113] = black;
    display_mem[136] = black;
    display_mem[137] = black;
    display_mem[139] = pink;
    display_mem[142] = pink;
    display_mem[145] = black;
    display_mem[146] = black;
    display_mem[167] = black;
    display_mem[168] = black;
    display_mem[172] = pink;
    display_mem[173] = pink;
    display_mem[178] = black;
    display_mem[179] = black;
    display_mem[198] = black;
    display_mem[199] = black;
    display_mem[211] = black;
    display_mem[212] = black;
    display_mem[230] = black;
    display_mem[244] = black;
    display_mem[245] = black;
    display_mem[262] = black;
    display_mem[263] = white;
    display_mem[277] = black;
    display_mem[294] = black;
    display_mem[296] = white;
    display_mem[298] = white;
    display_mem[299] = white;
    display_mem[310] = black;
    display_mem[326] = black;
    display_mem[329] = white;
    display_mem[332] = white;
    display_mem[333] = white;
    display_mem[334] = white;
    display_mem[343] = black;
    display_mem[358] = black;
    display_mem[359] = black;
    display_mem[367] = white;
    display_mem[368] = white;
    display_mem[370] = white;
    display_mem[375] = black;
    display_mem[391] = black;
    display_mem[392] = black;
    display_mem[393] = black;
    display_mem[401] = white;
    display_mem[403] = white;
    display_mem[404] = white;
    display_mem[405] = white;
    display_mem[407] = black;
    display_mem[425] = black;
    display_mem[426] = black;
    display_mem[427] = black;
    display_mem[428] = black;
    display_mem[429] = black;
    display_mem[438] = white;
    display_mem[439] = black;
    display_mem[461] = black;
    display_mem[462] = black;
    display_mem[463] = black;
    display_mem[464] = black;
    display_mem[465] = black;
    display_mem[471] = black;
    display_mem[497] = black;
    display_mem[498] = black;
    display_mem[499] = black;
    display_mem[500] = black;
    display_mem[501] = black;
    display_mem[502] = black;
    
    //filling ( pink )
    display_mem[138] = pink;
    display_mem[143] = pink;
    display_mem[144] = pink;
    display_mem[169] = pink;
    display_mem[170] = pink;
    display_mem[171] = pink;
    display_mem[174] = pink;
    display_mem[176] = pink;
    display_mem[177] = pink;
    display_mem[200] = pink;
    display_mem[201] = pink;
    display_mem[202] = pink;
    display_mem[203] = pink;
    display_mem[204] = pink;
    display_mem[205] = pink;
    display_mem[206] = pink;
    display_mem[207] = pink;
    display_mem[209] = pink;
    display_mem[210] = pink;
    display_mem[231] = pink;
    display_mem[232] = pink;
    display_mem[233] = pink;
    display_mem[234] = pink;
    display_mem[235] = pink;
    display_mem[237] = pink;
    display_mem[238] = pink;
    display_mem[239] = pink;
    display_mem[241] = pink;
    display_mem[242] = pink;
    display_mem[243] = pink;
    display_mem[264] = pink;
    display_mem[265] = pink;
    display_mem[266] = pink;
    display_mem[267] = pink;
    display_mem[268] = pink;
    display_mem[269] = pink;
    display_mem[270] = pink;
    display_mem[271] = pink;
    display_mem[272] = pink;
    display_mem[273] = pink;
    display_mem[274] = pink;
    display_mem[275] = pink;
    display_mem[276] = pink;
    display_mem[297] = pink;
    display_mem[300] = pink;
    display_mem[301] = pink;
    display_mem[302] = pink;
    display_mem[303] = pink;
    display_mem[304] = pink;
    display_mem[305] = pink;
    display_mem[306] = pink;
    display_mem[307] = pink;
    display_mem[308] = pink;
    display_mem[309] = pink;
    display_mem[335] = pink;
    display_mem[336] = pink;
    display_mem[337] = pink;
    display_mem[338] = pink;
    display_mem[339] = pink;
    display_mem[340] = pink;
    display_mem[341] = pink;
    display_mem[342] = pink;
    display_mem[369] = pink;
    display_mem[371] = pink;
    display_mem[372] = pink;
    display_mem[373] = pink;
    display_mem[374] = pink;
    display_mem[406] = pink;
    
    //yellow
    display_mem[295] = yellow;
    display_mem[327] = yellow;
    display_mem[328] = yellow;
    display_mem[330] = yellow;
    display_mem[331] = yellow;
    display_mem[360] = yellow;
    display_mem[361] = yellow;
    display_mem[362] = yellow;
    display_mem[363] = yellow;
    display_mem[364] = yellow;
    display_mem[365] = yellow;
    display_mem[366] = yellow;
    display_mem[394] = yellow;
    display_mem[395] = yellow;
    display_mem[396] = yellow;
    display_mem[397] = yellow;
    display_mem[398] = yellow;
    display_mem[399] = yellow;
    display_mem[400] = yellow;
    display_mem[402] = yellow;
    display_mem[430] = yellow;
    display_mem[431] = yellow;
    display_mem[432] = yellow;
    display_mem[433] = yellow;
    display_mem[434] = yellow;
    display_mem[435] = yellow;
    display_mem[436] = yellow;
    display_mem[437] = yellow;
    display_mem[466] = yellow;
    display_mem[467] = yellow;
    display_mem[468] = yellow;
    display_mem[469] = yellow;
    display_mem[470] = yellow;
    
    //red
    display_mem[44] = red;
    display_mem[45] = red;
    display_mem[75] = red;
    display_mem[77] = red;
    display_mem[78] = red;
    display_mem[107] = red;
    display_mem[109] = red;
    display_mem[110] = red;
    display_mem[140] = red;
    display_mem[141] = red;
    
    //white
    display_mem[76] = white;
    display_mem[108] = white;
    display_mem[175] = white;
    display_mem[208] = white;
    display_mem[236] = white;
    display_mem[240] = white;
       
    break;
      
    
  }

  
  while( millis() - splash_start < 3000 ) {
    // do nothing, just wait
  }
 
}

/*  

 digits ( store in prg mem at some point to retrieve and them move to desired location )
 
display_mem[0] = 7;
display_mem[1] = 7;
display_mem[5] = 7;
display_mem[6] = 7;
display_mem[7] = 7;
display_mem[11] = 7;
display_mem[12] = 7;
display_mem[13] = 7;
display_mem[19] = 7;
display_mem[22] = 7;
display_mem[23] = 7;
display_mem[24] = 7;
display_mem[25] = 7;
display_mem[26] = 7;
display_mem[33] = 6;
display_mem[36] = 7;
display_mem[40] = 7;
display_mem[42] = 7;
display_mem[46] = 7;
display_mem[50] = 7;
display_mem[51] = 7;
display_mem[54] = 7;
display_mem[65] = 7;
display_mem[72] = 7;
display_mem[78] = 7;

display_mem[81] = 7;

display_mem[83] = 4;

display_mem[86] = 7;

display_mem[87] = 4;

display_mem[88] = 7;

display_mem[89] = 7;

display_mem[97] = 5;

display_mem[101] = 7;

display_mem[102] = 7;

display_mem[103] = 6;

display_mem[108] = 7;

display_mem[109] = 7;

display_mem[112] = 7;

display_mem[115] = 6;

display_mem[122] = 7;

display_mem[129] = 6;

display_mem[132] = 7;

display_mem[142] = 7;

display_mem[144] = 7;

display_mem[145] = 6;

display_mem[146] = 6;

display_mem[147] = 7;

display_mem[148] = 7;

display_mem[154] = 7;

display_mem[161] = 5;

display_mem[164] = 5;

display_mem[170] = 7;

display_mem[174] = 7;

display_mem[179] = 7;

display_mem[182] = 7;

display_mem[186] = 7;

display_mem[192] = 7;

display_mem[193] = 5;

display_mem[194] = 6;

display_mem[196] = 7;

display_mem[197] = 7;

display_mem[198] = 7;

display_mem[199] = 5;

display_mem[200] = 7;

display_mem[203] = 7;

display_mem[204] = 7;

display_mem[205] = 6;

display_mem[211] = 7;

display_mem[215] = 7;

display_mem[216] = 7;

display_mem[217] = 7;

display_mem[289] = 7;

display_mem[290] = 7;

display_mem[291] = 7;

display_mem[294] = 7;

display_mem[295] = 7;

display_mem[296] = 6;

display_mem[297] = 4;

display_mem[298] = 7;

display_mem[301] = 6;

display_mem[302] = 3;

display_mem[303] = 7;

display_mem[307] = 7;

display_mem[308] = 4;

display_mem[309] = 7;

display_mem[313] = 4;

display_mem[314] = 4;

display_mem[315] = 5;

display_mem[320] = 7;

display_mem[324] = 7;

display_mem[330] = 7;

display_mem[332] = 5;

display_mem[336] = 4;

display_mem[338] = 7;

display_mem[342] = 7;

display_mem[344] = 7;

display_mem[348] = 3;

display_mem[352] = 6;

display_mem[361] = 7;

display_mem[364] = 7;

display_mem[368] = 6;

display_mem[370] = 7;

display_mem[374] = 4;

display_mem[376] = 6;

display_mem[380] = 4;

display_mem[384] = 7;

display_mem[385] = 7;

display_mem[386] = 7;

display_mem[387] = 7;

display_mem[392] = 7;

display_mem[397] = 5;

display_mem[398] = 3;

display_mem[399] = 4;

display_mem[403] = 5;

display_mem[404] = 7;

display_mem[405] = 5;

display_mem[406] = 7;

display_mem[408] = 5;

display_mem[412] = 4;

display_mem[416] = 7;

display_mem[420] = 7;

display_mem[424] = 7;

display_mem[428] = 6;

display_mem[432] = 6;

display_mem[438] = 7;

display_mem[440] = 7;

display_mem[444] = 5;

display_mem[448] = 7;

display_mem[452] = 7;

display_mem[456] = 7;

display_mem[460] = 7;

display_mem[464] = 6;

display_mem[466] = 7;

display_mem[470] = 7;

display_mem[472] = 7;

display_mem[476] = 7;

display_mem[481] = 7;

display_mem[482] = 7;

display_mem[483] = 4;

display_mem[488] = 7;

display_mem[493] = 7;

display_mem[494] = 7;

display_mem[495] = 5;

display_mem[499] = 7;

display_mem[500] = 7;

display_mem[501] = 6;

display_mem[505] = 6;

display_mem[506] = 4;

display_mem[507] = 3;
*/



