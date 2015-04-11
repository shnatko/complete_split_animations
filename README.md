# complete_split_animations
arduino code for LED table

This is the full set of arduino code for my interactive RGB LED matrix table.

The repository consists of 7 files:

complete_split_animations.ino 
 - main arduino source file w/ all the globals, setup and loop functions and timers/interrupts for driving the LED panel, IR sensors and NES controllers
 
animations.h / animations.cpp
- code for all the various animations the table runs

tetris.h / tetris.cpp
- code specifically for the tetris game

pgm_memory.h / pgm_memory.cpp
- code for storing fixed data in flash memory for things like text scrolling and calibration settings, etc..

Details on hardware build and code operation can be found at  http://shnatko.tumblr.com
 
