/*
	Header file for animation functions.
	
	Contains function prototypes for animation functions

   animation_number        animation
   0                       nes_paint
   1                       blink_rand_interval
   2                       bounce
   3                       rain animation
   4                       static color test ( clear_all w/ color argument )
   4                       random fill, unfill of display, random or fixed color
   5                       random led on fade off
   6                       wave   
   7			   line sweeper
   8                       chaser
   9                       ir sense
   10                      text scroll
   11                      "sprite" animate
   12                      stopwatch
   13		           serial control
   15                      it's a secret to everyone
   
   
*/

#ifndef animations_h
#define animations_h

#include "pgm_memory.h"       // data defined for program memory such as sprites and scroll text alphabet
#include "tetris.h"

// grrr the compiler doesn't know what a byte is...
typedef unsigned char byte;

void nes_paint(volatile byte*, byte*, int16_t*, int16_t*, int16_t, int16_t*, int16_t*, byte*, byte, const byte*, byte*, int16_t*, byte*, byte*, byte*, byte, byte*, byte*, int16_t*, int16_t*, byte* );
void blink_rand_interval(byte, byte*, int16_t, int16_t*, int16_t*, int16_t*, byte, int16_t* );
void bounce(byte, int16_t*, byte, byte*, byte*, byte*, byte*, byte*, byte*, byte*, int16_t*, byte*, int16_t, byte, int16_t*);
void pong(int16_t*, int16_t, byte, int16_t*, byte*, int16_t*, byte*, byte*, byte*, byte*, byte*, byte*, byte*, int16_t*, byte);
void rain(byte, int16_t*, byte, byte*, byte*, byte*, int16_t*, byte, byte*, float*, byte);
void rfill ( byte, int16_t*, int16_t, byte, byte*, int16_t*, int16_t*, int16_t*, int16_t*, int16_t*);
void random_fade(byte, int16_t*, int16_t, byte, byte*, int16_t*, int16_t, int16_t, byte*, int16_t*, byte, int16_t, int16_t* );
void wave(byte, int16_t*, byte, float*, int16_t*, float*, float*, int16_t*, byte*, byte*, int16_t*, byte*, byte);
void lines( byte, int16_t*, int16_t, byte*, short*, short*, byte, byte, byte );
void chaser(byte, int16_t, int16_t*, byte, byte*, byte*, byte, byte*, byte*, byte*, byte*, short*, short*, int16_t*, byte*, int16_t*, unsigned long*, int16_t );
void light_sense( byte, int16_t*, int16_t, int16_t*, byte*, byte*, volatile byte*, byte*, byte*, byte*, int16_t*, int16_t*, unsigned long*, int16_t*, byte*, byte*, byte, int16_t, byte);
void text_scroll(byte, String, byte, byte, int16_t*, int16_t, int16_t, int16_t*, byte, byte );
void sprite_animate (byte, byte, int16_t*, byte, int16_t, byte*, unsigned long* );
void stopwatch(byte, long*, long*, long*, byte*, int16_t*, byte, byte);
void serial_control(int16_t*, int16_t*, int16_t, int16_t);
void tetris(byte*, byte*, byte*, byte*, byte*, unsigned long*, int16_t*, byte*, byte*, byte*, byte*, int16_t*, int16_t*, int16_t, byte, unsigned long*, byte);
void snake(byte*, int16_t*, int16_t*, int16_t, byte, byte, int16_t*, int16_t*, byte*, byte*, byte*, byte*, int16_t*, int16_t*, byte*, const byte, int16_t*, byte);
void life(byte*, int*, int16_t*, int16_t, byte*, byte, int16_t*, int16_t*, byte);




// helpers
void splash(byte, int16_t*);
void clear_all(int16_t, int16_t, int16_t*);
void blankchar(int16_t, int16_t, int16_t*);
void printchar(int16_t, int16_t, char, int16_t*);
byte check_code( byte*, byte, byte, const byte* );
void cursor_blink(int16_t, int16_t*);
byte fade_down(int16_t, byte, int16_t*);
byte fade_up(int16_t, int16_t, int16_t*);
void draw_paddle(int16_t*, int16_t, byte);
void rfill_init(byte, int16_t, int16_t*);
byte in_rfill(int16_t, int16_t, int16_t*);
void set_led(int16_t, int16_t, int16_t*);
void newline(byte, byte, byte*, short*, short*);
void draw_row(byte, int16_t, int16_t*, byte);
void draw_col(byte, int16_t, int16_t*, byte);
int16_t opposite_color(int16_t);
int16_t paint_choose(int16_t*, int16_t*, volatile byte*, int16_t, byte*, int16_t*, byte*, byte*, byte*, const byte*, byte);
void updateSnakes(int16_t*, byte, int16_t*, byte, const byte, byte);
void drawSnakes(int16_t*, int16_t*, int16_t*, int16_t, const byte, int16_t, int16_t, byte);
void checkSnakes(int16_t*, int16_t*, byte, byte*, byte*, int16_t*, int16_t*, byte, byte);
void newFood(int16_t*, int16_t*, const byte, int16_t*, int16_t*, byte, byte);
void init_life(byte*, int*, int16_t, int16_t*);
 

#endif
