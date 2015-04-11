/*
	Header file for tetris functions.
	
	Contains function prototypes for tetris game functions
*/

#ifndef tetris_h
#define tetris_h

#include <Arduino.h>

// grrr the compiler doesn't know what a byte is...
typedef unsigned char byte;

// color defines          red, green, blue, yellow, purple, orange, green/blue 
const int16_t partcolor[7] = {448, 7, 56, 480, 71, 455, 62 };
const byte tetris_rows = 26;        // how many rows in full stack
const byte partstart_x = 8;         // x and y starting locations for new parts
const byte partstart_y = 8;

// initialize variables for game
void setup_tetris(int16_t, byte*, byte*, byte*, int16_t*, byte, byte*, byte*, int16_t*, int16_t*);
  
// helper to set the move bounds for the current piece in the gamespace
void setbounds(byte, byte,  byte*, byte*, byte*, byte* );

// update tetris stack with new piece location
void update_stack(byte, byte, byte, byte, byte*, byte, int16_t);

// function to move/rotate part based on NES controller input
void movepart(byte, byte*, byte*, byte*, byte*, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte*, byte, int16_t, unsigned long*);

// function to draw current part and given location/orientation
void drawpart(byte type, byte orientation, byte x_loc, byte y_loc, int16_t clr, int16_t *display_mem, const int16_t *partcolor);

// helper to manage dropping a part down 1 line location
byte droppart(byte, byte, byte, byte*, byte, byte*, byte, int16_t);

// check tetris stack to make sure requested piece move is valid, return true if it is
bool check_stack(byte, byte, byte, byte, byte*, byte, int16_t);
  
// check tetris stack for a completed row
void checkrows(byte, byte*, int16_t*, int16_t*, int16_t*);

// animation to clear a filled row in the stack
void clearrow(byte, byte*, int16_t*);

// print score digits
void printdigit(byte, byte, int16_t*);


#endif
