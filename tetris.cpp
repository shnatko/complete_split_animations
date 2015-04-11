#include "tetris.h"
#include <Arduino.h>


// *************************************************************************************************************************************
// initial set-up or start of new game
void setup_tetris(int16_t ledCount, byte *tetris_stack, byte *part_type, byte *next_part, int16_t *display_mem, byte partstart_y, byte *gameOver, byte *previousPauseButtonState, int16_t *fall_rate, int16_t *score) {
 
  // clear playing board and randomize parts
  int16_t i=0;
  for ( i=0; i<ledCount; i++ ) {
    display_mem[i] = 0;
    tetris_stack[i] = 0;
  }
  
  // initialize random seeds
  randomSeed(analogRead(0));
  randomSeed(random(0,1025));
 
  // set initial piece state 
  *part_type = random(0,7);    
  *next_part = random(0,7);    
  
  // draw top of game space
  for ( i=0; i<16; i++ ) {
    display_mem[(partstart_y-2) + 32*i] = 511;
  }
  
  *score = 0;
  *gameOver = 0;
  *previousPauseButtonState = 1;
  *fall_rate = 500;
  
}


// *************************************************************************************************************************************
// helper to set the movebounds of the current part
void setbounds(byte type, byte part_o, byte *left, byte *right, byte *top, byte *bottom ) {

  // set game area boundaries based on part type / orientatation
  switch ( type ) {
    case 0:              // 2x2 square, only 1 orientation
      *left = 15;
      *right = 1; 
      *top = 0;
      *bottom = 30;
    break;
    
    case 1:              // 4x1 piece, 2 orientations
      if ( part_o == 0 ) {
        *right = 2; 
        *left = 14;
        *bottom = 31;
        *top = 0;
      } else {
        *right = 0; 
        *left = 15;
        *bottom = 30;
        *top = 2;
      }
     break;
     
     case 2:              // L piece, 4 orientations
      if ( part_o == 0 ) {
        *right = 1;
        *left = 15;
        *bottom = 30; 
        *top = 1;
      } else if ( part_o == 1 ) {
        *right = 1;
        *left = 14;
        *bottom = 30; 
        *top = 0;
      } else if ( part_o == 2 ) {
        *right = 0;
        *left = 14;
        *bottom = 30; 
        *top = 1;
      } else if ( part_o == 3 ) {
        *right = 1;
        *left = 14;
        *bottom = 31; 
        *top = 1;
      }
     break;
     
     case 3:              // reverse L piece, 4 orientations
      if ( part_o == 0 ) {
        *right = 0;
        *left = 14;
        *bottom = 30; 
        *top = 1;
      } else if ( part_o == 1 ) {
        *right = 1;
        *left = 14;
        *bottom = 31; 
        *top = 1;
      } else if ( part_o == 2 ) {
        *right = 1;
        *left = 15;
        *bottom = 30; 
        *top = 1;
      } else if ( part_o == 3 ) {
        *right = 1;
        *left = 14;
        *bottom = 30; 
        *top = 0;
      }
     break;
     
     case 4:              // T shape, 4 orientations
      if ( part_o == 0 ) {
        *right = 1;
        *left = 14;
        *bottom = 31; 
        *top = 1;
      } else if ( part_o == 1 ) {
        *right = 1;
        *left = 15;
        *bottom = 30; 
        *top = 1;
      } else if ( part_o == 2 ) {
        *right = 1;
        *left = 14;
        *bottom = 30; 
        *top = 0;
      } else if ( part_o == 3 ) {
        *right = 0;
        *left = 14;
        *bottom = 30; 
        *top = 1;
      }
     break;
     
     case 5:              // Z piece, 2 orientations
      if ( part_o == 0 ) {
        *right = 1; 
        *left = 14;
        *bottom = 31;
        *top = 1;
      } else {
        *right = 1; 
        *left = 15;
        *bottom = 30;
        *top = 1;
      }
     break;
     
     case 6:              // mirror Z piece, 2 orientations
      if ( part_o == 0 ) {
        *right = 1; 
        *left = 14;
        *bottom = 31;
        *top = 1;
      } else {
        *right = 0; 
        *left = 14;
        *bottom = 30;
        *top = 1;
      }
     break;
         
  } 
}

// *************************************************************************************************************************************
// update stack function is used to manage stacking up the pieces and ensuring no overlap
// this function is basically the same as drawpart but will keep part locations in seperate memory from display memory
// it should make clearing rows easier later...
void update_stack(byte type, byte orientation, byte x_loc, byte y_loc, byte *tetris_stack, byte debug, int16_t ledCount) {
  
  if ( debug == 1 ) {
    Serial.println("update_stack");
  }

  // depending on part type and orientation, store color info in stack, any non-zero location will mean square is occupied 
  switch (type) {
    case 0:   // 2x2 square
      tetris_stack[32*y_loc + x_loc ]         = 1;
      tetris_stack[32*y_loc + (x_loc+1) ]     = 1;
      tetris_stack[32*(y_loc-1) + x_loc ]     = 1;
      tetris_stack[32*(y_loc-1) + (x_loc+1) ] = 1;
    break;
    
    case 1:  // 4x1 rod
      tetris_stack[32*y_loc + x_loc]          = 1;
      
      if ( orientation == 0 ) {
        tetris_stack[32*(y_loc+1) + x_loc]    = 1;
        tetris_stack[32*(y_loc-1) + x_loc]    = 1;
        tetris_stack[32*(y_loc-2) + x_loc]    = 1;
        
      } else {
        tetris_stack[32*y_loc + (x_loc-1)]    = 1;
        tetris_stack[32*y_loc + (x_loc-2)]    = 1;
        tetris_stack[32*y_loc + (x_loc+1)]    = 1;
        
      }
    break;
    
    case 2:  // L 
      tetris_stack[32*y_loc + x_loc]          = 1;
      
      if ( orientation == 0 ) {
        tetris_stack[32*y_loc + (x_loc-1)]       = 1;
        tetris_stack[32*y_loc + (x_loc+1)]       = 1;
        tetris_stack[32*(y_loc-1) + (x_loc+1)]   = 1;
       
      } else if (orientation == 1) {
        tetris_stack[32*(y_loc+1) + x_loc]      = 1;
        tetris_stack[32*(y_loc-1) + x_loc]      = 1;
        tetris_stack[32*(y_loc+1) + (x_loc+1)]  = 1;
       
  
      } else if (orientation == 2) {
        tetris_stack[32*y_loc + (x_loc+1)]       = 1;
        tetris_stack[32*y_loc + (x_loc-1)]       = 1;
        tetris_stack[32*(y_loc+1) + (x_loc-1)]   = 1; 
        
      } else if (orientation == 3) {
        tetris_stack[32*(y_loc-1) + x_loc]       = 1;
        tetris_stack[32*(y_loc+1) + x_loc]       = 1;
        tetris_stack[32*(y_loc-1) + (x_loc-1)]   = 1;
        
      }
      break;
      
    case 3:  // mirror L 
      tetris_stack[32*y_loc + x_loc]          = 1;
      
      if ( orientation == 0 ) {
        tetris_stack[32*y_loc + (x_loc+1)]       = 1;
        tetris_stack[32*y_loc + (x_loc-1)]       = 1;
        tetris_stack[32*(y_loc+1) + (x_loc+1)]   = 1;
        
      } else if (orientation == 1) {
        tetris_stack[32*(y_loc+1) + x_loc]      = 1;
        tetris_stack[32*(y_loc-1) + x_loc]      = 1;
        tetris_stack[32*(y_loc+1) + (x_loc-1)]  = 1;
        
      } else if (orientation == 2) {
        tetris_stack[32*y_loc + (x_loc-1)]       = 1;
        tetris_stack[32*y_loc + (x_loc+1)]       = 1;
        tetris_stack[32*(y_loc-1) + (x_loc-1)]   = 1;
        
      } else if (orientation == 3) {
        tetris_stack[32*(y_loc-1) + x_loc]       = 1;
        tetris_stack[32*(y_loc+1) + x_loc]       = 1;
        tetris_stack[32*(y_loc-1) + (x_loc+1)]   = 1;
        
      }
    break;
    
    case 4:  // T 
      tetris_stack[32*y_loc + x_loc]          = 1;
      
      if ( orientation == 0 ) {
        tetris_stack[32*y_loc + (x_loc-1)]       = 1;
        tetris_stack[32*(y_loc-1) + x_loc]       = 1;
        tetris_stack[32*(y_loc+1) + x_loc]       = 1;
        
      } else if (orientation == 1) {
        tetris_stack[32*(y_loc-1) + x_loc]      = 1;
        tetris_stack[32*y_loc + (x_loc+1)]      = 1;
        tetris_stack[32*y_loc + (x_loc-1)]      = 1;
      
      } else if (orientation == 2) {
        tetris_stack[32*y_loc + (x_loc+1)]       = 1;
        tetris_stack[32*(y_loc-1) + x_loc]       = 1;
        tetris_stack[32*(y_loc+1) + x_loc]       = 1;
     
      } else if (orientation == 3) {
        tetris_stack[32*(y_loc+1) + x_loc]       = 1;
        tetris_stack[32*y_loc + (x_loc-1)]       = 1;
        tetris_stack[32*y_loc + (x_loc+1)]       = 1;
      }
    break;
    
    case 5:  // Z 
      tetris_stack[32*y_loc + x_loc]          = 1;
            
      if ( orientation == 0 ) {
        tetris_stack[32*y_loc + (x_loc-1)]       = 1;
        tetris_stack[32*(y_loc-1) + x_loc]       = 1;
        tetris_stack[32*(y_loc+1) + (x_loc-1)]   = 1;
        
      } else if (orientation == 1) {
        tetris_stack[32*(y_loc-1) + x_loc]      = 1;
        tetris_stack[32*y_loc + (x_loc+1)]      = 1;
        tetris_stack[32*(y_loc-1) + (x_loc-1)]  = 1;
      
      }    
    break;
    
    case 6:  // mirror Z 
      tetris_stack[32*y_loc + x_loc]          = 1;

      if ( orientation == 0 ) {
        tetris_stack[32*y_loc + (x_loc-1)]       = 1;
        tetris_stack[32*(y_loc+1) + x_loc]       = 1;
        tetris_stack[32*(y_loc-1) + (x_loc-1)]   = 1;
        
      } else if (orientation == 1) {
        tetris_stack[32*(y_loc+1) + x_loc]      = 1;
        tetris_stack[32*y_loc + (x_loc+1)]      = 1;
        tetris_stack[32*(y_loc+1) + (x_loc-1)]  = 1;
      
      }
  }
  
  if ( debug == 1 ) {
    
    Serial.println("current stack:");  
    for ( int16_t i; i<ledCount; i++ ) {
      if ( tetris_stack[i] > 0 ) Serial.println(i);
    }  
      
  }
  
}


// *************************************************************************************************************************************
// move current part based on input from controller
// this involves checking where the new part wil be moved/rotated to before allowing the move to occurr

void movepart(byte type, byte *part_o, byte *prev_o, byte *partloc_x, byte *partloc_y, byte up, byte down, byte left, byte right, byte a, byte b, byte topstop, byte bottomstop, byte rightstop, byte leftstop, byte *tetris_stack, byte debug, int16_t ledCount, unsigned long *nesButtonMillis) {
  
 if(down == 0 ) {
    if ( *partloc_y < bottomstop && check_stack(type, *part_o, *partloc_y+1, *partloc_x, tetris_stack, debug, ledCount) ) *partloc_y = *partloc_y + 1;
  } else if ( right == 0 ) {
    if ( *partloc_x > rightstop && check_stack(type, *part_o, *partloc_y, *partloc_x-1, tetris_stack, debug, ledCount) ) *partloc_x = *partloc_x - 1;
  } else if ( left == 0 ) {
    if ( *partloc_x < leftstop && check_stack(type, *part_o, *partloc_y, *partloc_x+1, tetris_stack, debug, ledCount) )  *partloc_x = *partloc_x + 1;
  } 
    
  if ( debug == 1 && up == 0 ) {
    if ( *partloc_y > topstop ) *partloc_y = *partloc_y - 1;
  }
    
    
  // **** need to correct these for vertical orientation
  //rotate part based on controller input, check that new part orientation does not conflict with current stack before allowing rotation to occur
  if(a == 0 && ( millis() - *nesButtonMillis > 150 ) ) {
    switch ( type ) {
      case 1:
        if ( *part_o == 0 && *partloc_y > 1 && *partloc_y < 31 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount)  ) { 
          *prev_o = *part_o;
          *part_o = 1; 
        } else if ( *part_o == 1 && *partloc_x > 1 && *partloc_x < 15 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        }
      break; 
      
      case 2:
        if (*part_o == 0 && *partloc_x > 0 && *partloc_x < 15 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 1;
        } else if ( *part_o == 1 && *partloc_y > 0 && *partloc_y < 31 && check_stack(type, 2, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 2;
        } else if ( *part_o == 2 && *partloc_x > 0 && check_stack(type, 3, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 3;
        } else if ( *part_o == 3 && *partloc_y > 0 && *partloc_y < 31 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        }
      break; 
      
      case 3:
        if (*part_o == 0 && *partloc_x < 15 && *partloc_x > 0 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 1;
        } else if ( *part_o == 1 && *partloc_y < 31 && check_stack(type, 2, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 2;
        } else if ( *part_o == 2 && *partloc_x > 0 && *partloc_x < 15 && check_stack(type, 3, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 3;
        } else if ( *part_o == 3 && *partloc_y > 0 && *partloc_y < 31 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        }
      break; 
      
      case 4:
        if (*part_o == 0 && *partloc_y < 31 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 1;
        } else if ( *part_o == 1 && *partloc_x < 15 && check_stack(type, 2, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 2;
        } else if ( *part_o == 2 && *partloc_y > 0 && check_stack(type, 3, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 3;
        } else if ( *part_o == 3 && *partloc_x > 0 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        }
      break; 
      
      case 5:
        if ( *part_o == 0 && *partloc_y < 31 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) { 
          *prev_o = *part_o;
          *part_o = 1; 
        } else if ( *part_o == 1 && *partloc_x < 15 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        }
      break;  
      
      case 6:
        if ( *part_o == 0 && *partloc_y < 31 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) { 
          *prev_o = *part_o;
          *part_o = 1; 
        } else if ( *part_o == 1 && *partloc_x > 0 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        }
      break; 
             
    }
    *nesButtonMillis = millis();
      
  } else if ( b == 0 && ( millis() - *nesButtonMillis > 150 ) ) {
    switch ( type ) {
      case 1:
        if ( *part_o == 0 && *partloc_y > 1 && *partloc_y < 31 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) { 
          *prev_o = *part_o;
          *part_o = 1; 
        } else if ( *part_o == 1 && *partloc_x > 1 && *partloc_x < 15 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        }
      break; 
       
      case 2:
        if (*part_o == 0 && *partloc_x < 15 && check_stack(type, 3, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 3;
        } else if ( *part_o == 1 && *partloc_y > 0 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        } else if ( *part_o == 2 && *partloc_x > 0 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 1;
        } else if ( *part_o == 3 && *partloc_y < 31 && check_stack(type, 2, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 2;
        }
      break; 
      
      case 3:
        if (*part_o == 0 && *partloc_x > 0 && check_stack(type, 3, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 3;
        } else if ( *part_o == 1 && *partloc_y < 31 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        } else if ( *part_o == 2 && *partloc_x < 15 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 1;
        } else if ( *part_o == 3 && *partloc_y > 0 && check_stack(type, 2, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 2;
        }
      break; 
      
      case 4:
        if (*part_o == 0 && *partloc_y < 31 && check_stack(type, 3, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 3;
        } else if ( *part_o == 1 && *partloc_x < 15 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        } else if ( *part_o == 2 && *partloc_y > 0 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 1;
        } else if ( *part_o == 3 && *partloc_x > 0 && check_stack(type, 2, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 2;
        }
      break; 
      
      case 5:
        if ( *part_o == 0 && *partloc_y < 31 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) { 
          *prev_o = *part_o;
          *part_o = 1; 
        } else if ( *part_o == 1 && *partloc_x < 15 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        }
      break;  
      
      case 6:
        if ( *part_o == 0 && *partloc_y < 31 && check_stack(type, 1, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) { 
          *prev_o = *part_o;
          *part_o = 1; 
        } else if ( *part_o == 1 && *partloc_x > 0 && check_stack(type, 0, *partloc_y, *partloc_x, tetris_stack, debug, ledCount) ) {
          *prev_o = *part_o;
          *part_o = 0;
        }
      break; 
         
    }
    *nesButtonMillis = millis();
  }
}
  

// *************************************************************************************************************************************
// helper to draw a given tetris part at the provided location, input is type, x and y coordinates
// **REMEMBER: the x_loc and y_loc bytes used in the function were passed in as the y_loc and x_loc of the parts respectivly
void drawpart(byte type, byte orientation, byte x_loc, byte y_loc, int16_t clr, int16_t *display_mem, const int16_t *partcolor) {

  //draw different parts based on part type 
  switch (type) {
    case 0:   // 2x2 square
      display_mem[32*y_loc + x_loc ]         = partcolor[type] & clr;
      display_mem[32*y_loc + (x_loc+1) ]     = partcolor[type] & clr;
      display_mem[32*(y_loc-1) + x_loc ]     = partcolor[type] & clr;
      display_mem[32*(y_loc-1) + (x_loc+1) ] = partcolor[type] & clr;
    break;
    
    case 1:  // 4x1 rod
      display_mem[32*y_loc + x_loc]          = partcolor[type] & clr;
      
      if ( orientation == 0 ) {
        display_mem[32*(y_loc+1) + x_loc]    = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + x_loc]    = partcolor[type] & clr;
        display_mem[32*(y_loc-2) + x_loc]    = partcolor[type] & clr;
        /*
        display_mem[32*y_loc + (x_loc-1)]    = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc+1)]    = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc+2)]    = partcolor[type] & clr;
        */
      } else {
        display_mem[32*y_loc + (x_loc-1)]    = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc-2)]    = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc+1)]    = partcolor[type] & clr;
        /*
        display_mem[32*(y_loc+1) + x_loc]    = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + x_loc]    = partcolor[type] & clr;
        display_mem[32*(y_loc-2) + x_loc]    = partcolor[type] & clr;
        */
      }
    break;
    
    case 2:  // L 
      display_mem[32*y_loc + x_loc]          = partcolor[type] & clr;
      
      if ( orientation == 0 ) {
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc+1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + (x_loc+1)]   = partcolor[type] & clr;
        /*
        display_mem[32*(y_loc+1) + x_loc]      = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + x_loc]      = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + (x_loc+1)]  = partcolor[type] & clr;
        */
      } else if (orientation == 1) {
        display_mem[32*(y_loc+1) + x_loc]      = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + x_loc]      = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + (x_loc+1)]  = partcolor[type] & clr;
        /*
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc+1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + (x_loc+1)]   = partcolor[type] & clr;
        */
  
      } else if (orientation == 2) {
        display_mem[32*y_loc + (x_loc+1)]       = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + (x_loc-1)]   = partcolor[type] & clr; 
        /*
        display_mem[32*(y_loc-1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + (x_loc-1)]   = partcolor[type] & clr;
        */
      } else if (orientation == 3) {
        display_mem[32*(y_loc-1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + (x_loc-1)]   = partcolor[type] & clr;
        /*
        display_mem[32*y_loc + (x_loc+1)]       = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + (x_loc-1)]   = partcolor[type] & clr;
        */
      }
      break;
      
    case 3:  // mirror L 
      display_mem[32*y_loc + x_loc]          = partcolor[type] & clr;
      
      if ( orientation == 0 ) {
        display_mem[32*y_loc + (x_loc+1)]       = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + (x_loc+1)]   = partcolor[type] & clr;
        /*
        display_mem[32*(y_loc+1) + x_loc]      = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + x_loc]      = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + (x_loc-1)]  = partcolor[type] & clr;
        */
      } else if (orientation == 1) {
        display_mem[32*(y_loc+1) + x_loc]      = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + x_loc]      = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + (x_loc-1)]  = partcolor[type] & clr;
        /*
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc+1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + (x_loc-1)]   = partcolor[type] & clr;
        */
      } else if (orientation == 2) {
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc+1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + (x_loc-1)]   = partcolor[type] & clr;
        /*
        display_mem[32*(y_loc-1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + (x_loc+1)]   = partcolor[type] & clr;
        */
      } else if (orientation == 3) {
        display_mem[32*(y_loc-1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + (x_loc+1)]   = partcolor[type] & clr;
        /*
        display_mem[32*y_loc + (x_loc+1)]       = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + (x_loc+1)]   = partcolor[type] & clr;
        */
      }
    break;
    
    case 4:  // T 
      display_mem[32*y_loc + x_loc]          = partcolor[type] & clr;
      
      if ( orientation == 0 ) {
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + x_loc]       = partcolor[type] & clr;
        
      } else if (orientation == 1) {
        display_mem[32*(y_loc-1) + x_loc]      = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc+1)]      = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc-1)]      = partcolor[type] & clr;
      
      } else if (orientation == 2) {
        display_mem[32*y_loc + (x_loc+1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + x_loc]       = partcolor[type] & clr;
     
      } else if (orientation == 3) {
        display_mem[32*(y_loc+1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc+1)]       = partcolor[type] & clr;
      }
    break;
    
    case 5:  // Z 
      display_mem[32*y_loc + x_loc]          = partcolor[type] & clr;
            
      if ( orientation == 0 ) {
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + (x_loc-1)]   = partcolor[type] & clr;
        
      } else if (orientation == 1) {
        display_mem[32*(y_loc-1) + x_loc]      = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc+1)]      = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + (x_loc-1)]  = partcolor[type] & clr;
      
      }    
    break;
    
    case 6:  // mirror Z 
      display_mem[32*y_loc + x_loc]          = partcolor[type] & clr;

      if ( orientation == 0 ) {
        display_mem[32*y_loc + (x_loc-1)]       = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + x_loc]       = partcolor[type] & clr;
        display_mem[32*(y_loc-1) + (x_loc-1)]   = partcolor[type] & clr;
        
      } else if (orientation == 1) {
        display_mem[32*(y_loc+1) + x_loc]      = partcolor[type] & clr;
        display_mem[32*y_loc + (x_loc+1)]      = partcolor[type] & clr;
        display_mem[32*(y_loc+1) + (x_loc-1)]  = partcolor[type] & clr;
      
      }
  }
      
}

// *************************************************************************************************************************************
//helper to manange moving part location down 1 line, is dependant upon part type, location, and orientation
byte droppart(byte bottomstop, byte part_type, byte part_o, byte *partloc_y, byte partloc_x, byte *tetris_stack, byte debug, int16_t ledCount ) {

  byte return_flag = 1;
  
  if ( check_stack(part_type, part_o, *partloc_y+1, partloc_x, tetris_stack, debug, ledCount) ) {  // first check if new part position will overlap current tetris stack   !!!!NOTE partloc_y and partloc_x swapped for function because board is rotated!!!
  
    if ( *partloc_y < bottomstop ) {   // if current stack is clear, next check if part is at the bottom
      //partloc_y++;
      *partloc_y = *partloc_y + 1;
    } else {
      return_flag = 0;
    }
  } else {
    return_flag = 0;
  }

  return return_flag;  
}


// *************************************************************************************************************************************
// check stack function is used to manage stacking up the pieces and ensuring no overlap
// this function is basically the same as drawpart but will keep part locations in seperate memory from display memory
// it should make clearing rows easier later...
bool check_stack(byte type, byte orientation, byte x_loc, byte y_loc, byte *tetris_stack, byte debug, int16_t ledCount) {
  
  if ( debug == 1 ) {
    Serial.println("check_stack");
  }
  
  boolean return_flag = true;
  
  // temp storage for where new part location would be if it were allowed to move to this location
  int16_t newloc0, newloc1, newloc2, newloc3 = 0;  
    
  //draw different parts based on part type 
  switch (type) {
    case 0:   // 2x2 square
      newloc0 = 32*y_loc + x_loc;
      newloc1 = 32*y_loc + (x_loc+1);    
      newloc2 = 32*(y_loc-1) + x_loc;    
      newloc3 = 32*(y_loc-1) + (x_loc+1);
    break;
    
    case 1:  // 4x1 rod
      newloc0 = (32*y_loc + x_loc );
            
      if ( orientation == 0 ) {
        newloc1 = 32*(y_loc+1) + x_loc;
        newloc2 = 32*(y_loc-1) + x_loc;
        newloc3 = 32*(y_loc-2) + x_loc;
        
      } else {
        newloc1 = 32*y_loc + (x_loc-1);
        newloc2 = 32*y_loc + (x_loc-2);
        newloc3 = 32*y_loc + (x_loc+1);
        
      }
    break;
    
    case 2:  // L 
      newloc0 = (32*y_loc + x_loc );
      
      if ( orientation == 0 ) {
        newloc1 = 32*y_loc + (x_loc-1);
        newloc2 = 32*y_loc + (x_loc+1);
        newloc3 = 32*(y_loc-1) + (x_loc+1);
       
      } else if (orientation == 1) {
        newloc1 = 32*(y_loc+1) + x_loc;
        newloc2 = 32*(y_loc-1) + x_loc;
        newloc3 = 32*(y_loc+1) + (x_loc+1);
       
  
      } else if (orientation == 2) {
        newloc1 = 32*y_loc + (x_loc+1);
        newloc2 = 32*y_loc + (x_loc-1);
        newloc3 = 32*(y_loc+1) + (x_loc-1); 
        
      } else if (orientation == 3) {
        newloc1 = 32*(y_loc-1) + x_loc;
        newloc2 = 32*(y_loc+1) + x_loc;
        newloc3 = 32*(y_loc-1) + (x_loc-1);
        
      }
      break;
      
    case 3:  // mirror L 
      newloc0 = (32*y_loc + x_loc );
      
      if ( orientation == 0 ) {
        newloc1 = 32*y_loc + (x_loc+1);
        newloc2 = 32*y_loc + (x_loc-1);
        newloc3 = 32*(y_loc+1) + (x_loc+1);
        
      } else if (orientation == 1) {
        newloc1 = 32*(y_loc+1) + x_loc;
        newloc2 = 32*(y_loc-1) + x_loc;
        newloc3 = 32*(y_loc+1) + (x_loc-1);
        
      } else if (orientation == 2) {
        newloc1 = 32*y_loc + (x_loc-1);
        newloc2 = 32*y_loc + (x_loc+1);
        newloc3 = 32*(y_loc-1) + (x_loc-1);
        
      } else if (orientation == 3) {
        newloc1 = 32*(y_loc-1) + x_loc;
        newloc2 = 32*(y_loc+1) + x_loc;
        newloc3 = 32*(y_loc-1) + (x_loc+1);
        
      }
    break;
    
    case 4:  // T 
      newloc0 = (32*y_loc + x_loc );
      
      if ( orientation == 0 ) {
        newloc1 = 32*y_loc + (x_loc-1);
        newloc2 = 32*(y_loc-1) + x_loc;
        newloc3 = 32*(y_loc+1) + x_loc;
        
      } else if (orientation == 1) {
        newloc1 = 32*(y_loc-1) + x_loc;
        newloc2 = 32*y_loc + (x_loc+1);
        newloc3 = 32*y_loc + (x_loc-1);
      
      } else if (orientation == 2) {
        newloc1 = 32*y_loc + (x_loc+1);
        newloc2 = 32*(y_loc-1) + x_loc;
        newloc3 = 32*(y_loc+1) + x_loc;
     
      } else if (orientation == 3) {
        newloc1 = 32*(y_loc+1) + x_loc;
        newloc2 = 32*y_loc + (x_loc-1);
        newloc3 = 32*y_loc + (x_loc+1);
      }
    break;
    
    case 5:  // Z 
      newloc0 = (32*y_loc + x_loc );
            
      if ( orientation == 0 ) {
        newloc1 = 32*y_loc + (x_loc-1);
        newloc2 = 32*(y_loc-1) + x_loc;
        newloc3 = 32*(y_loc+1) + (x_loc-1);
        
      } else if (orientation == 1) {
        newloc1 = 32*(y_loc-1) + x_loc;
        newloc2 = 32*y_loc + (x_loc+1);
        newloc3 = 32*(y_loc-1) + (x_loc-1);
      
      }    
    break;
    
    case 6:  // mirror Z 
      newloc0 = (32*y_loc + x_loc );

      if ( orientation == 0 ) {
        newloc1 = 32*y_loc + (x_loc-1);
        newloc2 = 32*(y_loc+1) + x_loc;
        newloc3 = 32*(y_loc-1) + (x_loc-1);
        
      } else if (orientation == 1) {
        newloc1 = 32*(y_loc+1) + x_loc;
        newloc2 = 32*y_loc + (x_loc+1);
        newloc3 = 32*(y_loc+1) + (x_loc-1);
      
      }
  }
  
  // once the newlocs are set for the piece to check, check current tetris stack if any of the new locations would conflict with the current stack
  // if no conflicts are found, return true, otherwise false
    
  // check each newloc location in the stack, if it's nonzero, there would be a conflict so the piece could not be moved to that location
  if ( tetris_stack[newloc0] > 0 || tetris_stack[newloc1] > 0 || tetris_stack[newloc2] > 0 || tetris_stack[newloc3] > 0 ) {
    return_flag = false;
  } else {
    return_flag = true;
  }
  
  if ( debug == 1 ) {
    Serial.print("newloc: ");
    Serial.print(newloc0);
    Serial.print(" ");
    Serial.print(newloc1);
    Serial.print(" ");
    Serial.print(newloc2);
    Serial.print(" ");
    Serial.println(newloc3);
     
    for ( int16_t i; i<ledCount; i++ ) {
      if ( tetris_stack[i] > 0 ) Serial.println(i);
    } 
         
  }
  
  return return_flag;
  
}

// *************************************************************************************************************************************
// check for completed rows in the tetris stack.  if a row has been completed, play the clear animation for that row and then move everything above it in the stack down one row
void checkrows(byte partstart_y, byte *tetris_stack, int16_t *display_mem, int16_t *score, int16_t *fall_rate) {

  // traverse stack from top to bottom to search for completed row(s)
  for (byte row=partstart_y; row<32; row++ ) {
    
    byte row_full = 1;
    // for the selected row, traverse the row and check if any locations in tetris stack are zero.  if they are, then the row is not full
    for( byte i=0; i<16; i++ ) {
      if ( tetris_stack[row + (32*i)] == 0 ) row_full = 0;
    }
    
    // once row check is complete, if the current row is in fact full, clear it out and drop above pieces down
    if ( row_full == 1 ) {
  
      *score = *score + 1;
           
      // play animation to clear current completed row
      clearrow(row, tetris_stack, display_mem);
      
      // now traverse stack from current cleared row upwards and drop everything down a level ( tetris_stack and display_mem )
      for( byte j=row; j>=partstart_y; j--) {
        
        // for current drop-row, move data from next higher row down to current row
        for( byte k=0; k<16; k++) {
          tetris_stack[j + (32*k)] = tetris_stack[(j-1) + (32*k)];
          display_mem[j + (32*k)] = display_mem[(j-1) + (32*k)];
        }
      }
      
      // recompute fall rate
      *fall_rate = constrain( 500 - ( (*score/10) * 50 ), 80, 500 );
          
    }
  }
      
}



// *************************************************************************************************************************************
// play row clear animation
void clearrow(byte rowID, byte *tetris_stack, int16_t *display_mem ) {

  unsigned long clearPreviousFrame = millis();
    
  // currently just quick blank row,  can add fancy animation stuff later
  byte i=0;
  for ( i=0; i<8; i++ ) {
    tetris_stack[rowID + (32*(7-i))] = 0;
    tetris_stack[rowID + (32*(8+i))] = 0;
    display_mem[rowID + (32*(7-i))] = 0;
    display_mem[rowID + (32*(8+i))] = 0;
    
    while ( millis() - clearPreviousFrame < 50 ) {
      // do nothing, this is just to wait a bit
    }
    
    clearPreviousFrame = millis();
  }
}


// *************************************************************************************************************************************
// print the digits for the score
void printdigit(byte digit, byte value, int16_t *display_mem) {
  
  int16_t score_color = 56;
  int16_t y_start, x_start = 4;

  // establish starting location to draw score based on which digit position to draw
  if ( digit == 0 ) {
    y_start = 13;
  } else if ( digit == 1 ) {
    y_start = 9;
  } else if ( digit == 2 ) {
    y_start = 5;
  }
  
  // now draw digit at selected location based on what it's value is
  switch ( value ) {
    
     case 0:
       if ( digit == 2 ) {
         //display_mem[(32*y_start) + x_start ]           = score_color;
         display_mem[(32*y_start) + (x_start-1) ]       = score_color;
         display_mem[(32*y_start) + (x_start-2) ]       = score_color;
         display_mem[(32*y_start) + (x_start-3) ]       = score_color;
         //display_mem[(32*y_start) + (x_start-4) ]       = score_color;
         display_mem[(32*(y_start+1)) + (x_start-4) ]   = score_color;
         display_mem[(32*(y_start+1)) + x_start ]       = score_color;
         //display_mem[(32*(y_start+2)) + x_start ]       = score_color;
         display_mem[(32*(y_start+2)) + (x_start-1) ]   = score_color;
         display_mem[(32*(y_start+2)) + (x_start-2) ]   = score_color;
         display_mem[(32*(y_start+2)) + (x_start-3) ]   = score_color;
         //display_mem[(32*(y_start+2)) + (x_start-4) ]   = score_color;
      }
    break;
    
    case 1:
      display_mem[(32*(y_start+1)) + x_start ]           = score_color;
      display_mem[(32*(y_start+1)) + (x_start-1) ]       = score_color;
      display_mem[(32*(y_start+1)) + (x_start-2) ]       = score_color;
      display_mem[(32*(y_start+1)) + (x_start-3) ]       = score_color;
      display_mem[(32*(y_start+1)) + (x_start-4) ]       = score_color;
    break;
    
    case 2:
      display_mem[(32*y_start) + x_start ]           = score_color;
      display_mem[(32*y_start) + (x_start-2) ]       = score_color;
      display_mem[(32*y_start) + (x_start-3) ]       = score_color;
      //display_mem[(32*y_start) + (x_start-4) ]       = score_color;
      display_mem[(32*(y_start+1)) + (x_start-4) ]   = score_color;
      display_mem[(32*(y_start+1)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+1)) + x_start ]       = score_color;
      display_mem[(32*(y_start+2)) + x_start ]       = score_color;
      display_mem[(32*(y_start+2)) + (x_start-1) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-4) ]   = score_color;
    break;
  
    case 3:
      display_mem[(32*y_start) + x_start ]           = score_color;
      display_mem[(32*y_start) + (x_start-1) ]       = score_color;
      display_mem[(32*y_start) + (x_start-2) ]       = score_color;
      display_mem[(32*y_start) + (x_start-3) ]       = score_color;
      display_mem[(32*y_start) + (x_start-4) ]       = score_color;
      display_mem[(32*(y_start+1)) + (x_start-4) ]   = score_color;
      display_mem[(32*(y_start+1)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+1)) + x_start ]       = score_color;
      display_mem[(32*(y_start+2)) + x_start ]       = score_color;
      display_mem[(32*(y_start+2)) + (x_start-4) ]   = score_color;
    break;
    
    case 4:
      display_mem[(32*y_start) + x_start ]           = score_color;
      display_mem[(32*y_start) + (x_start-1) ]       = score_color;
      display_mem[(32*y_start) + (x_start-2) ]       = score_color;
      display_mem[(32*y_start) + (x_start-3) ]       = score_color;
      display_mem[(32*y_start) + (x_start-4) ]       = score_color;
      display_mem[(32*(y_start+1)) + (x_start-2) ]   = score_color;
      //display_mem[(32*(y_start+2)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-3) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-4) ]   = score_color;
    break;
    
    case 5:
      //display_mem[(32*y_start) + x_start ]           = score_color;
      display_mem[(32*y_start) + (x_start-1) ]       = score_color;
      display_mem[(32*y_start) + (x_start-2) ]       = score_color;
      display_mem[(32*y_start) + (x_start-4) ]       = score_color;
      display_mem[(32*(y_start+1)) + x_start ]       = score_color;
      display_mem[(32*(y_start+1)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+1)) + (x_start-4) ]   = score_color;
      display_mem[(32*(y_start+2)) + x_start ]       = score_color;
      display_mem[(32*(y_start+2)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-3) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-4) ]   = score_color;
    break;
    
    case 6:
      display_mem[(32*y_start) + x_start ]           = score_color;
      display_mem[(32*y_start) + (x_start-1) ]       = score_color;
      display_mem[(32*y_start) + (x_start-2) ]       = score_color;
      display_mem[(32*y_start) + (x_start-4) ]       = score_color;
      display_mem[(32*(y_start+1)) + x_start ]       = score_color;
      display_mem[(32*(y_start+1)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+1)) + (x_start-4) ]   = score_color;
      display_mem[(32*(y_start+2)) + x_start ]       = score_color;
      display_mem[(32*(y_start+2)) + (x_start-1) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-3) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-4) ]   = score_color;
    break;
    
    case 7:
      display_mem[(32*y_start) + x_start ]           = score_color;
      display_mem[(32*y_start) + (x_start-1) ]       = score_color;
      display_mem[(32*y_start) + (x_start-2) ]       = score_color;
      display_mem[(32*y_start) + (x_start-3) ]       = score_color;
      display_mem[(32*y_start) + (x_start-4) ]       = score_color;
      display_mem[(32*(y_start+1)) + (x_start-4) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-4) ]   = score_color;
    break;
    
    case 8:
      display_mem[(32*y_start) + x_start ]           = score_color;
      display_mem[(32*y_start) + (x_start-1) ]       = score_color;
      display_mem[(32*y_start) + (x_start-2) ]       = score_color;
      display_mem[(32*y_start) + (x_start-3) ]       = score_color;
      display_mem[(32*y_start) + (x_start-4) ]       = score_color;
      display_mem[(32*(y_start+1)) + x_start ]       = score_color;
      display_mem[(32*(y_start+1)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+1)) + (x_start-4) ]   = score_color;
      display_mem[(32*(y_start+2)) + x_start ]       = score_color;
      display_mem[(32*(y_start+2)) + (x_start-1) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-3) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-4) ]   = score_color;
    break;
    
    case 9:
      display_mem[(32*y_start) + x_start ]           = score_color;
      display_mem[(32*y_start) + (x_start-1) ]       = score_color;
      display_mem[(32*y_start) + (x_start-2) ]       = score_color;
      display_mem[(32*y_start) + (x_start-3) ]       = score_color;
      display_mem[(32*y_start) + (x_start-4) ]       = score_color;
      display_mem[(32*(y_start+1)) + x_start ]       = score_color;
      display_mem[(32*(y_start+1)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+1)) + (x_start-4) ]   = score_color;
      display_mem[(32*(y_start+2)) + x_start ]       = score_color;
      display_mem[(32*(y_start+2)) + (x_start-2) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-3) ]   = score_color;
      display_mem[(32*(y_start+2)) + (x_start-4) ]   = score_color;
    break;
    
  }
  
}

