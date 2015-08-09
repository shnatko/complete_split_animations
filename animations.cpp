/*
   animations.cpp
   
   All of the animations functions moved to a separate source file now.  This file will contain the code for running specific animations now.
   The main table source file will contain the framework only for running animations, polling controllers, dealing w/ wifi and the IR array etc..
   
   Although animation source is contained here, globals for animations still are defined in main table source ( maybe try to change this later )
   
   
   animation_number        animation
   0                       nes_paint
   1                       blink_rand_interval
   2                       bounce
   3                       rain animation
   4                       static color test ( clear_all w/ color argument )
   4                       random fill, unfill of display, random or fixed color
   5                       random led on fade off
   6                       wave   
   7			   		   line sweeper
   8                       chaser
   9                       ir sense
   10                      text scroll
   11                      "sprite" animate
   12                      stopwatch
   13					   serial control
   14                      it's a secret to everyone
   15                      snake
   16                      life
  
    Date      Comment
    02.21.15  Created initial version and moved some animations over 
    02.22.15  PITA but moved all the animations over to own source file for better code maintenance 
    03.07.15  First pass at getting snake game working
	04.02.15  Snake improvements, 2 players, border wrap added
    05.01.15  Added game of life
	
	
	TODOs:
	
	 bounce:  for small # of balls, wall strike triggers whole screen on/fade down
	 rain:    added upside-down mode, also stacking mode
	 
*/

#include "animations.h"
#include "tetris.h"
#include <Arduino.h>

// #################################################################
// 0: paint using nes controller
void nes_paint(volatile byte *nes_state1, byte *last_button, int16_t *display_mem, int16_t *rfill_array, int16_t ledCount, int16_t *color, int16_t *nes_location, byte *current_code, byte debug, const byte *code_array, byte *animation_number, int16_t *animation_interval, byte *tetris_stack, byte *part_type, byte *next_part, byte partstart_y, byte *gameOver, byte *previousPauseButtonState, int16_t *fall_rate, int16_t *score, byte *color_select ) {

  // grab current NES control button state
  byte up, down, left, right, a, b, sel, start = 1;
  String paint_string = "";
  byte code_valid = 0;

  up = bitRead(*nes_state1, 4);
  down = bitRead(*nes_state1, 5);
  left = bitRead(*nes_state1, 6);
  right = bitRead(*nes_state1, 7);
  a = bitRead(*nes_state1, 0);
  b = bitRead(*nes_state1, 1);
  sel = bitRead(*nes_state1, 2);
  start = bitRead(*nes_state1, 3);


  if ( *nes_state1 == 255 ) {
    *last_button = 99;
  }

  // start button used to clear array
  if ( start == 0 ) {

	
	if ( debug == 1 ) Serial.println("screen output:");
    
	for ( int16_t i = 0; i < ledCount; i++ ) {

		// if (display_mem[i] > 0) {	
			// paint_string = String("display_mem[");
			// paint_string = String(paint_string + i + "] = " + display_mem[i] + ";" );
			// if (debug == 2 && display_mem[i] > 0  ) {
			// Serial.println(paint_string);
			// }
		// }
		
		if ( debug == 1 ) {
			Serial.print(display_mem[i]);
			Serial.print(", ");
			Serial.println("done!");
		}
		
		display_mem[i] = 0;
	
	}
  	
	// clear paint choose state
	*color_select = 0;
    
	// check current button combo
	code_valid = check_code(current_code, 7, *last_button, code_array);
	*last_button = 7;

  }

  // a button will paint selected LED up by current color
  if ( a == 0 ) {
    // paint LED up based on currently selected color
    display_mem[*nes_location] = *color;
   
    // check current button combo
    code_valid = check_code(current_code, 5, *last_button, code_array);
    *last_button = 5;

  }

  // b button will paint selected LED down by current color
  if ( b == 0 ) {
    // paint LED down based on currently selected color
    display_mem[*nes_location] = 0;

    // check current button combo
    code_valid = check_code(current_code, 4, *last_button, code_array);
    *last_button = 4;
  }
  
  // select button used to display color selector

  if ( sel == 0 || *color_select == 1 ) {
	
	if ( sel == 0 ) {
		// check current button combo before doing other shit
		code_valid = check_code(current_code, 6, *last_button, code_array);
		*last_button = 6;
	}
    
    if ( sel == 0 && *color_select == 0 ) {
      *color_select = 1;
      
      // need to save current state of display_mem to temporary location and then restore it back once selection is made, re-purpose rfill_array variable already declared for this purpose
      memcpy( rfill_array, display_mem, 1024 );
    }
    
    *color = paint_choose(display_mem, rfill_array, nes_state1, ledCount, color_select, nes_location, &code_valid, current_code, last_button, code_array, debug);

  }

  // LED cursor position is being moved, calculate new position
  if ( up == 0 ) {
    // move led up if not in top row already, no wrap
    if (*nes_location > 31) {
      *nes_location = *nes_location - 32;
      cursor_blink(*nes_location, display_mem);
    }

    // check current button combo
    code_valid = check_code(current_code, 0, *last_button, code_array);
    *last_button = 0;


  } else if ( down == 0 ) {
    // move led down if not in bottom row already, no wrap
    if (*nes_location < 480) {
      *nes_location = *nes_location + 32;
      cursor_blink(*nes_location, display_mem);
    }

    // check current button combo
    code_valid = check_code(current_code, 1, *last_button, code_array);
    *last_button = 1;

  } else if ( left == 0 ) {
    if (*nes_location % 32 > 0) {       // move left, no wrap
      *nes_location = *nes_location - 1;
      cursor_blink(*nes_location, display_mem);
    }

    // check current button combo
    code_valid = check_code(current_code, 2, *last_button, code_array);
    *last_button = 2;

  } else if ( right == 0 ) {
    if (*nes_location % 32 < 31) {        // move right, no wrap
      *nes_location = *nes_location + 1;
      cursor_blink(*nes_location, display_mem);
    }

    // check current button combo
    code_valid = check_code(current_code, 3, *last_button, code_array);
    *last_button = 3;

  }

  if ( debug == 4 ) {
	for ( byte i=0; i<14; i++ ) {
		Serial.print(current_code[i]);
	}
	Serial.println(" e");
  }
  
  
  // konami code valid!  switch to tetris mode
  if ( code_valid == 1 ) {
    clear_all(0, ledCount, display_mem);
    splash(1, display_mem);
    setup_tetris(ledCount, tetris_stack, part_type, next_part, display_mem, partstart_y, gameOver, previousPauseButtonState, fall_rate, score);
    *animation_number = 14;
    *animation_interval = 50;
  }
  

  code_valid = 0;

}

// #################################################################
// 1: animation to randomly turn on a LED for a random amount of time
void blink_rand_interval(byte type, byte *blink_count, int16_t ledCount, int16_t *display_mem, int16_t *wave_color, int16_t *sense_color_on, byte nes_state1, int16_t *animation_interval ) {

  byte up, down, a, b, sel, start = 1;

  *blink_count = constrain(*blink_count, 1, 200);

  // basic random blink
  if ( type == 0 ) {
    clear_all(0, ledCount, display_mem);
    for ( byte i = 0; i < *blink_count; i++ ) {

      //grab an led to update
      int16_t led_to_update = random(0, ledCount);

      //if led is currently on, turn it off
      if ( display_mem[led_to_update] > 0 ) {
        display_mem[led_to_update] = 0;      // turn LED off
      } else {
        if ( wave_color[0] == 0 ) {
          display_mem[led_to_update] = random(0, 512);
        } else {
          display_mem[led_to_update] = *sense_color_on;
        }
      }
    }
  }

  // check controller inputs to speed up/slow down or change the number of active LEDs
  // a/b speed up / slow down blinking
  // up/down increase / decrease number of leds checked

  up = bitRead(nes_state1, 4);
  down = bitRead(nes_state1, 5);
  a = bitRead(nes_state1, 0);
  b = bitRead(nes_state1, 1);
  sel = bitRead(nes_state1, 2);
  start = bitRead(nes_state1, 3);

  if ( a == 0 ) {
    *animation_interval = *animation_interval - 1;
    *animation_interval = constrain(*animation_interval, 5, 150);
  }

  if ( b == 0 ) {
    *animation_interval = *animation_interval + 1;
    *animation_interval = constrain(*animation_interval, 5, 150);
  }

  if ( up == 0 ) {
    *blink_count = *blink_count + 1;
    *blink_count = constrain(*blink_count, 1, 200);
  }

  if ( down == 0 ) {
    *blink_count = *blink_count - 1;
    *blink_count = constrain(*blink_count, 1, 200);
  }

  if ( sel == 0 ) {
    *sense_color_on = random(1, 512);
  }

  // reuse wave_color[0] for same thing here, determine if LED is random color or static
  if (start == 0 ) {
    if (wave_color[0] == 0 ) {
      wave_color[0] = 1;
    } else if ( wave_color[0] == 1 ) {
      wave_color[0] = 0;
    }
  }
}

// #################################################################
// 2: bouncing balls
void bounce(byte type, int16_t *display_mem, byte balls, byte *location_x, byte *location_y, byte *direction_x, byte *direction_y, byte *speed_x, byte *speed_y, byte *interval_counter, int16_t *ball_color, byte *shape, int16_t ledCount, byte nes_state1, int16_t *animation_interval) {

  /*  type info:
      type = 0, single pixels bouncing around, if count < 25, fade down, otherwise just clear previous position
      type = 1, use shape setting, shapes so far include doughnut, circle, and square, no fade, just clear prev. position
      type = 2, use shape setting, but persistence setting, no clear
      type = 3, use shape setting, but fade down shape after moving it
  */

  byte flash_top = 0;
  byte flash_bottom = 0;
  byte flash_left = 0;
  byte flash_right = 0;
  int16_t edge_flash_color = 7;
  byte left_edge = 0;
  byte right_edge = 31;
  byte top = 0;
  byte bottom = 15;
  byte a, b, sel, start = 1;

  //if type is 1, we're using bigger balls, adjust edges accordingly
  if ( type > 0 ) {
    left_edge = 1;
    right_edge = 29;
    top = 2;
    bottom = 14;
  }

  // fade down previous led position when ball count <=25
  if ( (balls <= 25 && type == 0) || type == 3 ) {
    for ( int16_t j = 0; j < ledCount; j++) {
      fade_down(j, 1, display_mem);
    }
  }

  for (int16_t i = 0; i < balls; i++ ) {

    //clear last frame ( no fade ) for ball count > 2
    if ( balls > 25 || type == 1 ) {

      int16_t ball_loc = location_x[i] + (32 * location_y[i]);

      if ( type == 1 ) {

        // clear center of ball
        if ( shape[i] == 0 || shape[i] == 2 ) {
          display_mem[ball_loc] = 0;
          display_mem[ball_loc + 1] = 0;
          display_mem[ball_loc - 32] = 0;
          display_mem[ball_loc - 31] = 0;
        }

        // clear outer ring of ball ( for any shape )
        display_mem[ball_loc - 1] = 0;
        display_mem[ball_loc + 2] = 0;
        display_mem[ball_loc + 32] = 0;
        display_mem[ball_loc + 33] = 0;
        display_mem[ball_loc - 33] = 0;
        display_mem[ball_loc - 30] = 0;
        display_mem[ball_loc - 64] = 0;
        display_mem[ball_loc - 63] = 0;


        // clear corners for square shapes
        if ( shape[i] == 2 ) {
          display_mem[ball_loc - 65] = 0;
          display_mem[ball_loc - 62] = 0;
          display_mem[ball_loc + 31] = 0;
          display_mem[ball_loc + 34] = 0;
        }

      } else {
        display_mem[location_x[i] + (32 * location_y[i])] = 0;
      }

    }


    // calculate new X position if interval counter is multiple of ball's speed rating ( high speed rating means lower update interval in this case )
    if ( *interval_counter % speed_x[i] == 0 ) {

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
    if ( *interval_counter % speed_y[i] == 0 ) {
      if ( location_y[i] < bottom && direction_y[i] == 1) {
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

    if ( type > 0) {
      //draw 12-pixel ball based on current location
      int16_t ball_loc = location_x[i] + (32 * location_y[i]);

      //draw "center" of ball
      if ( shape[i] == 0 || shape[i] == 2 ) {
        display_mem[ball_loc] = ball_color[i];
        display_mem[ball_loc + 1] = ball_color[i];
        display_mem[ball_loc - 32] = ball_color[i];
        display_mem[ball_loc - 31] = ball_color[i];
      }

      //draw out edge of ball for all 3 shapes
      display_mem[ball_loc - 1] = ball_color[i];
      display_mem[ball_loc - 33] = ball_color[i];
      display_mem[ball_loc + 2]  = ball_color[i];
      display_mem[ball_loc + 32] = ball_color[i];
      display_mem[ball_loc + 33] = ball_color[i];
      display_mem[ball_loc - 30] = ball_color[i];
      display_mem[ball_loc - 64] = ball_color[i];
      display_mem[ball_loc - 63] = ball_color[i];

      //draw corners of square
      if ( shape[i] == 2 ) {
        display_mem[ball_loc - 65] = ball_color[i];
        display_mem[ball_loc - 62] = ball_color[i];
        display_mem[ball_loc + 31] = ball_color[i];
        display_mem[ball_loc + 34] = ball_color[i];
      }

    } else {
      display_mem[location_x[i] + (32 * location_y[i])] = ball_color[i];
    }

  }

  // increment interval counter, used to determine whether or not to update an led's position based on it's speed setting
  if ( *interval_counter < 16 ) {
    *interval_counter = *interval_counter + 1;
  } else {
    *interval_counter = 0;
  }

  // lastly, check NES controller input to speed up/slow down movement, or chance ball colors, or clear screen
  a = bitRead(nes_state1, 0);
  b = bitRead(nes_state1, 1);
  sel = bitRead(nes_state1, 2);
  start = bitRead(nes_state1, 3);

  if ( a == 0 ) {
    *animation_interval = *animation_interval - 1;
    *animation_interval = constrain(*animation_interval, 1, 200);
  }

  if ( b == 0 ) {
    *animation_interval = *animation_interval + 1;
    *animation_interval = constrain(*animation_interval, 1, 200);
  }

  if ( sel == 0 ) {
    for ( byte i = 0; i < balls; i++ ) {
      ball_color[i] = random(0, 512);
    }
  }

  if ( start == 0 ) {
    clear_all(0, ledCount, display_mem);
  }


}


// #################################################################
// 3: pong.. still a little buggy

void pong(int16_t *display_mem, int16_t ledCount, byte nes_state1, int16_t *nes_location, byte *paddle_width, int16_t *score, byte *direction_x, byte *direction_y, byte *location_x, byte *location_y, byte *speed_x, byte *speed_y, byte *interval_counter, int16_t *ball_color, byte debug) {

  // grab current NES control button state
  byte left, right, flash, start, select = 1;

  left = bitRead(nes_state1, 6);
  right = bitRead(nes_state1, 7);
  start = bitRead(nes_state1, 3);
  select = bitRead(nes_state1, 2);

  //re-init game when user presses start
  if ( start == 0 ) {
    clear_all(0, ledCount, display_mem);
    *nes_location = 494;   // starting paddle location
    direction_x[0] = random(1, 2);
    direction_y[0] = random(1, 2);
    location_x[0] = random(0, 32);
    location_y[0] = random(0, 3);
    speed_x[0] = B00000001 << random(0, 3);
    speed_y[0] = speed_x[0];
    //speed_y[0] = B00000001 << random(0,3);
    //ball_color[i] = (7*random(0,2)) + (56*random(0,2))+(448*random(0,2));
    ball_color[0] = random(0, 512);
    *interval_counter = 0;
    flash = 0;
    *score = 0;
  }

  //change paddle width when user presses select  ( make this a level thing later )
  if ( select == 0 ) {

    if (*paddle_width < 8 ) {
      *paddle_width = *paddle_width + 1;
    } else {
      *paddle_width = 2;
    }
  }


  // check direction buttons and move paddle to new location
  if ( left == 0 ) {
    if (*nes_location > 480 && *nes_location > 479) {         // move left
      *nes_location = *nes_location - 1;
    }
  } else if ( right == 0 ) {
    if (*nes_location < (512 - *paddle_width) ) {             // move right
      *nes_location = *nes_location + 1;
    }
  }

  // calculate new position for ball

  //clear last frame
  display_mem[location_x[0] + (32 * location_y[0])] = 0;

  // calculate new X position if interval counter is multiple of ball's speed rating ( high speed rating means lower update interval in this case )
  if ( *interval_counter % speed_x[0] == 0 ) {
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
  if ( *interval_counter % speed_y[0] == 0 ) {
    if ( location_y[0] < 15 && direction_y[0] == 1) {
      location_y[0]++;
    }  else if ( location_y[0] > 0 && direction_y[0] == 0 ) {
      location_y[0]--;
    } else if ( location_y[0] == 15 ) {    // at bottom, change direction up if x location is within paddle range

      // check that ball location is on paddle
      if ( (location_x[0] + 480) >= *nes_location && (location_x[0] + 480) < (*nes_location + *paddle_width) ) {
        location_y[0]--;
        direction_y[0] = 0;
        *score = *score + 1;
      } else {
        location_y[0] = 16;
        flash = 1;
        if (debug == 1) {
          Serial.print(location_x[0]);
          Serial.print("  ");
          Serial.println(*nes_location);
        }
      }
    } else if ( location_y[0] == 0  ) {    // at top, change direction down
      location_y[0]++;
      direction_y[0] = 1;
    }
  }

  // draw ball
  if (location_y[0] < 16 ) {
    display_mem[location_x[0] + (32 * location_y[0])] = ball_color[0];
  }

  // show score at top if ball escaped
  if (flash == 1) {
    for ( byte i = 0; i < *score; i++ ) {
      display_mem[i] = 56;
    }
    display_mem[location_x[0]] = 448;
  }

  // draw paddle now
  draw_paddle(display_mem, *nes_location, *paddle_width);

  // lastly increment interval counter, used to determine whether or not to update an led's position based on it's speed setting
  if ( *interval_counter < 16 ) {
    *interval_counter = *interval_counter + 1;
  } else {
    *interval_counter = 0;
  }

}

// #################################################################
// 3: rain animation, led will fade in at top then "drip" down

void rain(byte type, int16_t *display_mem, byte nes_state1, byte *drop_state, byte *drop_pos_y, byte *drop_wait_count, int16_t *drop_color, byte drop_wait_time, byte *drop_fall_timer, float *gravity, byte debug) {

  byte fade_on = 0;
  byte fade_off = 0;
  byte a, b = 1;

  if ( type == 0 ) {  // basic random column raindrop

    //byte active_col = random(0,32);

    for ( byte active_col = 0; active_col < 32; active_col++ ) {

      if ( debug == 2 ) {
        Serial.print(active_col);
        Serial.print(" state: ");
        Serial.println(drop_state[active_col]);
      }

      switch (drop_state[active_col]) {

        case 0:  // drop is in idle state, decrement it's counter
          if ( drop_wait_count[active_col] > 0 ) {
            drop_wait_count[active_col]--;
          } else if ( drop_wait_count[active_col] == 0 ) {   // wait count hit zero, move to fade in state
            drop_wait_count[active_col] = random(1, drop_wait_time);
            drop_state[active_col] = 1;
          }
          break;

        case 1: // drop is in fade in state, fade up LED brightness to max color

          fade_on = fade_up(active_col, drop_color[active_col], display_mem);

          if (fade_on == 1) {
            drop_wait_count[active_col]--;  				// adding random wait at full brightness before drop fall
            if ( drop_wait_count[active_col] == 0 ) {
              drop_state[active_col] = 2;
              drop_fall_timer[active_col] = 0;
              drop_wait_count[active_col] = random(1, drop_wait_time);
            }
          }
          break;

        case 2: // drop is in fall state
          if ( drop_pos_y[active_col] < 15 ) {

            //clear current drop location
            display_mem[(32 * drop_pos_y[active_col]) + active_col] = 0;


            //  below method used equation to determine position based on fall timer comment out for now, calculate d for given t
            // calculate new freefall location based on drop time and acceleration constant
            float displacement = 0.5 * *gravity * pow(drop_fall_timer[active_col], 2);
            drop_pos_y[active_col] = constrain(int(displacement), 0, 15);

            display_mem[(32 * drop_pos_y[active_col]) + active_col] = drop_color[active_col];

            // bump drop time by 1 "unit" for next time through
            drop_fall_timer[active_col]++;


            /*
            // new method, calculate t for a given d and turn on that LED if sufficient time has passed to light it up, hopefully this method will illuminate each LED in the fall

            //for each drop position, calculate time when drop should be at that point.  if the current time is greater than the time for the drop to be at location, light up led
            float droptime = 0;
            byte ondrop = 0;
            for ( byte i=0; i<16; i++) {
            droptime = sqrt((2*i)/gravity);
            if ( drop_fall_timer[active_col] > droptime ) ondrop = i;
            }

            //once we have the lowest drop, turn that LED on and increment the drop timer
            drop_pos_y[active_col] = ondrop;
            display_mem[(32*drop_pos_y[active_col]) + active_col] = drop_color[active_col];
            drop_fall_timer[active_col]++;
            */


          } else if ( drop_pos_y[active_col] == 15 ) {
            drop_pos_y[active_col] = 15;
            drop_state[active_col] = 3;
            display_mem[(32 * drop_pos_y[active_col]) + active_col] = drop_color[active_col];
          }
          break;

        case 3: // drop is at bottom, fade it back down now

          fade_off = fade_down((32 * drop_pos_y[active_col]) + active_col, 1, display_mem);

          if (fade_off == 1 ) {
            drop_state[active_col] = 0;
            drop_pos_y[active_col] = 0;
            drop_fall_timer[active_col] = 0;
          }

          break;
      }

    }
  }

  // lastly, check NES controller input buttons to change gravity strength
  a = bitRead(nes_state1, 0);
  b = bitRead(nes_state1, 1);

  if ( a == 0 ) {
    *gravity = *gravity + 0.05;
    *gravity = constrain(*gravity, 0.05, 1.8);
  }

  if ( b == 0 ) {
    *gravity = *gravity - 0.05;
    *gravity = constrain(*gravity, 0.05, 1.8);
  }



  //lastly check if current time is greater than color cycle interval and if so, change fade color.. fix this.  this breaks things
  /*
  if ( millis() - previousSenseMillis > fade_color_interval ) {
    clear_all(0, ledCount, display_mem);
    int color = random(1,512);
    for ( byte i=0; i<32; i++ ) {
      drop_color[i] = color;
    }
    previousSenseMillis = millis();
  }
  */

}

// #################################################################
// 4: random screen fill
void rfill ( byte type, int16_t *display_mem, int16_t ledCount, byte nes_state1, byte *fill_dir, int16_t *rfill_array, int16_t *current_pos, int16_t *wave_color, int16_t *sense_color_on, int16_t *animation_interval ) {

  byte a, b, up, sel, start = 1;

  // screen is not full yet, fill up
  if ( *fill_dir == 0 && display_mem[rfill_array[*current_pos]] == 0 ) {
    if ( wave_color[0] == 0 ) {
      display_mem[rfill_array[*current_pos]] = random(1, 512);
    } else {
      display_mem[rfill_array[*current_pos]] = *sense_color_on;
    }
    *current_pos = *current_pos + 1;
	
	// for type 1, change fill color halfway up the fill
	if ( type == 2 && *current_pos == 255 ) *sense_color_on = random(1,512);
  }

  // screen is full and we're turning off instead
  if ( *fill_dir == 1 && display_mem[rfill_array[*current_pos]] > 0 ) {
    display_mem[rfill_array[*current_pos]] = 0;
    *current_pos = *current_pos + 1;
  }

  // check state of array to decide if we should be in fill up mode or fill down mode
  byte array_full = 1;
  byte array_empty = 1;
  for ( int16_t i = 0; i < ledCount; i++ ) {
    if ( display_mem[i] > 0 ) array_empty = 0;
    if ( display_mem[i] == 0 ) array_full = 0;
  }

  // if the array was found to be full or empty, toggle fill direction and clear current position in rfill_array
  if ( array_full == 1 ) {
    *fill_dir = 1;
    *current_pos = 0;
  }

  if ( array_empty == 1 ) {
    *current_pos = 0;
    *fill_dir = 0;
    if (type == 1) *sense_color_on = random(1, 512);
  }

  // as a backup, if we made it this far and current_pos count reached 512, automatically re-init array because something went wrong
  if ( *current_pos > 511 ) {
    *current_pos = 0;
    rfill_init(0, ledCount, rfill_array);
    rfill_init(1, ledCount, rfill_array);
    *fill_dir = 0;
    clear_all(0, ledCount, display_mem);
  }

  // read NES state to control screen state/color used
  sel = bitRead(nes_state1, 2);
  start = bitRead(nes_state1, 3);
  up = bitRead(nes_state1, 4);
  a = bitRead(nes_state1, 0);
  b = bitRead(nes_state1, 1);

  // blank screen on start and set fill dir = 0
  if ( start == 0 ) {
    clear_all(0, ledCount, display_mem);
    current_pos = 0;
    rfill_init(0, ledCount, rfill_array);
    rfill_init(1, ledCount, rfill_array);
    *fill_dir = 0;
  }

  // sel swaps between random fill and fixed (random) color fill
  if (sel == 0 ) {
    if (wave_color[0] == 0 ) {
      wave_color[0] = 1;
    } else if ( wave_color[0] == 1 ) {
      wave_color[0] = 0;
    }
  }

  // change fixed fill color w/ up button
  if ( up == 0 ) {
    *sense_color_on = random(1, 512);
  }

  // speed up / slow down fill w/ a/b
  if ( a == 0 ) {
    *animation_interval = *animation_interval - 1;
    *animation_interval = constrain(*animation_interval, 1, 50);
  }

  if ( b == 0 ) {
    *animation_interval = *animation_interval + 1;
    *animation_interval = constrain(*animation_interval, 1, 50);
  }

}


// #################################################################
// 5:  animation to randomly fade each led in the array on/off\
// modified fade animation to have random fwait interval before turning led on.  also, led turn on to full color level and only fades to off.  nicer effect
void random_fade(byte type, int16_t *display_mem, int16_t ledCount, byte fadeLeds, byte *wait_interval, int16_t *led_to_fade, int16_t fwait_min, int16_t fwait_max, byte *fw_state, int16_t *fade_color, byte firework_count, int16_t debug, int16_t *animation_interval ) {


  // basic blink-on fade down individual LEDs
  if ( type == 0 ) {
    // only work with fadeLeds count of leds at a time for performance
    for ( byte i = 0; i < fadeLeds; i++ ) {

      // if led has non-zero brightness, fade it down, also if it has faded down to zero on all 3 colors, replace led_to_fade array location with new random led
      if ( fade_down(led_to_fade[i], 1, display_mem) == 1 ) {
        led_to_fade[i] = random(0, ledCount);
      }

      // if led is off and wait count non-zero, keep waiting
      if (wait_interval[led_to_fade[i]] > 0 && display_mem[led_to_fade[i]] == 0 ) {
        wait_interval[led_to_fade[i]]--;
      }

      // if count has reached zero re-init wait count and start fade by turning brightness on led to 1
      if (wait_interval[led_to_fade[i]] == 0) {
        // re-randomize wait interval for next time though for this LED
        wait_interval[led_to_fade[i]] = random(fwait_min, fwait_max);
        byte color = random(0, 7);
        if (color == 0) {
          display_mem[led_to_fade[i]] = 448;  // red
        } else if (color == 1) {
          display_mem[led_to_fade[i]] = 7;    // blue
        } else if (color == 2)  {
          display_mem[led_to_fade[i]] = 36;   // blue green
        } else if (color == 3) {
          display_mem[led_to_fade[i]] = 455;  // red blue
        } else if (color == 4 ) {
          display_mem[led_to_fade[i]] = 56;   // green
        } else if (color == 5 ) {
          display_mem[led_to_fade[i]] = 504;  // red green ( yellow )
        } else if (color == 6 ) {
          display_mem[led_to_fade[i]] = 480;  // red green ( orange )
        }
      }
    }
  }

  // "fireworks" mode. play firework animation w/ fade down from center
  if ( type == 1 ) {

    for ( byte i = 0; i < firework_count; i++ ) {

      // if led is off and wait count non-zero, keep waiting
      if (wait_interval[led_to_fade[i]] > 0 && fw_state[i] == 0 ) {
        wait_interval[led_to_fade[i]]--;
      }

      // if count has reached zero re-init wait count and start firework animation
      if (wait_interval[led_to_fade[i]] == 0 && fw_state[i] == 0) {
        // re-randomize wait interval for next time through for this LED
        wait_interval[led_to_fade[i]] = random(fwait_min, fwait_max);

        //start firework animation
        fw_state[i] = 1;
        byte color = random(0, 7);
        if (color == 0) {
          color = 448;  // red
        } else if (color == 1) {
          color = 7;    // blue
        } else if (color == 2)  {
          color = 36;   // blue green
        } else if (color == 3) {
          color = 70;  // red blue
        } else if (color == 4 ) {
          color = 56;   // green
        } else if (color == 5 ) {
          color = 376;  // red green ( yellow )
        } else if (color == 6 ) {
          color = 456;  // red green ( orange )
        }  else {
          color = 511;
        }
        fade_color[i] = color;
      }

      // set LEDs on based on where in animation frame the current LED is
      switch (fw_state[i]) {

        case 1:
          set_led(led_to_fade[i], fade_color[i], display_mem);
          fw_state[i] = 2;
          break;

        case 2:
          set_led(led_to_fade[i] - 33, fade_color[i], display_mem);
          set_led(led_to_fade[i] - 31, fade_color[i], display_mem);
          set_led(led_to_fade[i] + 31, fade_color[i], display_mem);
          set_led(led_to_fade[i] + 33, fade_color[i], display_mem);
          fw_state[i] = 3;
          break;

        case 3:
          set_led(led_to_fade[i] - 32, fade_color[i], display_mem);
          set_led(led_to_fade[i] - 1 , fade_color[i], display_mem);
          set_led(led_to_fade[i] + 1 , fade_color[i], display_mem);
          set_led(led_to_fade[i] + 32, fade_color[i], display_mem);
          set_led(led_to_fade[i] - 66, fade_color[i], display_mem);
          set_led(led_to_fade[i] - 62, fade_color[i], display_mem);
          set_led(led_to_fade[i] + 62, fade_color[i], display_mem);
          set_led(led_to_fade[i] + 66, fade_color[i], display_mem);
          fw_state[i] = 4;
          break;

        case 4:
          set_led(led_to_fade[i] - 64, fade_color[i], display_mem);
          set_led(led_to_fade[i] - 2 , fade_color[i], display_mem);
          set_led(led_to_fade[i] + 2 , fade_color[i], display_mem);
          set_led(led_to_fade[i] + 64, fade_color[i], display_mem);
          set_led(led_to_fade[i] - 99, fade_color[i], display_mem);
          set_led(led_to_fade[i] - 93, fade_color[i], display_mem);
          set_led(led_to_fade[i] + 93, fade_color[i], display_mem);
          set_led(led_to_fade[i] + 99, fade_color[i], display_mem);
          fw_state[i] = 5;
          break;

        case 5:
          set_led(led_to_fade[i] - 96, fade_color[i], display_mem);
          set_led(led_to_fade[i] - 3 , fade_color[i], display_mem);
          set_led(led_to_fade[i] + 3 , fade_color[i], display_mem);
          set_led(led_to_fade[i] + 96, fade_color[i], display_mem);

          // once fully drawn, wait random amount of time before fading back down
          fw_state[i] = 6;
          wait_interval[led_to_fade[i]] = random(1, 10);
          break;

        case 6:
          if (wait_interval[led_to_fade[i]] > 0 ) {
            wait_interval[led_to_fade[i]]--;
          } else if ( wait_interval[led_to_fade[i]] == 0 ) {
            fw_state[i] = 7;
          }
          break;

        case 7:
          // fade down firework by calling fade_down function for each location until the last led turned on is back to zero
          fade_down(led_to_fade[i], 1, display_mem);
          fw_state[i] = 8;
          break;

        case 8:
          fade_down(led_to_fade[i], 1, display_mem);
          fade_down(led_to_fade[i] - 33, 1, display_mem);
          fade_down(led_to_fade[i] - 31, 1, display_mem);
          fade_down(led_to_fade[i] + 31, 1, display_mem);
          fade_down(led_to_fade[i] + 33, 1, display_mem);
          fw_state[i] = 9;
          break;

        case 9:
          fade_down(led_to_fade[i], 1, display_mem);
          fade_down(led_to_fade[i] - 33, 1, display_mem);
          fade_down(led_to_fade[i] - 31, 1, display_mem);
          fade_down(led_to_fade[i] + 31, 1, display_mem);
          fade_down(led_to_fade[i] + 33, 1, display_mem);
          fade_down(led_to_fade[i] - 32, 1, display_mem);
          fade_down(led_to_fade[i] - 1, 1, display_mem);
          fade_down(led_to_fade[i] + 1, 1, display_mem);
          fade_down(led_to_fade[i] + 32, 1, display_mem);
          fade_down(led_to_fade[i] - 66, 1, display_mem);
          fade_down(led_to_fade[i] - 62, 1, display_mem);
          fade_down(led_to_fade[i] + 62, 1, display_mem);
          fade_down(led_to_fade[i] + 66, 1, display_mem);
          fw_state[i] = 10;
          break;

        case 10:
          fade_down(led_to_fade[i], 1, display_mem);
          fade_down(led_to_fade[i] - 33, 1, display_mem);
          fade_down(led_to_fade[i] - 31, 1, display_mem);
          fade_down(led_to_fade[i] + 31, 1, display_mem);
          fade_down(led_to_fade[i] + 33, 1, display_mem);
          fade_down(led_to_fade[i] - 32, 1, display_mem);
          fade_down(led_to_fade[i] - 1, 1, display_mem);
          fade_down(led_to_fade[i] + 1, 1, display_mem);
          fade_down(led_to_fade[i] + 32, 1, display_mem);
          fade_down(led_to_fade[i] - 66, 1, display_mem);
          fade_down(led_to_fade[i] - 62, 1, display_mem);
          fade_down(led_to_fade[i] + 62, 1, display_mem);
          fade_down(led_to_fade[i] + 66, 1, display_mem);
          fade_down(led_to_fade[i] - 64, 1, display_mem);
          fade_down(led_to_fade[i] - 2, 1, display_mem);
          fade_down(led_to_fade[i] + 2, 1, display_mem);
          fade_down(led_to_fade[i] + 64, 1, display_mem);
          fade_down(led_to_fade[i] - 99, 1, display_mem);
          fade_down(led_to_fade[i] - 93, 1, display_mem);
          fade_down(led_to_fade[i] + 93, 1, display_mem);
          fade_down(led_to_fade[i] + 99, 1, display_mem);

          fw_state[i] = 11;
          break;

        case 11:
          byte lastled = 0;
          lastled = lastled + fade_down(led_to_fade[i], 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 33, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 31, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 31, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 33, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 32, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 1, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 1, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 32, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 66, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 62, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 62, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 66, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 64, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 2, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 2, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 64, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 99, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 93, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 93, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 99, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 96, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] - 3, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 3, 1, display_mem);
          lastled = lastled + fade_down(led_to_fade[i] + 96, 1, display_mem);

          if ( debug == 1 ) {
            Serial.print("lastled: ");
            Serial.println(lastled);
          }

          if ( lastled > 24 ) {   // once faded back down fully, assign new random led location to wait on
            led_to_fade[i] = (32 * random(3, 13)) + random(3, 29); // constrain new led location so stays on panel and doesn't fuck up memory
            fw_state[i] = 0;
            wait_interval[led_to_fade[i]] = random(fwait_min, fwait_max);
            *animation_interval = random(10, 40);
            lastled = 0;

            if (debug == 1 ) {
              Serial.print("lastled hit 25 for i = ");
              Serial.println(i);
            }

          } else {
            lastled = 0;
            fw_state[i] = 11;
          }
          break;

      }

      /*
      byte newled = 0;
      //if ( fw_state[i] > 15 ) {
        // now fade down the LEDs in the current star
        // if led has non-zero brightness, fade it down, also if it has faded down to zero on all 3 colors, replace led_to_fade array location with new random led
        newled = fade_down(led_to_fade[i]);
        fade_down(led_to_fade[i]-33);
        fade_down(led_to_fade[i]-31);
        fade_down(led_to_fade[i]+31);
        fade_down(led_to_fade[i]+33);
        fade_down(led_to_fade[i]-32);
        fade_down(led_to_fade[i]-1);
        fade_down(led_to_fade[i]+1);
        fade_down(led_to_fade[i]+32);
        fade_down(led_to_fade[i]-66);
        fade_down(led_to_fade[i]-62);
        fade_down(led_to_fade[i]+66);
        fade_down(led_to_fade[i]+62);
        fade_down(led_to_fade[i]-64);
        fade_down(led_to_fade[i]-2);
        fade_down(led_to_fade[i]+2);
        fade_down(led_to_fade[i]+64);
        fade_down(led_to_fade[i]-99);
        fade_down(led_to_fade[i]-93);
        fade_down(led_to_fade[i]+93);
        fade_down(led_to_fade[i]+99);
        fade_down(led_to_fade[i]-96);
        fade_down(led_to_fade[i]-3);
        fade_down(led_to_fade[i]+3);
        fade_down(led_to_fade[i]+96);
      //}

      // once center LED fades down, wait a little bit longer to let rest of burst fade, then pick new led location
      if ( newled == 1 ) {
        fw_state[i] = 10;
      }

      if ( fw_state[i] > 9 && fw_state[i] < 15 ) {
          fw_state[i]++;
      } else if ( fw_state[i] >= 15 ) {
          led_to_fade[i] = (32*random(3,13)) + random(3,29);  // constrain new led location so stays on panel and doesn't fuck up memory
          fw_state[i] = 0;
          animation_interval = random(20, 70);
      }
      */
    }
  }
} 


// #################################################################
// 6: wave animation
void wave(byte type, int16_t *display_mem, byte nes_state1, float *angle, int16_t *previous_loc, float *dx, float *dy, int16_t *led_mask, byte *wave_rate, byte *wave_height, int16_t *wave_color, byte *fill_wave, byte debug ) {

  //  type 0:   draw sinusoidal waves
  //  type 1:   color cycle
  //  type 2:   2d sinusoidal
  //  type 3:   color cycle but cycle whole panel in unison rather than as a wave

  float x = *angle;  // this is the current "base" angle to calculate the sin off of
  float y = *angle;
  int16_t new_loc0;      // value basically stores the height of the wave
  int16_t new_loc1;      // value basically stores the height of the wave
  
  // grab current NES control button state
  byte up, down, left, right, a, b, select, start = 1;

  if ( type == 0 ) {
    // loop though each column in the matrix, compute the wave height and draw it
    for (byte i = 0; i < 32; i++) {
      new_loc0 = 8 + (sin(x) * *wave_height);
      new_loc1 = 8 + (sin(-1 * x) * *wave_height);

      x += *dx * (TWO_PI / 32);

      // if option is to draw wave full, iterate from peak of wave for current X position and draw down/up to the center of the board
      if ( *fill_wave == 1 ) {

        // clear previous locations, just clear the whole column pointed to be i
        for ( byte j = 0; j < 16; j++ ) {
          display_mem[32 * j + i] = 0;
        }

        // fill in wave 0 first based on whether or not it's current value is "positive" or "negative" relative to the center of the board
        if ( new_loc0 < 8 ) {
          for ( byte j = new_loc0; j < 8; j++ ) {
            if ( wave_color[0] == 0 ) {
              display_mem[32 * j + i] = random(0, 512);
            } else {
              display_mem[32 * j + i] = wave_color[1];
            }
          }
        } else if ( new_loc0 > 7 ) {
          for ( byte j = 8; j <= new_loc0; j++ ) {
            if ( wave_color[0] == 0 ) {
              display_mem[32 * j + i] = random(0, 512);
            } else {
              display_mem[32 * j + i] = wave_color[1];
            }
          }
        }

        // do the same for wave 1 now
        if ( new_loc1 < 8 ) {
          for ( byte j = new_loc1; j < 8; j++ ) {
            if ( wave_color[0] == 0 ) {
              display_mem[32 * j + i] = random(0, 512);
            } else {
              display_mem[32 * j + i] = wave_color[2];
            }
          }
        } else if ( new_loc1 > 7 ) {
          for ( byte j = 8; j <= new_loc1; j++ ) {
            if ( wave_color[0] == 0 ) {
              display_mem[32 * j + i] = random(0, 512);
            } else {
              display_mem[32 * j + i] = wave_color[2];
            }
          }
        }

      } else {

        // clear previous location
        display_mem[previous_loc[2 * i]] = 0;
        display_mem[previous_loc[(2 * i) + 1]] = 0;

        if ( wave_color[0] == 0 ) {
          display_mem[32 * new_loc0 + i] = random(0, 512);
          display_mem[32 * new_loc1 + i] = random(0, 512);
        } else {
          display_mem[32 * new_loc0 + i] = wave_color[1];
          display_mem[32 * new_loc1 + i] = wave_color[2];
        }

      }

      previous_loc[2 * i] = (32 * new_loc0) + i;
      previous_loc[(2 * i) + 1] = (32 * new_loc1) + i;
    }

  } else if ( type == 1 ) {
    // cycle through colors
	
	// set color greyscale based on sin(x)
    int16_t color_r; 
    int16_t color_g; 
    int16_t color_b;
	 
	for (byte i = 0; i < 32; i++) {
	
		// set color greyscale based on cos(x)
		color_r = (cos(x) * 7);
		color_g = (cos(x+(TWO_PI/3)) * 7);
		color_b = (cos(x+((2*TWO_PI)/3)) * 7);	

		// only take positive portion of value
		if (color_r < 0 ) color_r = 0;
		if (color_b < 0 ) color_b = 0;
		if (color_g < 0 ) color_g = 0;

		// scale greyscale intensity w/ selected color
		color_r = color_r * 64;
		color_g = color_g * 8;
	
		x += *dx * (TWO_PI / 32);

		// lastly, assign led array color
		for ( byte j = 0; j < 16; j++ ) {
			display_mem[32 * j + i] = (color_r + color_g + color_b) & led_mask[32 * j + i];
			//display_mem[32*j + i] = color_r + color_g + color_b;
		}
	 	 
		
	}
	
  } else if ( type == 2 ) {
    
    // graph 2 dimensional equations   
    for (byte i=0; i<32; i++){
      
      for ( byte j=0; j<16; j++ ){
        
        float z = 3.5*(1+(sin(x)*sin(y) ) );  //z oscillates between 0 and 7
        //float z = 9*(1+(sin(x)*sin(y) ) ); // z oscillates between 0 and 18
        
        int16_t color_r = 0;
        int16_t color_g = 0;
        int16_t color_b = 0;      
        
        if ( debug == 1 ) {
          Serial.print("i: " );
          Serial.print(i);
          Serial.print(" j: ");
          Serial.print(j);
          Serial.print(" x: ");
          Serial.print(x);
          Serial.print(" y: ");
          Serial.print(y);
          Serial.print(" z: ");
          Serial.println(z);
        }
        
        /*               
        if ( z < 8 ) {
          color_b = floor(z);
        } 
        if ( z > 5 && z < 13 ) {
          color_g = floor(z-5);
        }
        if ( z > 10 ) {
          color_r = floor(z-10);
        }
        */
        
        
        switch(wave_color[0]) {
        
          case 0:
            color_r = z;
            color_b = 7-z;
            /*
            if ( z < 8 ) color_r = 7-z;
            if ( z > 10 ) color_b = z-11;
            if ( z > 1 && z < 10 ) color_g = z-2;
            if ( z > 9 ) color_g = 16-z;
            */
            break;
          
          
          case 1:
            color_r = z;
            color_g = 7-z;
            /*
            if ( z < 8 ) color_b = 7-z;
            if ( z > 10 ) color_g = z-11;
            if ( z > 1 && z < 10 ) color_r = z-2;
            if ( z > 9 ) color_r = 16-z;
            */            
            break;
            
          case 2: 
            color_b = z;
            color_g = 7-z;
            /*
            if ( z < 8 ) color_g = 7-z;
            if ( z > 10 ) color_r = z-11;
            if ( z > 1 && z < 10 ) color_b = z-2;
            if ( z > 9 ) color_b = 16-z;
            */
            break; 
          
          
        }
            
        display_mem[32*j + i] = (color_r*64) + (color_g*8) + color_b;    
        
        y+=*dy*(TWO_PI / 16 );
        
      }
      
      if (wave_color[1] == 0 ) y = 0;
      x+=*dx*(TWO_PI / 16);
    } 
  
  } else if ( type == 3 ) {
	  
	// set color greyscale based on cos(x)
	int16_t	color_r = (cos(x) * 7);
	int16_t	color_g = (cos(x+(TWO_PI/3)) * 7);
	int16_t	color_b = (cos(x+((2*TWO_PI)/3)) * 7);	
	
	// only take positive portion of value
	if (color_r < 0 ) color_r = 0;
	if (color_b < 0 ) color_b = 0;
	if (color_g < 0 ) color_g = 0;
	
	// scale greyscale intensity w/ selected color
	color_r = color_r * 64;
	color_g = color_g * 8;
	
	// assign color to every led in panel
	if ( type == 3 ) {
		for ( int16_t i = 0; i<512; i++ ) {
			display_mem[i] = (color_r + color_g + color_b) & led_mask[i];
			//display_mem[i] = (color_r) & led_mask[i];

		}
	}
	
  }

  // amount wave "moves" along led array.. higher = faster
  if ( type < 3 ) {
	*angle += (TWO_PI / *wave_rate);
  } else if ( type == 3 ) {
	*angle = *angle + (*dx * (TWO_PI / 32));
  }
 

  // last, check NES control inputs to adjust wavelength or travel rate
  // left/right movements will shrink/increase wavelength  (dx)
  // a/b buttons will decrease/increase wave speed  (wave_rate)
  // up/down will increase/decrease wave height ( wave_height )
  // select will random select colors
  // start will cycle between fully random and fixed color for each wave line

  up = bitRead(nes_state1, 4);
  down = bitRead(nes_state1, 5);
  left = bitRead(nes_state1, 6);
  right = bitRead(nes_state1, 7);
  a = bitRead(nes_state1, 0);
  b = bitRead(nes_state1, 1);
  select = bitRead(nes_state1, 2);
  start = bitRead(nes_state1, 3);

  if ( left == 0 ) {
    *dx = *dx + 0.1;
    *dx = constrain(*dx, 0.1, 3.9);
  }

  if ( right == 0 ) {
    *dx -= 0.1;
    *dx = constrain(*dx, 0.1, 3.9);
  }

  if ( up == 0 ) {
    *wave_height = *wave_height + 1;
    *wave_height = constrain(*wave_height, 1, 8 );
    *dy = *dy + 0.1;
    *dy = constrain(*dy, 0.1, 3.9);
  }

  if ( down == 0 ) {
    *wave_height = *wave_height - 1;
    *wave_height = constrain(*wave_height, 1, 8 );
    *dy = *dy - 0.1;
    *dy = constrain(*dy, 0.1, 3.9);
  }

  if ( b == 0 ) {
    *wave_rate = *wave_rate + 1;
    *wave_rate = constrain(*wave_rate, 1, 40);
  }

  if ( a == 0 ) {
    *wave_rate = *wave_rate - 1;
    *wave_rate = constrain(*wave_rate, 1, 40);
  }

  if ( select == 0 ) {
    if ( type != 2 ) {
      wave_color[1] = random(0, 512);
      wave_color[2] = random(0, 512);
    }
    if ( type == 2 ) wave_color[0] = random(0,3);
  }

  if ( start == 0 ) {
    if (wave_color[0] == 0 ) {
      wave_color[0] = 1;
      fill_wave = 0;
    } else if ( wave_color[0] == 1 ) {
      wave_color[0] = 2;
      *fill_wave = 1;
    } else if ( wave_color[0] == 2 ) {
      wave_color[0] = 0;
      *fill_wave = 0;
    }
  }

}


// #################################################################
// 7:  sweep led row/colum across array, either left/right or up/down
void lines( byte type, int16_t *display_mem, int16_t ledCount, byte *line_type, short *cur_x, short *cur_y, byte rows, byte cols, byte debug ) {

  if ( type < 3 ) {

    for ( byte box = 0; box < 8; box++) {

      // verticle sweep left
      if ( line_type[box] == 0 ) { 	//box

        if (cur_x[box] < (8 * (box / 2)) + 8  ) { // check to see if sweep reached end of board yet    //box,  (8*(box/2))+7

          // first clear prev line and draw new one
          for ( byte i = (8 * (box % 2)); i < (8 * (box % 2)) + 8; i++ ) {
            if ( type == 0 && cur_x[box] > 8 * (box / 2) ) display_mem[32 * i + (cur_x[box] - 1)] = 0;
            if ( type == 1 && cur_x[box] > 8 * (box / 2) ) {
              for ( int16_t j = 8 * (box / 2); j < 8 * (box / 2) + 8; j++ ) {
                fade_down(32 * i + j, 2, display_mem);
              }
            }
            display_mem[32 * i + cur_x[box]] = 448;
          }

          cur_x[box]++;

        } else {        // if end of board has been reached, change line type for next go round
        
          newline(box, line_type[box], line_type, cur_x, cur_y);
        }

      }

      // verticle sweep right
      if ( line_type[box] == 1 ) {

        if ( debug == 2 ) {
          Serial.print("cur_x: ");
          Serial.println(cur_x[box]);
        }

        if (cur_x[box] > (8 * (box / 2)) - 1 ) {

          // now draw new line and clear old one
          for ( byte i = (8 * (box % 2)); i < (8 * (box % 2)) + 8; i++ ) {
            if ( type == 0 && cur_x[box] < (8 * (box / 2)) + 7 ) display_mem[32 * i + (cur_x[box] + 1)] = 0;
            if ( type == 1 && cur_x[box] < (8 * (box / 2)) + 7 ) {
              for ( int16_t j = 8 * (box / 2); j < 8 * (box / 2) + 8; j++ ) {
                fade_down(32 * i + j, 2, display_mem);
              }
            }
            display_mem[32 * i + cur_x[box]] = 7;
          }

          cur_x[box]--;

        } else { // once we hit last column, cycle to next line type
          newline(box, line_type[box], line_type, cur_x, cur_y);
        }
      }

      // horizontal sweep down
      if ( line_type[box] == 2 ) {

        if (cur_y[box] < 8 * (box % 2) + 8 ) {

          // now draw new line and clear old one
          for ( byte i = (8 * (box / 2)); i < (8 * (box / 2)) + 8; i++ ) {
            if ( type == 0 && cur_y[box] > 0 ) display_mem[32 * (cur_y[box] - 1) + i] = 0;
            if ( type == 1 && cur_y[box] > 0 ) {
              for ( int16_t j = 8 * (box % 2); j < 8 * (box % 2) + 8; j++ ) {
                fade_down(32 * j + i, 1, display_mem);
              }
            }
            display_mem[32 * cur_y[box] + i] = 288;
          }

          cur_y[box]++;	// move to next row position

        } else {
          newline(box, line_type[box], line_type, cur_x, cur_y);
        }
      }

      // horizontal sweep up
      if ( line_type[box] == 3 ) {

        if (cur_y[box] > (8 * (box % 2)) - 1  ) {

          // now draw new line and clear old one
          for ( byte i = (8 * (box / 2)); i < (8 * (box / 2)) + 8; i++ ) {
            if ( type == 0 && cur_y[box] < 15 ) display_mem[32 * (cur_y[box] + 1) + i] = 0;
            if ( type == 1 && cur_y[box] < 15 ) {
              for ( int16_t j = 8 * (box % 2); j < 8 * (box % 2) + 8; j++ ) {
                fade_down(32 * j + i, 2, display_mem);
              }
            }
            display_mem[32 * cur_y[box] + i] = 56;

          }

          cur_y[box]--;	// move to next row position

        } else {
          newline(box, line_type[box], line_type, cur_x, cur_y);
        }
      }
    }
  }

  // random row / column lines ( will expand upon later
  if ( type == 3 ) {

    clear_all(0, ledCount, display_mem);

    // use cur_x[0], cur_y[0] for sweeping
    byte row_col = random(0, 2);

    for ( byte i = 0; i < cur_x[0]; i++ ) {
      // draw row or column line
      if ( row_col == 0 ) {
        draw_row(random(0, 15), random(1, 511), display_mem, cols);
      } else {
        draw_col(random(0, 31), random(1, 511), display_mem, rows);
      }
      row_col = random(0, 2);
    }
  }
}



// #################################################################
// 8: color chase
void chaser(byte type, int16_t ledCount, int16_t *display_mem, byte nes_state1, byte *dir, byte *time_chaser_cycle, byte chaser_cycle, byte *last_x0, byte *last_y0, byte *last_x1, byte *last_y1, short *cur_x, short *cur_y, int16_t *fade_color, byte *random_type, int16_t *animation_interval, unsigned long *previousSenseMillis, int16_t fade_color_interval ) {

  byte a, b, up, down, left, right, sel, start = 1;
  byte reset = 0;
  int16_t led_position = 32 * cur_y[0] + cur_x[0];
  int16_t led_position1 = 32 * cur_y[1] + cur_x[1];
  byte y_mode0, y_mode1 = 0;
  byte x_mode0, x_mode1 = 0;

  if ( display_mem[led_position] == 0 ) {
    display_mem[led_position] = fade_color[0];
  } else {

    for ( int16_t j = 0; j < ledCount; j++) {
      fade_down(j, 1, display_mem);
    }
    //display_mem[led_position] = 0;

    /* increment to next led in chain, based on type, if at end, update color
    type     mode
    0        top to bottom scan
    1        bottom to top scan
    2        top to bototm snake
    3        bottom to top snake
    4        clockwise outside
    5        counter clockwise outside
    6        left to right snake
    7        right to left snake
    8        random walk
    */

    switch (type) {

      case 0:  // type 0 scans through LEDs in sequence through 16 rows and 32 columns, updating LED color by 1 each time it scans through the whole array

        cur_x[0]++;
        if ( cur_x[0] > 31 ) {  // at end of row, return to start and increment row
          cur_x[0] = 0;
          cur_y[0]++;
          if ( cur_y[0] > 15 ) { // at end of all rows, go back to start
            cur_y[0] = 0;
            fade_color[0]++;
            if (fade_color[0] > 511) fade_color[0] = 1;
            reset = 1;
          }
        }
        break;

      case 1:  // type 1 scans through LEDs in sequence through 16 rows and 32 columns in reverse order updating LED color by 1 each time it scans through the whole array

        cur_x[0]--;
        if ( cur_x[0] < 0 ) {  // at end of row, return to start and increment row
          cur_x[0] = 31;
          cur_y[0]--;
          if ( cur_y[0] < 0 ) { // at end of all rows, go back to start
            cur_y[0] = 15;
            fade_color[0]++;
            if (fade_color[0] > 511) fade_color[0] = 1;
            reset = 1;
          }
        }
        break;

      case 2:  // type 2 will "snake" through array, jumping down a row and reversing direction when it hits the end of a row, rather than scanning back to the start of the next row

        // set direction for led movement in row based on which row we're in
        *dir = cur_y[0] % 2;

        switch (*dir) {

          case 0:  // left to right led movement
            cur_x[0]++;
            if ( cur_x[0] > 31 ) {
              cur_x[0] = 31;
              cur_y[0]++;
              *dir = 1;
            }
            break;

          case 1:  // right to left led movement
            cur_x[0]--;
            if ( cur_x[0] < 0 ) {
              cur_x[0] = 0;
              cur_y[0]++;
              *dir = 0;
            }
            break;

        }

        if ( cur_y[0] > 15 ) {
          cur_y[0] = 0;
          cur_x[0] = 0;
          dir = 0;
          fade_color[0]++;
          if ( fade_color[0] > 511 ) fade_color[0] = 1;
          reset = 1;
        }
        break;

      case 3:  // type 3 will "snake" through array starting at the bottom jumping up a row and reversing direction when it hits the end of a row

        // set direction for led movement in row based on which row we're in
        *dir = cur_y[0] % 2;

        switch (*dir) {

          case 0:  // left to right led movement
            cur_x[0]++;
            if ( cur_x[0] > 31 ) {
              cur_x[0] = 31;
              cur_y[0]--;
            }
            break;

          case 1:  // right to left led movement
            cur_x[0]--;
            if ( cur_x[0] < 0 ) {
              cur_x[0] = 0;
              cur_y[0]--;
            }
            break;

        }

        if ( cur_y[0] < 0 ) {
          cur_y[0] = 15;
          cur_x[0] = 31;
          *dir = 1;
          fade_color[0]++;
          if ( fade_color[0] > 511 ) fade_color[0] = 1;
          reset = 1;
        }
        break;

      case 4:       // type 4 will just run the pattern around the outside of the array rather than through the whole thing

        if ( cur_y[0] == 0 ) {
          cur_x[0]++;                  // increment to the right if on the top row
          if ( cur_x[0] > 31 ) {       // if at row end, move to incrementing rows, keeping LED in same column
            cur_x[0] = 31;
            cur_y[0]++;
          }
        } else if ( cur_y[0] > 0 && cur_x[0] == 31 ) {  // moving down right side of array
          cur_y[0]++;
          if ( cur_y[0] > 15 ) {
            cur_y[0] = 15;
            cur_x[0]--;
          }
        } else if ( cur_y[0] == 15 && cur_x[0] < 31 ) {   // moveing right to left on the bottom row
          cur_x[0]--;
          if ( cur_x[0] < 0 ) {
            cur_x[0] = 0;
            cur_y[0]--;
          }
        } else if ( cur_y[0] > 0 && cur_x[0] == 0 ) {  // moving up the left side of the array, new color once we get back to the top
          cur_y[0]--;
          if ( cur_y[0] == 0 ) {
            fade_color[0]++;
            if (fade_color[0] > 511 ) fade_color[0] = 1;
            reset = 1;
          }
        }
        break;

      case 5:      // type 5 will just run the pattern around the outside of the array in reverse direction to type 2

        if ( cur_y[0] >= 0 && cur_x[0] == 0 ) {  // moving down left side of array
          cur_y[0]++;
          if ( cur_y[0] > 15 ) {
            cur_y[0] = 15;
            cur_x[0]++;
          }
        } else if ( cur_y[0] == 15 ) {
          cur_x[0]++;                  // increment to the right if on the bottom
          if ( cur_x[0] > 31 ) {       // if at row end, move to incrementing rows, keeping LED in same column
            cur_x[0] = 31;
            cur_y[0]--;
          }
        } else if ( cur_y[0] > 0 && cur_x[0] == 31 ) {  // moving up the right side of the array
          cur_y[0]--;
          if ( cur_y[0] == 0 ) {
            cur_y[0] = 0;
            cur_x[0] = 31;
          }
        } else if ( cur_y[0] == 0 && cur_x[0] <= 31 ) {   // moving right to left on the top row, change color once we hit the end of the row
          cur_x[0]--;
          if ( cur_x[0] == 0 ) {
            cur_x[0] = 0;
            fade_color[0]++;
            if (fade_color[0] > 511 ) fade_color[0] = 1;
            reset = 1;
          }
        }
        break;

      case 6:  // veritcal progression through columns rather than horizonal through rows, left to right top to bottom

        // set direction for led movement in row based on which col we're in
        *dir = cur_x[0] % 2;

        switch (*dir) {

          case 0:  // down a column
            cur_y[0]++;
            if ( cur_y[0] > 15 ) {
              cur_y[0] = 15;
              cur_x[0]++;
              *dir = 1;
            }
            break;

          case 1:  // up a column
            cur_y[0]--;
            if ( cur_y[0] < 0 ) {
              cur_y[0] = 0;
              cur_x[0]++;
              *dir = 0;
            }
            break;
        }

        if ( cur_x[0] > 31 ) {
          cur_y[0] = 0;
          cur_x[0] = 0;
          *dir = 0;
          fade_color[0]++;
          if ( fade_color[0] > 511 ) fade_color[0] = 1;
          reset = 1;
        }
        break;

      case 7: // veritcal progression through columns rather than horizonal through rows, right to left top to bottom

        // set direction for led movement in row based on which col we're in
        *dir = cur_x[0] % 2;

        switch (*dir) {

          case 0:  // down a column
            cur_y[0]++;
            if ( cur_y[0] > 15 ) {
              cur_y[0] = 15;
              cur_x[0]--;
              *dir = 1;
            }
            break;

          case 1:  // up a column
            cur_y[0]--;
            if ( cur_y[0] < 0 ) {
              cur_y[0] = 0;
              cur_x[0]--;
              *dir = 0;
            }
            break;
        }

        if ( cur_x[0] < 0 ) {
          cur_y[0] = 15;
          cur_x[0] = 31;
          *dir = 0;
          fade_color[0]++;
          if ( fade_color[0] > 511 ) fade_color[0] = 1;
          reset = 1;
        }

        break;

      case 8:  // random walk around the array, each update cycle either increment, decrement, or do nothing to cur_x and cur_y

        y_mode0 = random(0, 3);
        x_mode0 = random(0, 3);
        y_mode1 = random(0, 3);
        x_mode1 = random(0, 3);

        // calc new poistion for first wandering led
        if ( x_mode0 == 0 ) {
          if (cur_x[0] < 31 && *last_x0 != 1 ) {
            cur_x[0]++;
            *last_x0 = x_mode0;
          }
        } else if ( x_mode0 == 1 ) {
          if (cur_x[0] > 0 && *last_x0 != 0) {
            cur_x[0]--;
            *last_x0 = x_mode0;
          }
        } else if ( x_mode0 == 2 ) {
          *last_x0 = x_mode0;
        }

        if ( y_mode0 == 0 ) {
          if (cur_y[0] < 15 && *last_y0 != 1) {
            cur_y[0]++;
            *last_y0 = y_mode0;
          }
        } else if ( y_mode0 == 1 ) {
          if (cur_y[0] > 0 && *last_y0 != 0) {
            cur_y[0]--;
            *last_y0 = y_mode0;
          }
        } else if ( y_mode0 == 2 ) {
          *last_y0 = y_mode0;
        }


        // repeat for 2nd wandering led
        if ( x_mode1 == 0 ) {
          if (cur_x[1] < 31 && *last_x1 != 1 ) {
            cur_x[1]++;
            *last_x1 = x_mode1;
          }
        } else if ( x_mode1 == 1 ) {
          if (cur_x[1] > 0 && *last_x1 != 0) {
            cur_x[1]--;
            *last_x1 = x_mode1;
          }
        } else if ( x_mode1 == 2 ) {
          *last_x1 = x_mode1;
        }

        if ( y_mode1 == 0 ) {
          if (cur_y[1] < 15 && *last_y1 != 1) {
            cur_y[1]++;
            *last_y1 = y_mode1;
          }
        } else if ( y_mode1 == 1 ) {
          if (cur_y[1] > 0 && *last_y1 != 0) {
            cur_y[1]--;
            *last_y1 = y_mode1;
          }
        } else if ( y_mode1 == 2 ) {
          *last_y1 = y_mode1;
        }

        if ( (cur_x[0] == 31 && cur_y[0] == 15) || (cur_x[1] == 31 && cur_y[1] == 15) ) reset = 1;
        break;

    }

    led_position = 32 * cur_y[0] + cur_x[0];
    display_mem[led_position] = fade_color[0];

    if ( type == 8 ) {
      led_position1 = 32 * cur_y[1] + cur_x[1];
      display_mem[led_position1] = fade_color[1];
    }

  }

  // randomize chaser type when we do a cycle if programmed to
  if ( chaser_cycle && reset == 1 ) {
    cur_x[0] = 0;
    cur_y[0] = 0;
    cur_x[1] = 0;
    cur_y[1] = 0;
    *random_type = random(2, 9);
    if ( *random_type == 3 || *random_type == 7 ) {
      cur_x[0] = 31;
      cur_y[0] = 15;
    }
  }

  // read NES data and use to alter animation
  up = bitRead(nes_state1, 4);
  down = bitRead(nes_state1, 5);
  left = bitRead(nes_state1, 6);
  right = bitRead(nes_state1, 7);
  a = bitRead(nes_state1, 0);
  b = bitRead(nes_state1, 1);
  sel = bitRead(nes_state1, 2);
  start = bitRead(nes_state1, 3);

  // speed up / slow down animation w/ a/b
  if ( a == 0 ) {
    *animation_interval = *animation_interval - 5;
    *animation_interval = constrain(*animation_interval, 5, 500);
  }

  if ( b == 0 ) {
    *animation_interval = *animation_interval + 5;
    *animation_interval = constrain(*animation_interval, 5, 500);
  }

  // start button resets things
  if ( start == 0 ) {
    clear_all(0, ledCount, display_mem);
    cur_x[0] = 0;
    cur_y[0] = 0;
    cur_x[1] = 0;
    cur_y[1] = 0;
    fade_color[0] = 1;
  }

  // select button increments color and moves LED back to start of array, also randomizes type
  if ( sel == 0 ) {
    clear_all(0, ledCount, display_mem);
    cur_x[0] = 0;
    cur_y[0] = 0;
    cur_x[1] = 0;
    cur_y[1] = 0;
    fade_color[0]++;
    fade_color[1]++;
    if ( fade_color[0] > 511 ) fade_color[0] = 1;
    if ( fade_color[1] > 511 ) fade_color[1] = 1;
    *random_type = random(0, 9);
	*random_type = 8;
    if ( *random_type == 3 || *random_type == 7 ) {
      cur_x[0] = 31;
      cur_y[0] = 15;
    }
  }

  // up,down,left,right used to select mode of chaser
  if ( up == 0 ) {
    clear_all(0, ledCount, display_mem);
    cur_x[0] = 0;
    cur_y[0] = 0;
    cur_x[1] = 0;
    cur_y[1] = 0;
    *random_type = 0;
  }

  if ( down == 0 ) {
    clear_all(0, ledCount, display_mem);
    cur_x[0] = 0;
    cur_y[0] = 0;
    cur_x[1] = 0;
    cur_y[1] = 0;
    *random_type = 1;
  }

  if ( left == 0 ) {
    clear_all(0, ledCount, display_mem);
    cur_x[0] = 0;
    cur_y[0] = 0;
    cur_x[1] = 0;
    cur_y[1] = 0;
    *random_type = 2;
  }

  if ( right == 0 ) {
    clear_all(0, ledCount, display_mem);
    cur_x[0] = 0;
    cur_y[0] = 0;
    cur_x[1] = 0;
    cur_y[1] = 0;
    *random_type = 3;
  }


  //lastly check if current time is greater than color cycle interval and if so, change the chaser motion at random
  if ( *time_chaser_cycle == 1 && millis() - *previousSenseMillis > fade_color_interval ) {
    clear_all(0, ledCount, display_mem);
    cur_x[0] = 0;
    cur_y[0] = 0;
    cur_x[1] = 0;
    cur_y[1] = 0;
    *random_type = random(0, 9);
    if ( *random_type == 3 || *random_type == 7 ) {
      cur_x[0] = 31;
      cur_y[0] = 15;
    }
    *previousSenseMillis = millis();
  }
}


// #################################################################
// 9: light sense animation, start with the basics..

void light_sense( byte type, int16_t *display_mem, int16_t ledCount, int16_t *ball_color, byte *location_x, byte *location_y, volatile byte *ir_sense_data, byte *ir_cal_on, byte *ir_cal_off, byte *ir_node_state, int16_t *sense_color_on, int16_t *sense_color_off, unsigned long *previousSenseMillis, int16_t *led_mask, byte *ir_detected, byte *ir_fade_count, byte fade_timer, int16_t fade_color_interval, byte debug  ) {

  // type0 = fade down led on off
  // type1 = immediate led switch off, no fade
  // type2 = immediate led switch off, no fade, color opposites
  // type3 = draw ball and move it when IR detected where it is, use led mask for hit detection
  // type4 = ir as mask
  // type5 = simple ir detect for other actions

  // for type 3, draw ball, re-use ball location array[0] from bounce animation
  if ( type == 3) {
    int16_t ball_loc;
    for ( byte i = 0; i < 1; i++ ) {
      //draw 12-pixel ball based on current location
      ball_loc = location_x[i] + (32 * location_y[i]);

      //draw "center" of ball
      display_mem[ball_loc] = ball_color[i];
      display_mem[ball_loc + 1] = ball_color[i];
      display_mem[ball_loc - 32] = ball_color[i];
      display_mem[ball_loc - 31] = ball_color[i];

      //draw out edge of ball for all 3 shapes
      display_mem[ball_loc - 1] =  ball_color[i];
      display_mem[ball_loc - 33] = ball_color[i];
      display_mem[ball_loc + 2]  = ball_color[i];
      display_mem[ball_loc + 32] = ball_color[i];
      display_mem[ball_loc + 33] = ball_color[i];
      display_mem[ball_loc - 30] = ball_color[i];
      display_mem[ball_loc - 64] = ball_color[i];
      display_mem[ball_loc - 63] = ball_color[i];
    }
  }

  // each LED has 3 states:  0 = off, 1 = on, 2 = fading down  use state of IR sensor to move between states
  byte senseStateOn = 0;
  byte senseStateOff = 0;
  int16_t term0 = 0;

  // loop through saved ir sense data and check if each node is over threshold, if so, light up node
  for ( byte i = 0; i <= 127; i++ ) {

    term0 = (64 * (i / 16)) + (2 * (i % 16));
    senseStateOn = 0;
    senseStateOff = 0;

    if ( ir_sense_data[i] > ir_cal_on[i] ) {
      senseStateOn = 1;
	
	  if ( debug == 3 ) {
        Serial.print("node: ");
        Serial.print(i);
		Serial.print(" cal_on: ");
        Serial.print(ir_cal_on[i]);
        Serial.print(",  sense data: ");
        Serial.println(ir_sense_data[i]);
      }

    } else if ( ir_sense_data[i] < ir_cal_off[i] ) {
      senseStateOff = 1;
    }

    // in state 0, LED off state
    if ( ir_node_state[i] == 0 && senseStateOn == 1) {     // if IR sensor on and LED off, turn on and move to on state

      ir_node_state[i] = 1;

      if ( type < 3 ) {
        //sense_color_on = random(1,512);
        //sense_color_on = 7;
        display_mem[term0] = *sense_color_on;
        display_mem[term0 + 1] = *sense_color_on;
        display_mem[term0 + 32] = *sense_color_on;
        display_mem[term0 + 33] = *sense_color_on;
      } else if ( type == 3 || type == 4 ) {
        led_mask[term0] = 0x0FFF;
        led_mask[term0 + 1] = 0x0FFF;
        led_mask[term0 + 32] = 0xFFF;
        led_mask[term0 + 33] = 0xFFF;
      } else if ( type == 5 ) {
        *ir_detected = 1;
      }


      // in state 1, LED on state
    } else if ( ir_node_state[i] == 1 && senseStateOff == 1  ) {    //LED is on but IR sensor is off now, turn off and move back to off state

      if ( type == 0 ) {
        ir_node_state[i] = 2;
        ir_fade_count[i] = fade_timer;
      } else if ( type == 1 || type == 2 ) {
        ir_node_state[i] = 0;
        if ( type == 1 ) *sense_color_off = 0;
        display_mem[term0] = *sense_color_off;
        display_mem[term0 + 1] = *sense_color_off;
        display_mem[term0 + 32] = *sense_color_off;
        display_mem[term0 + 33] = *sense_color_off;
      } else if ( type == 3 || type == 4 ) {
        ir_node_state[i] = 0;
        led_mask[term0] = 0;
        led_mask[term0 + 1] = 0;
        led_mask[term0 + 32] = 0;
        led_mask[term0 + 33] = 0;
      } else if ( type == 4 ) {
        ir_node_state[i] = 0;
      }

      // in state2, LED fade down state
    } else if ( ir_node_state[i] == 2 ) {

      // first check and decrement fade counter, then go from there
      if ( ir_fade_count[i] > 0 ) {
        ir_fade_count[i]--;
      } else if ( ir_fade_count[i] == 0 ) {

        ir_fade_count[i] = fade_timer;

        // fade down the LED one increment for each color
        fade_down(term0, 1, display_mem);
        fade_down(term0 + 1, 1, display_mem);
        fade_down(term0 + 32, 1, display_mem);
        fade_down(term0 + 33, 1, display_mem);


        // once we've faded the node down to zero, jump out of fade state and back to off state
        if (display_mem[term0] == 0 ) {
          ir_fade_count[i] = fade_timer;
          ir_node_state[i] = 0;
        }
      }
    }
  }

  // for type 2, draw ball, re-use ball location array[0] from bounce animation
  if ( type == 3 && millis() - *previousSenseMillis > 100 ) {
    int16_t ball_loc;
    for ( byte i = 0; i < 1; i++ ) {
      //draw 12-pixel ball based on current location
      ball_loc = location_x[i] + (32 * location_y[i]);

      if ( led_mask[ball_loc] == 0x0FFF ) {

        // erase current ball location, pick a new one, and move it there
        //"center" of ball
        display_mem[ball_loc] = 0;
        display_mem[ball_loc + 1] = 0;
        display_mem[ball_loc - 32] = 0;
        display_mem[ball_loc - 31] = 0;

        //clear edge of ball
        display_mem[ball_loc - 1] =  0;
        display_mem[ball_loc - 33] = 0;
        display_mem[ball_loc + 2]  = 0;
        display_mem[ball_loc + 32] = 0;
        display_mem[ball_loc + 33] = 0;
        display_mem[ball_loc - 30] = 0;
        display_mem[ball_loc - 64] = 0;
        display_mem[ball_loc - 63] = 0;

        //set new random location for ball, within bounds and ensuring it lands on a valid IR node
        location_x[i] = 2 * random(1, 15);
        location_y[i] = 2 * random(1, 7) + 1;
        led_mask[ball_loc] = 0;
        ball_loc = location_x[i] + (32 * location_y[i]);
        ball_color[i] = random(1, 512);

        // draw ball in new location now
        display_mem[ball_loc] = ball_color[i];
        display_mem[ball_loc + 1] = ball_color[i];
        display_mem[ball_loc - 32] = ball_color[i];
        display_mem[ball_loc - 31] = ball_color[i];

        //draw out edge of ball for all 3 shapes
        display_mem[ball_loc - 1] =  ball_color[i];
        display_mem[ball_loc - 33] = ball_color[i];
        display_mem[ball_loc + 2]  = ball_color[i];
        display_mem[ball_loc + 32] = ball_color[i];
        display_mem[ball_loc + 33] = ball_color[i];
        display_mem[ball_loc - 30] = ball_color[i];
        display_mem[ball_loc - 64] = ball_color[i];
        display_mem[ball_loc - 63] = ball_color[i];
      }

    }

    *previousSenseMillis = millis();
  }


  //lastly check if current time is greater than color cycle interval and if so, change fade color
  if ( type < 3 && millis() - *previousSenseMillis > fade_color_interval ) {
    *sense_color_on = random(1, 512);
    if ( type == 2 ) {
      //sense_color_off = 64*(7-(sense_color_on / 64)) + 8*(7-((sense_color_on % 64) / 8 )) + (7-(sense_color_on % 8 ));
      *sense_color_off = opposite_color(*sense_color_on);
      clear_all(*sense_color_off, ledCount, display_mem);
    }

    if ( debug == 1 ) {
      Serial.print("sense_on: ");
      Serial.print(*sense_color_on);
      Serial.print("   sense_on RGB: ");
      Serial.print(*sense_color_on / 64);
      Serial.print(" ");
      Serial.print((*sense_color_on % 64) / 8 );
      Serial.print("  ");
      Serial.println(*sense_color_on % 8);
    }

    *previousSenseMillis = millis();
  }
}


// #################################################################
// 10: scroll text message from right to left
void text_scroll(byte type, String text, byte text_size, byte color_type, int16_t *display_mem, int16_t ledCount, int16_t text_color, int16_t *character_origin, byte ir_detected, byte debug ) {

  /* type options
     type = 0, ir activated text scroll
     type = 1, normal static text
     type = 2, changing text ( such as counter or timer )
     
     color types
     color_type = 0: normal one color, blank backround
     color_type = 1: inverse color
     color_type = 2: alternate colors for each letter
 
  */

  if ( debug == 2 ) {
    Serial.print("sizeof text is ");
    //Serial.println(sizeof(&text));
    Serial.println(text.length());
  }

  int16_t fg_color = text_color;
  int16_t bg_color = 0;

  // inverse color
  if ( color_type == 1 ) {
    fg_color = 0;
    bg_color = text_color;
    clear_all(bg_color, ledCount, display_mem);
  }


  if ( type > 0 || ( type == 0 && ir_detected == 1 ) ) {
    for ( int16_t i = 0; i < text_size - 1; i++ ) {

      if ( debug == 2 ) {
        Serial.print("i = ");
        Serial.println(i);
      }

	  /*
      if ( color_type == 2 ) {     
        if ( i % 2 == 0 ) {
          fg_color = 448;
        } else {
          fg_color = 56;
        } 
      }
	  */
	  
	  if ( color_type == 2 ) {     
        if ( i % 3 == 0 ) {
			fg_color = 448;
        } else if ( i % 3 == 1 ) {
			fg_color = 511;
        } else {
			fg_color = 7;
		}
      }

      //if (letter_lookup(text[i]) == 62 ) fg_color = 448;
      
      // clear previous char location
      if ( character_origin[i] < 128 && character_origin[i] > 91 ) {
        // 2 differnet ways to clear last character, type > 1 means string is changing so we need to blank whole letter, otherwise, just static led positoins for the current char which is not changing
        if ( type  > 1 ) {
          blankchar(character_origin[i], bg_color, display_mem);
        } else {
          printchar(character_origin[i], bg_color, text[i], display_mem);
        }
      }

      character_origin[i]--;

      if ( character_origin[i] < 128 && character_origin[i] > 91 ) {
        printchar(character_origin[i], fg_color, text[i], display_mem);
      }

      // when charcater origin reaches low enough value such that entire message has scrolled off screen, reset it's value
      if (character_origin[i] == (96 - 6 * (text_size - 1))) character_origin[i] = 160;

      if ( type == 0 && i == (text_size) - 2 && character_origin[i] == 90 ) {
        ir_detected = 0;
        for ( int16_t i = 0; i < text_size; i++ ) {
          character_origin[i] = 130 + 6 * i;
        }
        /*
        Serial.print("ir off at i = ");
        Serial.println(i);
        Serial.print("char origin = ");
        Serial.println(character_origin[i]);
        */
      }
    }
  }

  if ( debug == 2 ) {
    /*
    for ( byte j=0; j < 2; j++ ) {
      for ( byte k=0; k<35; k++ ) {
        Serial.println(pgm_read_word_near(&text_lookup[j][k]));
      }
    }
    */
    //Serial.println(ir_detected);
  }
}

// #################################################################
// 11: sprite animate:  animate sprites read from program memory, including control by NES controller
void sprite_animate (byte type, int16_t frametime, int16_t *display_mem, byte nes_state1, int16_t ledCount, byte *sprite_mode, unsigned long *mode_time ) {

  unsigned long frame_start;
  byte up, down, left, right,a,b;

  if ( type == 0 ) {
    // grab frame 1 from memory, display it
    for (int16_t i = 0; i < ledCount; i++ ) {
      if ( *sprite_mode == 0 ) display_mem[i] = pgm_read_word_near( &link_down_1[i] );
      if ( *sprite_mode == 1 ) display_mem[i] = pgm_read_word_near( &link_right_1[i] );
      if ( *sprite_mode == 2 ) display_mem[i] = pgm_read_word_near( &link_left_1[i] );
      if ( *sprite_mode == 3 ) display_mem[i] = pgm_read_word_near( &link_up_1[i] );
	  if ( *sprite_mode == 4 ) display_mem[i] = pgm_read_word_near( &flag2[i] );
	
    }
    frame_start = millis();
    while ( millis() - frame_start < frametime ) {
      // do nothing, just wait
    }

    // grab frame2 from memory, display it
    for (int16_t i = 0; i < ledCount; i++ ) {
      if ( *sprite_mode == 0 ) display_mem[i] = pgm_read_word_near( &link_down_2[i] );
      if ( *sprite_mode == 1 ) display_mem[i] = pgm_read_word_near( &link_right_2[i] );
      if ( *sprite_mode == 2 ) display_mem[i] = pgm_read_word_near( &link_left_2[i] );
      if ( *sprite_mode == 3 ) display_mem[i] = pgm_read_word_near( &link_up_2[i] );
	  if ( *sprite_mode == 4 ) display_mem[i] = pgm_read_word_near( &flag3[i] );
	  
    }

    frame_start = millis();
    while ( millis() - frame_start < frametime ) {
      // do nothing, just wait
    }
		

  }

  // grab nes button state to see if we should change the animation
  up = bitRead(nes_state1, 4);
  down = bitRead(nes_state1, 5);
  left = bitRead(nes_state1, 6);
  right = bitRead(nes_state1, 7);
  a = bitRead(nes_state1, 0);
  b = bitRead(nes_state1, 1);
  
  if ( down == 0 )  *sprite_mode = 0;
  if ( right == 0 ) *sprite_mode = 1;
  if ( left == 0 )  *sprite_mode = 2;
  if ( up == 0 )    *sprite_mode = 3;
  if ( a == 0 )     *sprite_mode = 4;
  if ( b == 0 )     *sprite_mode = 5;
  


  //can also use a timer to randomly switch direction
  if ( millis() - *mode_time > 2200 ) {
    //*sprite_mode = random(4, 6);
	*sprite_mode = 4;
    *mode_time = millis();
  }
}

// #################################################################
// 12: stopwatch
void stopwatch(byte type, long *stopwatch_time, long *stopwatch_start_time, long *prev_stopwatch_time, byte *stopwatch_running, int16_t *display_mem, byte nes_state1, byte debug) {

  // stopwatch_time = millis();
  long newtime = 0;
  byte minute1_val = 9;
  byte minute0_val = 9;
  byte second1_val = 9;
  byte second0_val = 9;
  byte ms_val = 9;

  byte minute1_origin = 97;
  byte minute0_origin = 103;
  byte second1_origin = 109;
  byte second0_origin = 115;
  byte millisecond_origin = 121;

  byte a, b, up, sel, start = 1;

  // if stopwatch is running, accumlate time
  if ( *stopwatch_running == 1 ) {
    *stopwatch_time = *prev_stopwatch_time + ( millis() - *stopwatch_start_time );
  }

  // if time is under display limits, calc minute, second, and ms values to print
  if ( *stopwatch_time < 5999900 ) {
    minute1_val = *stopwatch_time / 600000;
    newtime = *stopwatch_time - (600000 * minute1_val);
    minute0_val = newtime / 60000;
    newtime = newtime - ( 60000 * minute0_val);
    second1_val = newtime / 10000;
    newtime = newtime - ( 10000 * second1_val);
    second0_val = newtime / 1000;
    newtime = newtime - ( 1000 * second0_val);
    ms_val = newtime / 100;
  }

  // clear previous time
  blankchar(minute1_origin, 0, display_mem);
  blankchar(minute0_origin, 0, display_mem);
  blankchar(second1_origin, 0, display_mem);
  blankchar(second0_origin, 0, display_mem);
  blankchar(millisecond_origin, 0, display_mem);

  //draw punctuation
  display_mem[172] = 128;
  display_mem[268] = 128;
  display_mem[312] = 128;

  // print time value to screen
  String number = String(minute1_val);
  printchar(minute1_origin, 7, number[0], display_mem);
  number = String(minute0_val);
  printchar(minute0_origin, 7, number[0], display_mem);
  number = String(second1_val);
  printchar(second1_origin, 7, number[0], display_mem);
  number = String(second0_val);
  printchar(second0_origin, 7, number[0], display_mem);
  number = String(ms_val);
  printchar(millisecond_origin, 7, number[0], display_mem);

  // read NES state to control screen state/color used
  sel = bitRead(nes_state1, 2);
  start = bitRead(nes_state1, 3);
  up = bitRead(nes_state1, 4);
  a = bitRead(nes_state1, 0);
  b = bitRead(nes_state1, 1);

  // a button starts / restarts stopwatch
  if ( a == 0 ) {
    if ( *stopwatch_running == 0 ) {
      *stopwatch_running = 1;
      *stopwatch_start_time = millis();
    }
  }

  // b button stops stopwatch
  if ( b == 0 ) {
    if ( *stopwatch_running == 1 ) {
      *stopwatch_running = 0;
      *prev_stopwatch_time = *stopwatch_time;
    }
  }

  // select button clears ( and stops ) stopwatch,  basically a reset
  if ( sel == 0 ) {
    *stopwatch_running = 0;
    *stopwatch_time = 0;
    *stopwatch_start_time = 0;
    *prev_stopwatch_time = 0;
  }

  if ( debug == 1 ) {
    String time_string = String("stopwatch_running: ");
    time_string = String(time_string + *stopwatch_running + "  millis: " + millis() + "  stopwatch_time: " + *stopwatch_time + "  stopwatch_start_time:" + *stopwatch_start_time + "  prev_stopwatch_time: " + *prev_stopwatch_time );
    Serial.println(time_string);
  }

}

// #################################################################
// 13: manual control over each led brightness via serial communication from pc
void serial_control(int16_t *display_mem, int16_t *ball_color, int16_t ledCount, int16_t nes_location) {

  char serial_data = 0;

  // if there is serial data in the buffer, grab it and interpret
  while (Serial.available() > 0 ) {
    display_mem[511] = 0;
    serial_data = Serial.read();  //returns input as ascii character

    //corresponds to left mouse click
    if ( serial_data == 'A' ) {  // since serial port read byte as ascii, compare to ascii
      ball_color[0] = 7;
      ball_color[1] = 448;
      clear_all(0, ledCount, display_mem);

      // right mouse click
    } else if (serial_data == 'B') {
      ball_color[0] = 448;
      ball_color[1] = 7;
      clear_all(0, ledCount, display_mem);

      // mouse position string
    } else if (serial_data == 'P' ) {

      int16_t pos = 600;

      // grab position integer and update display based on current mouse position
      while (Serial.available() > 0) {
        pos = Serial.parseInt();

        if (pos < 512) {
          display_mem[nes_location] = 0;     // clear previous location, re-use nes_location for this purpose

          // draw x and y cursors across whole screen with single led in center for position
          for (byte i = 0; i < 32; i++) {
            display_mem[(32 * (nes_location / 32)) + i] = 0;

            if (i < 16 ) {
              display_mem[(32 * i) + (nes_location % 32)] = 0;
              display_mem[(32 * i) + (pos % 32)] = ball_color[1];
            }
            display_mem[(32 * (pos / 32)) + i ] = ball_color[1];
          }

          // now set current location color and save location as previous location
          display_mem[pos] = ball_color[0];
          nes_location = pos;
        }
      }
    }
  }

  display_mem[511] = 7;  // for serial communication debug

}

// #################################################################
// 14: tetris, accessible by easter egg code only
void tetris(byte *partloc_x, byte *partloc_y, byte *part_type, byte *next_part, byte *part_o, unsigned long *previousUpdate, int16_t *fall_rate, byte *tetris_stack, byte *pauseState, byte *previousPauseButtonState, byte *gameOver, int16_t *score, int16_t *display_mem, int16_t ledCount, byte nes_state1, unsigned long *nesButtonMillis, byte debug) {

  // grab current NES control button state
  byte up, down, left, right, a, b, sel = 1;
  byte prev_x, prev_y, prev_o;
  byte leftstop, rightstop, bottomstop, topstop = 0;

  prev_x = *partloc_x;
  prev_y = *partloc_y;
  prev_o = *part_o;

  if ( debug == 1 ) up = bitRead(nes_state1, 4);
  down = bitRead(nes_state1, 5);
  left = bitRead(nes_state1, 6);
  right = bitRead(nes_state1, 7);
  a = bitRead(nes_state1, 0);
  b = bitRead(nes_state1, 1);
  sel = bitRead(nes_state1, 2);

  // set move bounds for current part
  setbounds(*part_type, *part_o, &leftstop, &rightstop, &topstop, &bottomstop);

  // if part's fall_rate has not been met yet, controller movement will dictate new part location
  if ( millis() - *previousUpdate > *fall_rate ) {

    // if part is at bottom, generate new part at top,  if drop was not successful, check if y location still equals starting height, if so, it's game over jonny
    if ( droppart(bottomstop, *part_type, *part_o, partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
      // part drop was successful, do nothing

    } else {

      // part drop was not successful for whatever reason,  need to check if part is stuck at top, and in conflict w/ current board
      if ( *partloc_y == partstart_y && !(check_stack(*part_type, *part_o, *partloc_y, *partloc_x, tetris_stack, debug, ledCount)) ) {

        // game over at this point, print score
        *gameOver = 1;

        // call printdigit function w/ digit location and value
        printdigit(0, *score / 100, display_mem );                  // print hundreds digit
        printdigit(1, ( *score % 100 ) / 10, display_mem );         // print tens digit
        printdigit(2, *score % 10, display_mem );                   // print ones digit

        // draw top of game space
        for ( byte i = 0; i < 16; i++ ) {
          display_mem[(partstart_y - 2) + 32 * i] = 448;
        }

      } else {

        // part was at bottom or could go no furhter, move next part to current part and generate new next_part
        update_stack(*part_type, *part_o, *partloc_y, *partloc_x, tetris_stack, debug, ledCount);   // NOTE!!!! partloc_y sent in for x_location and vice versa!!!!!!
        *part_type = *next_part;
        *part_o = 0;
        *partloc_x = partstart_x;
        *partloc_y = partstart_y;
        prev_x = *partloc_x;
        prev_y = *partloc_y;
        prev_o = *part_o;
        *next_part = random(0, 7);

        // once that is done, check that any rows have been completed, clear and drop the stack if they have been
        checkrows(partstart_y, tetris_stack, display_mem, score, fall_rate);
      }
    }

    *previousUpdate = millis();

  } else {
    // part timer isn't up, move part based on NES input ( if any )
    movepart(*part_type, part_o, &prev_o, partloc_x, partloc_y, up, down, left, right, a, b, topstop, bottomstop, rightstop, leftstop, tetris_stack, debug, ledCount, nesButtonMillis);
  }

  // draw current part type  **NOTE, pass y part location in for x array position and vice versa when drawing part
  drawpart(*part_type, prev_o, prev_y, prev_x, 0, display_mem, partcolor);
  drawpart(*part_type, *part_o, *partloc_y, *partloc_x, 0xFFFF, display_mem, partcolor);
  drawpart(*part_type, 0, 2, 2, 0, display_mem, partcolor );
  drawpart(*next_part, 0, 2, 2, 0xFFFF, display_mem, partcolor);

  // for test, randomize current part
  if ( sel == 0 ) {
    *part_type = random(0, 7);
    *part_o = 0;
    *partloc_x = partstart_x;
    *partloc_y = partstart_y;
    *score = *score + 1;
  }
}


// #################################################################
// 14: snake
void snake(byte *players, int16_t *score, int16_t *display_mem, int16_t ledCount, byte nes_state1, byte nes_state0, int16_t *snake1, int16_t *snake2, byte *dir1, byte *dir2, byte *dead1, byte *dead2, int16_t *food1, int16_t *food2, byte *foodmode, const byte snakeLength, int16_t *animation_interval, byte debug) {

  // grab nes input states to determine snake movement
  byte up1, down1, left1, right1, a1, b1, sel1, start1 = 1;
  byte up2, down2, left2, right2, a2, b2, sel2, start2 = 1;
  
  up1 = bitRead(nes_state1, 4);
  down1 = bitRead(nes_state1, 5);
  left1 = bitRead(nes_state1, 6);
  right1 = bitRead(nes_state1, 7);
  a1 = bitRead(nes_state1, 0);
  b1 = bitRead(nes_state1, 1);
  sel1 = bitRead(nes_state1, 2);
  start1 = bitRead(nes_state1, 3);
  
  up2 = bitRead(nes_state0, 4);
  down2 = bitRead(nes_state0, 5);
  left2 = bitRead(nes_state0, 6);
  right2 = bitRead(nes_state0, 7);
  a2 = bitRead(nes_state0, 0);
  b2 = bitRead(nes_state0, 1);
  sel2 = bitRead(nes_state0, 2);
  start2 = bitRead(nes_state0, 3);
  
  
  // change snake dir based on controller input
  if ( up1 == 0 ) {
    if ( *dir1 != 3 ) *dir1 = 0;
  }
  if ( down1 == 0 ) {
    if ( *dir1 != 0 ) *dir1 = 3;
  }
  if ( left1 == 0 ) {
    if ( *dir1 != 2 ) *dir1 = 1;
  }
  if ( right1 == 0 ) {
    if ( *dir1 != 1 ) *dir1 = 2;
  }
  
  if ( up2 == 0 ) {
    if ( *dir2 != 3 ) *dir2 = 0;
  }
  if ( down2 == 0 ) {
    if ( *dir2 != 0 ) *dir2 = 3;
  }
  if ( left2 == 0 ) {
    if ( *dir2 != 2 ) *dir2 = 1;
  }
  if ( right2 == 0 ) {
    if ( *dir2 != 1 ) *dir2 = 2;
  }
    
  if ( *dead1 == 0 && *dead2 == 0 ) {  
    updateSnakes(snake1, *dir1, snake2, *dir2, snakeLength, debug);
    checkSnakes(snake1, snake2, snakeLength, dead1, dead2, food1, food2, *foodmode, *players);
    drawSnakes(snake1, snake2, display_mem, ledCount, snakeLength, *food1, *food2, *foodmode);
  } 
  
  
  // check for snake death.  initially just light the border for the snake who won, eventually add cool fade/flash animation for loser/winner
  if ( *dead1 == 1 ) {
	 
	 for ( byte i=0; i < snakeLength; i++ ) {
		if ( snake1[i] < 999 ) fade_down(snake1[i], 1, display_mem);
	 }
     draw_row(0, 7, display_mem, 32 );  
     draw_row(15, 7, display_mem, 32);  
     draw_col(0, 7, display_mem, 16);  
     draw_col(31, 7, display_mem, 16);  
  }
  if ( *dead2 == 1 ) {
	 
	 for ( byte i=0; i < snakeLength; i++ ) {
		if ( snake2[i] < 999 ) fade_down(snake2[i], 1, display_mem);
	 }
     draw_row(0, 448, display_mem, 32 );  
     draw_row(15, 448, display_mem, 32);  
     draw_col(0, 448, display_mem, 16);  
     draw_col(31, 448, display_mem, 16);  
  }
  if ( *dead1 == 1 && *dead2 == 1 ) {
	 for ( byte i=0; i < snakeLength; i++ ) {
		if ( snake1[i] < 999 ) fade_down(snake1[i], 1, display_mem); 
		if ( snake2[i] < 999 ) fade_down(snake2[i], 1, display_mem);
	 }
     draw_row(0, 56, display_mem, 32 );  
     draw_row(15, 56, display_mem, 32);  
     draw_col(0, 56, display_mem, 16);  
     draw_col(31, 56, display_mem, 16);  
  }
  
  
 if ( a1 == 0 ) {
    *animation_interval = *animation_interval - 5;
    *animation_interval = constrain(*animation_interval, 10, 250);
  }

  if ( b1 == 0 ) {
    *animation_interval = *animation_interval + 5;
    *animation_interval = constrain(*animation_interval, 10, 250);
  }
  
  if ( start1 == 0 ) {  // re-init game to default starting positions
    
    for ( byte i=0; i < snakeLength; i++ ) {
        snake1[i] = 999;
        snake2[i] = 999;
      }
      
      // set initial snake positions
      snake1[0] = 263;  
      snake1[1] = 262;
      snake1[2] = 261;
      snake1[3] = 260;
          
      if ( *players == 2 ) {
        snake2[0] = 312;  
        snake2[1] = 313;
        snake2[2] = 314;
        snake2[3] = 315;
      }
      
      newFood(snake1, snake2, snakeLength, food1, food2, *foodmode, 2 );  
      *dir1 = 2;
      *dir2 = 1;
      *dead1 = 0;
      *dead2 = 0;
      *animation_interval = 80;
  } 
  
  if ( sel1 == 0 ) {
    *players = *players + 1;
    if ( *players == 3 ) *players = 1;
    
    *foodmode = random(0,2);
  }
}



/*
    Any live cell with fewer than two live neighbours dies, as if caused by under-population.
    Any live cell with two or three live neighbours lives on to the next generation.
    Any live cell with more than three live neighbours dies, as if by overcrowding.
    Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
*/

// #################################################################
// 16: life
void life(byte *type, int *iteration, int16_t *display_mem, int16_t ledCount, byte *gamestate, byte nes_state1, int16_t *animation_interval, int16_t *wave_color, byte debug) {

  // grab nes input states 
  byte up1, down1, left1, right1, a1, b1, sel1, start1 = 1;
  
  up1 = bitRead(nes_state1, 4);
  down1 = bitRead(nes_state1, 5);
  left1 = bitRead(nes_state1, 6);
  right1 = bitRead(nes_state1, 7);
  a1 = bitRead(nes_state1, 0);
  b1 = bitRead(nes_state1, 1);
  sel1 = bitRead(nes_state1, 2);
  start1 = bitRead(nes_state1, 3);

  byte newState[512];  // temp array to hold the newly calculated state for the current location
  byte currentRow = 0;
  byte currentCol = 0;
  byte adjacentAlive = 0;

  // loop though gamestate and count # of adjacent cells to the current cell that are alive
  for ( int16_t i=0; i<ledCount; i++) {

    // calc row/column
    currentRow = i/32;
    currentCol = i%32;
	adjacentAlive = 0;
	
	if ( debug == 2 ) {
		
		String debugString = "led: ";
		debugString = String(debugString + i + " R/C: " + currentRow + "/" + currentCol );
		Serial.println(debugString);
	}
	

    // check all 8 possible adjacent cells for life and count # of alive adjacent to current cell
    if ( currentRow > 0 && currentCol > 0 ) {
      if ( gamestate[i-33] == 1 ) adjacentAlive++;  
    }

    if ( currentRow > 0 ) {
      if ( gamestate[i-32] == 1 ) adjacentAlive++;
    }

    if ( currentRow > 0 && currentCol < 31 ) {
      if ( gamestate[i-31] == 1 ) adjacentAlive++;
    }

    if ( currentCol > 0 ) {
      if ( gamestate[i-1] == 1 ) adjacentAlive++;
    }

    if ( currentCol < 31 ) {
      if ( gamestate[i+1] == 1 ) adjacentAlive++;
    }

    if ( currentRow < 15 && currentCol > 0 ) {
      if ( gamestate[i+31] == 1 ) adjacentAlive++;
    }

    if ( currentRow < 15 ) {
      if ( gamestate[i+32] == 1 ) adjacentAlive++;
    }

    if ( currentRow < 15 && currentCol < 31 ) {
      if ( gamestate[i+33] == 1 ) adjacentAlive++;
    }

    // once adjacent number of alive cells has been determined, update the newState array with the new state of the current cell
    newState[i] = 0;

    if (gamestate[i] == 0 ) {                     // current cell is dead, if has exactly 3 alive neighbours, reproduce
      if ( adjacentAlive == 3 ) newState[i] = 1;
    } else {                                      // current cell is alive, determine next state based on neighbours

    newState[i] = 1;
    if ( adjacentAlive < 2 || adjacentAlive > 3 ) newState[i] = 0;

    }

  }

  // now that newState of array has been determined, copy it over to the current gamestate and then update the display_mem
  for ( int16_t i = 0; i < ledCount; i++ ){
    gamestate[i] = newState[i];
		
	if ( debug == 2 ) {
		String debugString = "gamestate[";
		debugString = String(debugString + i + "]: " + gamestate[i]);
		Serial.println(debugString);
	
	}
	
    display_mem[i] = 0;
	
	if (*type == 0 || *type == 1 ) {
		if ( gamestate[i] == 1 ) display_mem[i] = wave_color[0];
	} else if ( *type == 2 ) {
		if ( gamestate[i] == 1 ) display_mem[i] = random(0,512);
	}
  }
  
  if ( *type == 1 ) wave_color[0] = random(1,512);


  // increment iteration
  *iteration = *iteration + 1;
  
  // reset after 250 iterations.  usually at this point the board is in a static state
  if ( *iteration > 250 ) {
	init_life(gamestate, iteration, ledCount, wave_color);
  }

  //lastly check NES state for button presses

  if ( a1 == 0 ) {
    *animation_interval = *animation_interval - 5;
    *animation_interval = constrain(*animation_interval, 10, 250);
  }

  if ( b1 == 0 ) {
    *animation_interval = *animation_interval + 5;
    *animation_interval = constrain(*animation_interval, 10, 250);
  }

  if ( start1 == 0 ) {
    init_life(gamestate, iteration, ledCount, wave_color);
  }
  
  if ( sel1 == 0 ) {
	*type = random(0,3);
  }
  
  if ( up1 == 0 ) {
	wave_color[0] = random(1,512);	
  }

}


// ************************
// 17: maze_solve
void maze_solve(int16_t *current_cell, byte *endnode, byte *maze_nodes, byte *maze_walls, byte *nodestack, char *stack_index, int16_t *display_mem, byte *wall_update, int16_t *wave_color, byte nes_state1, int16_t *animation_interval, unsigned long *reset_wait, byte *type, byte *next_cell, byte debug ) {
	
	 // grab nes input states 
	byte up1, down1, left1, right1, a1, b1, sel1, start1 = 1;
  
	up1 = bitRead(nes_state1, 4);
	down1 = bitRead(nes_state1, 5);
	left1 = bitRead(nes_state1, 6);
	right1 = bitRead(nes_state1, 7);
	a1 = bitRead(nes_state1, 0);
	b1 = bitRead(nes_state1, 1);
	sel1 = bitRead(nes_state1, 2);
	start1 = bitRead(nes_state1, 3);
	
	
	byte node_row = 0;
	byte node_col = 0;
	byte unvisited = 0;
	//int16_t pathcolor = 256;
	
	if ( *current_cell == *endnode ) {
		// done! do something here?
		
		node_row = *current_cell / 15;
		node_col = *current_cell % 15;
		
		if ( *type == 0 ) {
			//*animation_interval = 50;
		
			for ( int16_t i=0; i<512; i++ ) {
				fade_down(i, 1, display_mem);
			}			
				
			set_wall(*endnode, nodestack[*stack_index-1], wave_color[1], display_mem);
			display_mem[33 + (64*node_row) +(2*node_col)] = wave_color[1];
			display_mem[34 + (64*node_row) + (2*node_col)] = wave_color[1];
			display_mem[32] = wave_color[1];
		
		//draw_row(15, 56, display_mem, 32);
		//draw_col(31, 56, display_mem, 16);
		
		} else if ( *type == 1 ) {
			
			colorchange(wave_color[0], 16, display_mem, 512 );
			display_mem[34 + (64*node_row) + (2*node_col)] = wave_color[1];
			
		}
		
		if ( millis() - *reset_wait > 2000 ) init_maze(maze_nodes, maze_walls, nodestack, stack_index, 512, display_mem, current_cell, wave_color, type, wall_update, debug);

			
		
	} else {   // shit, you mean we actually have to try to solve this thing?
		
		// calc row and column for current node
		node_row = *current_cell / 15;
		node_col = *current_cell % 15;
		byte neighbors[4] = {110,110,110,110};  // 110 indicated no valid or visited neighbor, index 0 = upper, 1 = lower, 2 = left, 3 = right
		
		// first mark the current cell as visited
		maze_nodes[*current_cell] = 0;
			
		// find current cell's neighbors ( no wall between them ) and check if any of them have not yet been visited, if both are true, add it to the neighbors array to be randomly chosen from next
	
		// check upper neighbor
		if ( node_row > 0 ) {
			
			if ( maze_walls[97 + (6*node_col) + node_row] == 0 && maze_nodes[*current_cell - 15] == 1 ) {
				neighbors[0] = *current_cell - 15;
				unvisited++;
			}
		}
		
		// check lower neighbor
		if ( node_row < 6 ) {
			if ( maze_walls[98 + (6*node_col) + node_row] == 0  && maze_nodes[*current_cell + 15] == 1 ) {
				neighbors[1] = *current_cell + 15;
				unvisited++;
			}
		}
		
		// check left neighbor
		if ( node_col > 0 ) {
			if ( maze_walls[*current_cell - (node_row+1)] == 0 && maze_nodes[*current_cell - 1] == 1 ) {
				neighbors[2] = *current_cell - 1;
				unvisited++;
			}
		}
		
		// check right neighbor
		if ( node_col < 14 ) {
			if ( maze_walls[*current_cell - node_row] == 0 && maze_nodes[*current_cell + 1] == 1 ) {
				neighbors[3] = *current_cell + 1;
				unvisited++;
			}
		}
		
		// now randomly select a neighbor cell and if neighbor exists and if it has been checked yet, if it both exists and has not been checked, remove wall between cells recursively 	
		if ( unvisited > 0 ) {
			
			// actual update to new node takes place in 2 steps, first is to grab a new neighbor cell and move to the wall between the 2 cells
			// second step ( next iteration through loop ) is to move the current cell to the new one and light the new node
			if ( *wall_update == 0 ) {
				
				*next_cell = random(0,4);
			
				// we know at least one neighbor is unvisited, keep searching for it
				while( neighbors[*next_cell] == 110 ) {
					*next_cell = random(0,4);
				}
			
				// once found, make neighbor cell the current cell and push previous location to stack, next time through loop this will be the starting point
				
				if ( debug == 1 ) {
					Serial.print("current_cell: ");
					Serial.print(*current_cell);
					Serial.print("  next_cell: ");
					Serial.println(neighbors[*next_cell]);
				}
			
				// light wall between the 2 nodes and move current node to new node and set wall update flag
				set_wall(*current_cell, neighbors[*next_cell], wave_color[1], display_mem);
				//*current_cell = neighbors[*next_cell];			
				*wall_update = 1;
				
			} else {
				
				// update current cell to new neighbor cell, next time through loop this will be the starting point
				// push current cell to stack
				nodestack[*stack_index] = *current_cell;
				*stack_index = *stack_index + 1;
				*current_cell = neighbors[*next_cell];

				// draw current location
				node_row = *current_cell / 15;
				node_col = *current_cell % 15;
				display_mem[33 + (64*node_row) + (2*node_col)] = wave_color[1];
				*wall_update = 0;
				
			}
			
			if ( *current_cell == *endnode ) *reset_wait = millis();
			
				
		} else {
			// no unvisited cells in at current cell location, pop cell from stack and use as next current cell, this will work backwards through the traversed nodes until one with unvisted neighbors is found
			// if I did this correctly, this procedure should result in the algorithm finding a path from start to end node
						
			if ( *stack_index > 0 ) {
				//*stack_index = *stack_index - 1;
				
				if ( *wall_update == 0 ) {
					// clear current location
					node_row = *current_cell / 15;
					node_col = *current_cell % 15;
					display_mem[33 + (64*node_row) + (2*node_col)] = 0;
					*wall_update = 1;
				} else {
					// clear wall between the 2 nodes and move current node to previous node in stack
					*stack_index = *stack_index - 1;
					set_wall(*current_cell, nodestack[*stack_index], 0, display_mem);
					*current_cell = nodestack[*stack_index];
					nodestack[*stack_index] = 110;
					*wall_update = 0;
				}
				
				// clear wall between the 2 nodes and move current node to previous node in stack
				//*stack_index = *stack_index - 1;
				//set_wall(*current_cell, nodestack[*stack_index], 0, display_mem);
				//*current_cell = nodestack[*stack_index];
				//nodestack[*stack_index] = 110;
				
			}	
		}
	}

	// draw current navigation path through maze, for each nonzero (<110) node in the stack light up that node's LED
	for ( byte i=0; i<105; i++ ) {
		
		node_row = nodestack[i] / 15;
		node_col = nodestack[i] % 15;
		
		//if ( *type == 1 ) wave_color[1] = random(1,512);
			
		if ( nodestack[i] != 110 ) {
			display_mem[33 + (64*node_row) + (2*node_col)] = wave_color[1];
			if ( i > 0 ) set_wall(nodestack[i], nodestack[i-1], wave_color[1], display_mem);
		} else {
			display_mem[33 + (64*node_row) + (2*node_col)] = 0;
		}
			
	}
	
	
	// lastly, check NES start for re-init  ( will add timer re-init on done soon
	if ( start1 == 0 ) {
		init_maze(maze_nodes, maze_walls, nodestack, stack_index, 512, display_mem, current_cell, wave_color, type, wall_update, debug);
	}
	
	if ( a1 == 0 ) {
		*animation_interval = *animation_interval - 5;
		*animation_interval = constrain(*animation_interval, 10, 500);
	}

	if ( b1 == 0 ) {
		*animation_interval = *animation_interval + 5;
		*animation_interval = constrain(*animation_interval, 10, 500);
	}
}



// ********************************************************  helpers *************************************************************************
// display hello message at startup/reset
void splash(byte screen, int16_t *display_mem) {

  unsigned long splash_start = millis();
  int16_t splash_color = 63;
  int16_t black = 0;
  int16_t pink = 64 + 7;
  int16_t yellow = (64 * 6) + (8 * 2);
  int16_t red = 64 * 4;
  int16_t white = 511;

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
      // display animation name for tetris
      display_mem[161] = splash_color;
      display_mem[172] = splash_color;
      display_mem[183] = splash_color;
      display_mem[193] = splash_color;
      display_mem[204] = splash_color;
      display_mem[224] = splash_color;
      display_mem[225] = splash_color;
      display_mem[226] = splash_color;
      display_mem[227] = splash_color;
      display_mem[230] = splash_color;
      display_mem[231] = splash_color;
      display_mem[232] = splash_color;
      display_mem[235] = splash_color;
      display_mem[236] = splash_color;
      display_mem[237] = splash_color;
      display_mem[238] = splash_color;
      display_mem[240] = splash_color;
      display_mem[242] = splash_color;
      display_mem[243] = splash_color;
      display_mem[246] = splash_color;
      display_mem[247] = splash_color;
      display_mem[251] = splash_color;
      display_mem[252] = splash_color;
      display_mem[253] = splash_color;
      display_mem[254] = splash_color;
      display_mem[257] = splash_color;
      display_mem[261] = splash_color;
      display_mem[265] = splash_color;
      display_mem[268] = splash_color;
      display_mem[272] = splash_color;
      display_mem[273] = splash_color;
      display_mem[276] = splash_color;
      display_mem[279] = splash_color;
      display_mem[282] = splash_color;
      display_mem[289] = splash_color;
      display_mem[293] = splash_color;
      display_mem[294] = splash_color;
      display_mem[295] = splash_color;
      display_mem[296] = splash_color;
      display_mem[297] = splash_color;
      display_mem[300] = splash_color;
      display_mem[304] = splash_color;
      display_mem[311] = splash_color;
      display_mem[315] = splash_color;
      display_mem[316] = splash_color;
      display_mem[317] = splash_color;
      display_mem[321] = splash_color;
      display_mem[325] = splash_color;
      display_mem[332] = splash_color;
      display_mem[336] = splash_color;
      display_mem[343] = splash_color;
      display_mem[350] = splash_color;
      display_mem[354] = splash_color;
      display_mem[355] = splash_color;
      display_mem[358] = splash_color;
      display_mem[359] = splash_color;
      display_mem[360] = splash_color;
      display_mem[365] = splash_color;
      display_mem[366] = splash_color;
      display_mem[368] = splash_color;
      display_mem[374] = splash_color;
      display_mem[375] = splash_color;
      display_mem[376] = splash_color;
      display_mem[378] = splash_color;
      display_mem[379] = splash_color;
      display_mem[380] = splash_color;
      display_mem[381] = splash_color;
      break;
  }


  while ( millis() - splash_start < 3000 ) {
    // do nothing, just wait
  }

}

// ************************
// set brightness of all LEDs to zero
void clear_all(int16_t color, int16_t ledCount, int16_t *display_mem) {

  int16_t test_color = 6;

  switch (color) {
    case 512:
      for (int16_t i = 0; i < ledCount; i++) {
        if ( i < 128 ) display_mem[i] = 448; //448;
        if ( i > 127 && i < 256 ) display_mem[i] = 56; //56;
        if ( i > 255 & i < 384 ) display_mem[i] = 7; //7;
        if ( i > 383 ) display_mem[i] = 511;
      }

      break;

    case 513:
      // randomize colors for matrix
      for (int16_t i = 0; i < ledCount; i++) {
        display_mem[i] = random(0, 512);
      }
      break;

    case 514:
      // color = array index
      for (int16_t i = 0; i < ledCount; i++) {
        display_mem[i] = i;
      }
      break;

    case 515:
      //rgb checkerboard
      for (int16_t i = 0; i < ledCount; i++ ) {
        if (i % 3 == 0 ) {
          display_mem[i] = 448;
        } else if ( i % 3 == 1 ) {
          display_mem[i] = 56;
        } else if (i % 3 == 2) {
          display_mem[i] = 7;
        }
      }
      break;

    default:
      // set all LEDs to color passed to function

      //first constrain color to 0:511 range
      color = constrain(color, 0, 511);
      for (int16_t i = 0; i < ledCount; i++) {
        display_mem[i] = color;
      }
  }
}

// ************************
// blank text character at current origin location with selected color ( in prep to print next char )
void blankchar ( int16_t origin, int16_t color, int16_t *display_mem ) {
  for ( byte i = 0; i < 10; i++ ) {
    for ( byte j = 0; j < 5; j++ ) {
      display_mem[32 * i + (origin + j)] = color;
    }
  }
}


// ************************
// print text character at current origin location with selected color
void printchar(int16_t origin, int16_t color, char letter, int16_t* display_mem ) {

  word display_adder = 0;
  byte led_column = 0;

  // how this works is a little funky.
  // loop through the 20 pixel array for the given character, then for each non-zero (!=9999) pixel determine which column in the 10x5 matrix it is in
  // if it is in a column that currently should be illuminated based on where the moving origin location is, draw the pixel.  hopefully this handles the scrolling of the letters on and off the edges

  for ( byte i = 0; i < 20; i++) {
    display_adder = pgm_read_word_near(&text_lookup[letter_lookup(letter)][i]);

    // check if LED position in letter array is a valid one
    if ( display_adder < 999 ) {

      // next check which column the LED is in so can determine whether or not to draw it based on the current location of the letter's origin
      led_column = display_adder % 32;

      // now, based on where letter origin is, draw led or not
      switch (led_column) {

        case 0:
          if ( origin > 95 && origin < 128 ) display_mem[origin + display_adder] = color;
          break;

        case 1:
          if ( origin > 94 && origin < 127 ) display_mem[origin + display_adder] = color;
          break;

        case 2:
          if ( origin > 93 && origin < 126 ) display_mem[origin + display_adder] = color;
          break;

        case 3:
          if ( origin > 92 && origin < 125 ) display_mem[origin + display_adder] = color;
          break;

        case 4:
          if ( origin > 91 && origin < 124 ) display_mem[origin + display_adder] = color;
          break;

      }
    }
  }
}


// ************************
// check NES code vs. preset code
byte check_code( byte *current_code, byte button, byte last_button, const byte *code_array ) {

  byte code_match = 1;
  byte full_code = 1;
  byte current_code_start = 0;

  //locate start of currently entered code in array
  for ( byte i = 0; i < 14; i++ ) {
    if ( current_code[i] == 9 ) {
      current_code_start = i;
      break;
    }
  }

  // now the fun begins, check current button, last button to determine whether or not to add button to the current code array for the code check
  if ( button < 99 && last_button == 99  ) {
    current_code[current_code_start] = button;
  }

  //lastly, loop though code array to check if we have a match
  for ( byte i = 0; i < 14; i++ ) {
    if ( code_array[i] != current_code[i] && current_code[i] != 9) code_match = 0;
    if ( current_code[i] == 9 ) full_code = 0;
  }

  // if not match found and invalid code was entered, clear current array to restart code entry
  if ( code_match == 0 ) {
    for ( byte i = 0; i < 14; i++) {
      current_code[i] = 9;
    }
  }

  if ( full_code == 1 && code_match == 1 ) {
    for ( byte i = 0; i < 14; i++) {
      current_code[i] = 9;
    }
    return code_match;
  } else {
    return 0;
  }
}


// ************************
// blink a selected location white for .1 seconds
void cursor_blink(int16_t location, int16_t *display_mem) {

  // save off currrent state so we can load it back when done
  int16_t save_state = display_mem[location];

  display_mem[location] = 511;
  delay(50);
  display_mem[location] = 0;
  delay(50);
  display_mem[location] = save_state;

}

// ************************
// helper to fade down selected LED, return value indicates if LED is now off
byte fade_down(int16_t location, byte fade_step, int16_t *display_mem) {

  // fade down red
  if (display_mem[location] / 64 > 0 ) {
    display_mem[location] = constrain(display_mem[location] - (64 * fade_step), 0, 511);
    if ( display_mem[location] == 0 ) return 1;
  }

  // fade down blue
  if (display_mem[location] % 8 > 0 ) {
    display_mem[location] = constrain(display_mem[location] - (1 * fade_step), 0, 511);
    if ( display_mem[location] == 0 ) return 1;
  }

  // fade down green
  if ((display_mem[location] % 64) / 8 > 0 ) {
    display_mem[location] = constrain(display_mem[location] - (8 * fade_step), 0, 511);
    if ( display_mem[location] == 0 ) return 1;
  }

  if ( display_mem[location] == 0 ) return 1;
  return 0;
}

// ************************
// helper to fade up selected LED to target color, will bump R,G,B each by 1 increment if possible to reach target, return 1 when target reached
byte fade_up(int16_t location, int16_t target, int16_t *display_mem) {

  // fade up red
  if (display_mem[location] / 64 < target / 64 ) {
    display_mem[location] = display_mem[location] + 64;
  }

  // fade up blue
  if (display_mem[location] % 8 < target % 8 ) {
    display_mem[location] = display_mem[location] + 1;
  }

  // fade up green
  if ((display_mem[location] % 64) / 8 < (target % 64) / 8 ) {
    display_mem[location] = display_mem[location] + 8;
  }

  if ( display_mem[location] == target ) {
    return 1;
  } else {
    return 0;
  }
}

// ************************
// draw paddle for pong
void draw_paddle(int16_t *display_mem, int16_t nes_location, byte paddle_width) {

  int16_t paddle_color = 448;

  //clear bottom row and draw current paddle location
  for ( int16_t i = 480; i < 512; i++) {

    if ( i >= nes_location && i < ( nes_location + paddle_width ) ) {
      display_mem[i] = paddle_color;
    } else {
      display_mem[i] = 0;
    }

  }
}

// ************************
// initialize the rfill array, 512 led positions in 512 array spots randomly chosen with no repeats
void rfill_init(byte mode, int16_t ledCount, int16_t *rfill_array  ) {

  // mode = 0 means simply clear the array
  // mode = 1 means init w/ random led positions

  int16_t randLed = random(0, 512);
  byte led_in_array = 0;

  for ( int16_t i = 0; i < ledCount; i++ ) {
    // clear is easy
    if ( mode == 0 ) rfill_array[i] = 999;

    // fill is a bit harder
    if ( mode == 1 ) {

      // check if current
      led_in_array = in_rfill(randLed, ledCount, rfill_array);

      // if the random led position is already in the array, choose another one and repeat.  once open position found, complete this led
      while (led_in_array == 1) {
        randLed = random(0, 512);
        led_in_array = in_rfill(randLed, ledCount, rfill_array);
      }

      // once randomled is found not to be in array already, insert into array
      rfill_array[i] = randLed;
    }
  }
}

// ************************
// helper for rfill, check array if given led position is already in the array or not
byte in_rfill(int16_t pos, int16_t ledCount, int16_t *rfill_array) {
  // return 1 if the passed position is in the rfill_array, otherwise 0
  for (int16_t i = 0; i < ledCount; i++ ) {
    if (rfill_array[i] == pos ) return 1;
  }
  return 0;
}

// ************************
// helper to set LED color, include location / color checking
void set_led(int16_t location, int16_t color, int16_t *display_mem) {

  // check for valid location on panel do nothing otherwise
  if ( location > 0 && location < 512 ) {
    display_mem[location] = color;
  }
}


// ************************
// helper for line sweeper animation to select new sweep type
void newline(byte box, byte prev_type, byte *line_type, short *cur_x, short *cur_y) {

  line_type[box] = random(0, 4); // grab new line random line type for next animation
  while ( line_type[box] == prev_type ) {
    line_type[box] = random(0, 4);
  }
  //line_type = 0;
  //line_color = random(0, 512);
  if ( line_type[box] == 0 ) {
    cur_x[box] = (box / 2) * 8;
  } else if ( line_type[box] == 1 ) {
    cur_x[box] = ((box / 2) * 8) + 7;
    //cur_x[box] = 7;
  } else if ( line_type[box] == 2 ) {
    cur_y[box] = (box % 2) * 8;
  } else if ( line_type[box] == 3 ) {
    cur_y[box] = ((box % 2) * 8) + 7;
  }
}

// ************************
// draw selected row
void draw_row(byte row, int16_t color, int16_t *display_mem, byte cols ) {

  // loop through colums to set each LED in row on
  for ( byte i = 0; i < cols; i++ ) {
    display_mem[32 * row + i] = color;
  }
}

// ************************
// draw selected column
void draw_col(byte col, int16_t color, int16_t *display_mem, byte rows ) {

  // loop through rows to set each LED in column on
  for ( byte i = 0; i < rows; i++ ) {
    display_mem[32 * i + col] = color;
  }
}

// ************************
// compute complimentary color
int16_t opposite_color(int16_t color) {
  return 64 * (7 - (color / 64)) + 8 * (7 - ((color % 64) / 8 )) + (7 - (color % 8 ));
}

// ************************
// color selector for paint operation
int16_t paint_choose(int16_t *display_mem, int16_t *rfill_array, volatile byte *nes_state1, int16_t ledCount, byte *color_select, int16_t *nes_location, byte *code_valid, byte *current_code, byte *last_button, const byte *code_array, byte debug) {
  
  byte up = bitRead(*nes_state1, 4);
  byte down = bitRead(*nes_state1, 5);
  byte left = bitRead(*nes_state1, 6);
  byte right = bitRead(*nes_state1, 7);
  byte a = bitRead(*nes_state1, 0);

  //display color select screen
  clear_all(514, ledCount, display_mem);
  
  byte row = *nes_location / 32;
  byte column = *nes_location % 32;
  
  // draw blank box around currently selected LED color
  if ( row > 0 ) {   // current location is not top row
  
    display_mem[*nes_location - 32] = 448;  // blank above led
    
    if ( column > 0 ) {
      display_mem[*nes_location - 33] = 448;  // blank upper left led
      display_mem[*nes_location - 1] = 448;   // blank left led
    }
    
    if ( column < 31 ) { 
      display_mem[*nes_location - 31] = 448;  // blank upper right led
      display_mem[*nes_location + 1] = 448;   // blank right led 
    }
  }
  
  if ( row < 15 ) {
    display_mem[*nes_location + 32] = 448;  // blank below led
    
    if ( column > 0 ) {
      display_mem[*nes_location + 31] = 448;  // blank lower left led
      display_mem[*nes_location - 1] = 448;   // blank left led
    }
    
    if ( column < 31 ) { 
      display_mem[*nes_location + 33] = 448;  // blank lower right led
      display_mem[*nes_location + 1] = 448;   // blank right led 
    }
  }
    
    
  if ( a == 0 ) {
    //clear_all(0, ledCount, display_mem);
    memcpy( display_mem, rfill_array, 1024 );
    *color_select = 0;
    if ( debug == 1 ) Serial.println(*nes_location);
    return *nes_location;
  }
  
  // lastly, if user clicks start while in paint select mode, it may be part of code entry so check for valid
  if ( bitRead(*nes_state1, 3 ) == 0 ) {
	// check current button combo
	*code_valid = check_code(current_code, 7, *last_button, code_array);
	*last_button = 7;
  }
  
}

// ************************
// update snake positions based on direction
void updateSnakes(int16_t *snake1, byte dir1, int16_t *snake2, byte dir2, byte snakeLength, byte debug ) {

  int16_t adder1 = 0;
  int16_t adder2 = 0;
  byte row1, col1, row2, col2;
  
  // calc current row/col of snake head
  row1 = snake1[0] / 32;
  col1 = snake1[0] % 32;
  
  row2 = snake2[0] / 32;
  col2 = snake2[0] % 32;
 
  // the snakes direction will be used to determine how to adjust it's segments for it's next position
  switch (dir1) {
    case 0:  // up
		if ( row1 > 0 ) {
			adder1 = -32;
		} else if ( row1 == 0 ) {
			adder1 = 480;
		}
		break;
    case 1:  // left
		if ( col1 > 0 ) {
			adder1 = -1;
		} else if ( col1 == 0 ) {
			adder1 = 31;
		}
		break;
    case 2:  // right
		if ( col1 < 31 ) {
			adder1 = 1;
		} else if ( col1 == 31 ) {
			adder1 = -31;
		}
		break;
    case 3:  // down
		if ( row1 < 15 ) {
			adder1 = 32;
		} else if ( row1 == 15 ) {
			adder1 = -480;
		}
      break;    
  }
  
  switch (dir2) {
     case 0:  // up
		if ( row2 > 0 ) {
			adder2 = -32;
		} else if ( row2 == 0 ) {
			adder2 = 480;
		}
		break;
    case 1:  // left
		if ( col2 > 0 ) {
			adder2 = -1;
		} else if ( col2 == 0 ) {
			adder2 = 31;
		}
		break;
    case 2:  // right
		if ( col2 < 31 ) {
			adder2 = 1;
		} else if ( col2 == 31 ) {
			adder2 = -31;
		}
		break;
    case 3:  // down
		if ( row2 < 15 ) {
			adder2 = 32;
		} else if ( row2 == 15 ) {
			adder2 = -480;
		}
      break;    
  }
  
  if ( debug == 1 ) {
	Serial.print("dir: ");
	Serial.print(dir1);
    Serial.print (" row: ");
    Serial.print (row1);
    Serial.print (" col: ");
    Serial.print(col1);
    Serial.print(" adder: ");
    Serial.println(adder1);
  }
  
  // to update the snake positions, shift the segment locations to the right by 1 in the array and add the adder value to the head
  
  // save current head locations
  int16_t oldHead1 = snake1[0];
  int16_t oldHead2 = snake2[0];
  
  // update head locations
  if ( snake1[0] < 999 ) snake1[0] = snake1[0] + adder1;
  if ( snake2[0] < 999 ) snake2[0] = snake2[0] + adder2;
    
  // loop through the rest of the snake length and shift the previous segment to the new one
  for ( byte i = snakeLength;  i > 0; i-- ) {
    
    if ( snake1[i] < 999 ) {
      if ( i == 1 ) {
        snake1[i] = oldHead1;
      }
      else {
        snake1[i] = snake1[i-1];  
      }
    }
    
    if ( snake2[i] < 999 ) {
      if ( i == 1 ) {
        snake2[i] = oldHead2;
      }
      else {
        snake2[i] = snake2[i-1];  
      }
    }
          
    if ( debug == 1 ) {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(snake1[i]);
    }
  }
  
  if ( debug == 1 ) {
	Serial.print(0);
    Serial.print(": ");
    Serial.println(snake1[0]);
  }
  
}

// ************************
// draw snakes on display
void drawSnakes(int16_t *snake1, int16_t *snake2, int16_t *display_mem, int16_t ledCount, const byte snakeLength, int16_t food1, int16_t food2, byte foodmode ) {
  
  // clear last frame
  clear_all(0, ledCount, display_mem);
  
  // now draw new snake positions
  for ( byte i = 0; i < snakeLength; i++ ) {
    
    // update only segments of the snake that are "alive"
    if ( snake1[i] < 999 ) display_mem[snake1[i]] = 448;
    if ( snake2[i] < 999 ) display_mem[snake2[i]] = 7;
    
  }
  
  //draw food pellets
  if ( foodmode == 0 ) {
    display_mem[food1] = 304;
  } else if ( foodmode == 1 ) {
    display_mem[food1] = 448;
    display_mem[food2] = 7;
  }
}

// ************************
// check is snakes have made contact with each other, themselves or the borders
void checkSnakes(int16_t *snake1, int16_t *snake2, byte snakeLength, byte *dead1, byte *dead2, int16_t *food1, int16_t *food2, byte foodmode, byte players ) {

   for ( byte i = 1; i < snakeLength; i++ ) {
    
     //first check if head location is making contact elsewhere on same snake's body
    if ( snake1[i] < 999 ) {  // only check for live portion of snake
      if ( snake1[i] == snake1[0] ) *dead1 = 1;
    }
    if ( snake2[i] < 999 ) {  // only check for live portion of snake
      if ( snake2[i] == snake2[0] ) *dead2 = 1;
    }
  }
  
  // next check if snakes are making contact with any part of opponent's body
  for ( byte i = 0; i < snakeLength; i++ ){
    
    // check snake1
    if ( snake2[i] < 999 ) {
      if ( snake1[0] == snake2[i] ) *dead1 = 1;
    }
    
    // check snake2
    if ( snake1[i] < 999 ) {
      if ( snake2[0] == snake1[i] ) *dead2 = 1;
    }
  }
  
  //now check if snakes have "eaten" food pellet
  if ( foodmode == 0 ) {
    if ( snake1[0] == *food1 ) {
      for ( byte i=0; i<snakeLength; i++ ) {
        if ( snake1[i] == 999 ) {
          snake1[i] = snake1[i-1];
          break;
        }
      }
      newFood(snake1, snake2, snakeLength, food1, food2, foodmode, 0);
    }
    
    if ( players == 2 ) {
      if ( snake2[0] == *food1 ) {
        for ( byte i=0; i<snakeLength; i++ ) {
          if ( snake2[i] == 999 ) {
            snake2[i] = snake2[i-1];
            break;
          }
        }
        newFood(snake1, snake2, snakeLength, food1, food2, foodmode,0);
      }
    }
  }
  
  if ( foodmode == 1 ) {
  
    // check if snake1 ate its own pellet, grow if that's the case
    if ( snake1[0] == *food1 ) {
      for ( byte i=0; i<snakeLength; i++ ) {
        if ( snake1[i] == 999 ) {
          snake1[i] = snake1[i-1];
          break;
        }
      }
      newFood(snake1, snake2, snakeLength, food1, food2, foodmode,0 );
    }
    
    // check if snake1 ate opponent's pellet, shrink if that's the case
    if ( snake1[0] == *food2 ) {
      for ( byte i=0; i<snakeLength; i++ ) {
        if ( snake1[i] == 999 ) {
          snake1[i-1] = 999;
          if ( i-1 == 0 ) *dead1 = 1;
          break;
        }
      }
      newFood(snake1, snake2, snakeLength, food1, food2, foodmode,1 );    
    }
	
	// repeat checks for second snake
	// check if snake2 ate its own pellet, grow if that's the case
    if ( snake2[0] == *food2 ) {
      for ( byte i=0; i<snakeLength; i++ ) {
        if ( snake2[i] == 999 ) {
          snake2[i] = snake2[i-1];
          break;
        }
      }
      newFood(snake1, snake2, snakeLength, food1, food2, foodmode,1 );
    }
    
    // check if snake2 ate opponent's pellet, shrink if that's the case
    if ( snake2[0] == *food1 ) {
      for ( byte i=0; i<snakeLength; i++ ) {
        if ( snake2[i] == 999 ) {
          snake2[i-1] = 999;
          if ( i-1 == 0 ) *dead2 = 1;
          break;
        }
      }
      newFood(snake1, snake2, snakeLength, food1, food2, foodmode,0 );    
    }
	
	
  }
}

// ************************
// create new food location
void newFood(int16_t *snake1, int16_t *snake2, byte snakeLength, int16_t *food1, int16_t *food2, byte foodmode, byte whichfood ) {
  
  byte location_valid = 1;
  
  //based on foodmode, either create 1 or 2 "pellets"
  if ( foodmode == 0 ) {  // one pellet mode       
    do {    
      *food1 = random(0,512); // assign random pellet location
      location_valid = 1;
      
      // check against current snake position(s)
      for ( byte i=0; i < snakeLength; i++ ) {
        
        if ( snake1[i] < 999 ) {          
          if ( *food1 == snake1[i] ) location_valid = 0;  // check vs. snake1
        }
        
        if ( snake2[i] < 999 ) {                        // check vs. snake2 if snake head is there
          if ( *food1 == snake2[i] ) location_valid = 0;  
        }
      }
    } while ( location_valid ==  0 );     
  }
  
  if ( foodmode == 1 ) {  // two pellet mode    
    
    if ( whichfood == 0 || whichfood == 2 ) {  // need new snake1 pellet
      do {    
        *food1 = random(0,512); // assign random pellet location
        location_valid = 1;
      
        // check against current snake position(s)
        for ( byte i=0; i < snakeLength; i++ ) {
        
          if ( snake1[i] < 999 ) {          
            if ( *food1 == snake1[i] ) location_valid = 0;  // check vs. snake1
          }
        
          if ( snake2[i] < 999 ) {                        // check vs. snake2 if snake head is there
            if ( *food1 == snake2[i] ) location_valid = 0;  
          }
        }
        
        // check vs. other pellet
        if ( *food1 == *food2 ) location_valid = 0;
        
      } while ( location_valid ==  0 );  
    }   

    if ( whichfood == 1 || whichfood == 2 ) {  // need new snake2 pellet
      do {    
        *food2 = random(0,512); // assign random pellet location
        location_valid = 1;
      
        // check against current snake position(s) AND food1 location
        for ( byte i=0; i < snakeLength; i++ ) {
        
          if ( snake1[i] < 999 ) {          
            if ( *food2 == snake1[i] ) location_valid = 0;  // check vs. snake1
          }
        
          if ( snake2[i] < 999 ) {                        // check vs. snake2 if snake head is there
            if ( *food2 == snake2[i] ) location_valid = 0;  
          }
        }
      
        if ( *food1 == *food2 ) location_valid = 0;
      
      } while ( location_valid ==  0 );   
    }  
  }   
}

// ************************
// randomize gamestate for life
void init_life(byte *gamestate, int *iteration, int16_t ledCount, int16_t *wave_color) {

  
  byte weight = random(2, 4);
  byte newval = 0;
  *iteration = 0;
  
  for ( int16_t i = 0; i < ledCount; i++ ){
    newval = random(0, weight);
	
	switch (newval) {
		
		case 0:
		gamestate[i] = 1;
		break;
		
		case 1:
		gamestate[i] = 0;
		break;
		
		case 2:
		gamestate[i] = 0;
		break;
		
		case 3:
		gamestate[i] = 0;
		break;
		
	}
	
  }
  
  wave_color[0] = random(1,512);

}

// ************************
// randomize maze board before trying to solve it
void init_maze(byte *maze_nodes, byte *maze_walls, byte *nodestack, char *stack_index, int16_t ledCount, int16_t *display_mem, int16_t *current_cell, int16_t *wave_color, byte *type, byte *wall_update, byte debug) {
	
	// when the init_maze function is called, the board will be cleared and a new random maze will be generated via a depth first search algorithm across the 105 node / 188 wall space
	
	// first clear current state of the array
	clear_all(0, ledCount, display_mem);
	*current_cell = 0;
	*wall_update = 0;
	//wave_color[1] = random(1,512);
	wave_color[1] = random(0,8)*8 + random(0,8)*64;
	if ( wave_color[1] == 0 ) wave_color[1] = random(0,8)*8 + random(0,8)*64;
	*type = random(0,2);
	
	// reset the node array to every cell as unvisited and every wall as in place
	for ( byte i = 0; i < 188; i++ ) {
		
		if ( i < 105 ) {
			maze_nodes[i] = 1;
			nodestack[i] = 110;
		}
		
		maze_walls[i] = 1;
		
	}
	
	// TODO: add recursive algorithm to traverse space to generate maze, for now just draw grid
	maze_gen(current_cell, maze_nodes, maze_walls, nodestack, stack_index, debug);
	
	
	// now that the maze has been calculated, actually draw it.
	
		
	// draw the boards rows and columns followed by removal of the node walls based on the calculated wall state from above
	for ( byte i = 0; i < 31; i=i+2 ) {
		
		draw_col(i, wave_color[0], display_mem, 16 );
		
		if ( i < 15 ) draw_row(i, wave_color[0], display_mem, 32 );
		
	}
	
	//also add unused borders, row15 and column31
	draw_row(15, 0, display_mem, 32 );	
	draw_col(31, 0, display_mem, 16 );
	
	// now traverse though wall array and remove any zero'd walls
	byte wall_row = 0;
	byte wall_col = 0;
	

	// for debug only, randomly remove some wall locations to check if the board is being drawn correclty still
	//for ( byte i = 0; i<50; i++ ){
	//	maze_walls[random(0,188)] = 0;
	//}
	
	
	for ( byte i=0; i < 188; i++ ) {
		
		// calc wall row/col to determine LED position in matrix and then if current wall is not set, clear corresponding LED in matrix
		
		//wall_col = i%14;
		
		if( i < 98 ) {
			wall_row = i / 14;
			wall_col = i % 14;
			if(maze_walls[i] == 0 ) display_mem[34 + (2*wall_row*32) + (2*wall_col)] = 0;    // wall locations < 98 are vertical walls on the sides of nodes
			
		} else {
			wall_row = (i-98) % 6;
			wall_col = (i-98) / 6;
			//wall_row = (i/14) - 7;
			if (maze_walls[i] == 0 ) display_mem[65 + (2*wall_row*32) + (2*wall_col)] = 0;	 // wall locations > 97 are horizontal walls on the top/bottom of nodes
		}
		
	} 
	
	// remove start and end node edges
	// fixed start and end nodes for now
	display_mem[32] = wave_color[1];
	display_mem[33] = wave_color[1];
	display_mem[446] = 0;
	
	// now that maze is create, mark all the nodes unvisited and set current cell to maze start	
	for ( byte i=0; i < 105; i++ ) {
		maze_nodes[i] = 1;
		nodestack[i] = 110;
	}
	
	*current_cell = 0;
	*stack_index = 0;
	
}

// ************************
// recursive function to generate random maze board by depth first search of unvisited nodes from current cell

/*
    Make the initial cell the current cell and mark it as visited
    While there are unvisited cells
        If the current cell has any neighbours which have not been visited
            Choose randomly one of the unvisited neighbours
            Push the current cell to the stack
            Remove the wall between the current cell and the chosen cell
            Make the chosen cell the current cell and mark it as visited
        Else if stack is not empty
            Pop a cell from the stack
            Make it the current cell
        Else
            Pick a random unvisited cell, make it the current cell and mark it as visited
*/
void maze_gen(int16_t *current_cell, byte *maze_nodes, byte *maze_walls, byte *nodestack, char *stack_index, byte debug ) {
	
	// check nodes to see if any are still unvisited
	byte unvisited = checkMazeNodes(maze_nodes);
	
	// if there are, do the whole traverse thing
	if ( unvisited == 0 ) {
	
		// calc row and column for current node
		byte node_row = *current_cell / 15;
		byte node_col = *current_cell % 15;
		byte neighbors[4] = {110,110,110,110};  // 110 indicated no valid or visited neighbor, index 0 = upper, 1 = lower, 2 = left, 3 = right
		
		// first mark the current cell as visited
		maze_nodes[*current_cell] = 0;
			
		// find current cells neighbors  and check if any of them have not yet been visited
	
		// check upper neighbor
		if ( node_row > 0 ) {
			if ( maze_nodes[*current_cell - 15 ] == 1 ) {
				neighbors[0] = *current_cell - 15;
				unvisited++;
			}
		}
		
		// check lower neighbor
		if ( node_row < 6 ) {
			if ( maze_nodes[*current_cell + 15 ] == 1 ) {
				neighbors[1] = *current_cell + 15;
				unvisited++;
			}
		}
		
		// check left neighbor
		if ( node_col > 0 ) {
			if ( maze_nodes[*current_cell - 1 ] == 1 ) {
				neighbors[2] = *current_cell - 1;
				unvisited++;
			}
		}
		
		// check right neighbor
		if ( node_col < 14 ) {
			if ( maze_nodes[*current_cell + 1 ] == 1 ) {
				neighbors[3] = *current_cell + 1;
				unvisited++;
			}
		}
		
		// now randomly select a neighbor cell and if neighbor exists and if it has been checked yet, if it both exists and has not been checked, remove wall between cells recursively 	
		if ( unvisited > 0 ) {
			byte next_cell = random(0,4);
			
			// we know at least one neighbor is unvisited, keep searching for it
			while( neighbors[next_cell] == 110 ) {
				next_cell = random(0,4);
			}
			
			// once found, now the magic happens, mark wall between 2 cells as gone, make neighbor cell the current cell and call maze_gen with new current cell so process can repeat
				
			if ( debug == 1 ) {
				Serial.print("current_cell: ");
				Serial.print(*current_cell);
				Serial.print("  next_cell: ");
				Serial.print(neighbors[next_cell]);
				Serial.print("  remove wall: ");
				
			}
		
			// wall to remove depends on if neighbor is upper, lower, left or right (next_cell value )
			switch (next_cell) {
				
					case 0:		// upper wall
					maze_walls[97 + (6*node_col) + node_row] = 0;
					if ( debug == 1 ) Serial.println(97 + (6*node_col) + node_row);
					break;
				
				case 1:		// lower wall
					maze_walls[98 + (6*node_col) + node_row] = 0;
					if ( debug == 1 ) Serial.println(98 + (6*node_col) + node_row);
					break;
				
				case 2: 	// left wall
					maze_walls[*current_cell - (node_row+1)] = 0;
					if ( debug == 1 ) Serial.println(*current_cell - (node_row+1));
					break;
				
				case 3: 	// right wall
					maze_walls[*current_cell - node_row] = 0;
					if ( debug == 1 ) Serial.println(*current_cell - node_row);
					break;
			}
			
			// now update current cell to new neighbor cell and call maze_gen again with new neighbor
			
			// push current cell to stack
			nodestack[*stack_index] = *current_cell;
			*stack_index = *stack_index + 1;
			
			*current_cell = neighbors[next_cell];
			
			maze_gen(current_cell, maze_nodes, maze_walls, nodestack, stack_index, debug);	
			
		} else {
			// no unvisited cells in at current cell location, pop cell from stack and use as next current cell, this will work backwards through the traversed nodes until one with neighbors is found
			
			if ( *stack_index > 0 ) {
				*stack_index = *stack_index - 1;
				*current_cell = nodestack[*stack_index];
				nodestack[*stack_index] = 110;
				
				maze_gen(current_cell, maze_nodes, maze_walls, nodestack, stack_index, debug );
				
			}	
		}
	}	
}


// ************************
// check maze for any unvisited nodes
byte checkMazeNodes(byte *maze_nodes) {
	
	byte unvisited = 1;
	for ( byte i=0; i<105; i++ ) {
		if ( maze_nodes[i] == 1 ) unvisited = 0;
	}
	
	return unvisited;
	
}

// ************************
// set/unset wall LED between 2 given nodes, nodes must be adjacent!
void set_wall(byte n1, byte n2, int16_t color, int16_t *display_mem) {
	
	byte node_row = n2 / 15;
	byte node_col = n2 % 15;
	
	// compare 2 nodes to determine what their configuration is to decide which wall is to be set/unset
	if ( n1 > n2 ) {
		if ( n1 - n2 > 5 ) {  	// if true nodes are not in same row and n1 is below n2
			// set n2 bottom wall
			display_mem[65 + (64*node_row) + (2*node_col)] = color;
		} else {				// else nodes are in same row and n1 is right of n2
			// set n2 right wall
			display_mem[34 + (64*node_row) + (2*node_col)] = color;
		}			
	} else {
		if ( n2 - n1 > 5 ) {	// if true nodes are not in same row and n2 is below n1
			// set n2 top wall
			display_mem[1 + (64*node_row) + (2*node_col)] = color;
		} else {				// else nodes are in same row and n2 is right of n1
			// set n2 left wall
			display_mem[32 + (64*node_row) + (2*node_col)] = color;
		}
	}
}

// ************************
// color change,  replace any LED in current matrix of one color with another
void colorchange(int16_t current_color, int16_t new_color, int16_t *display_mem, int16_t ledCount) {

	for ( int16_t i=0; i<ledCount; i++ ) {
		
		if ( display_mem[i] == current_color ) display_mem[i] = new_color;
		
	}

}


