// Switch.h
// This software to input from switches or buttons
// Runs on TM4C123
// Program written by: Anthony Liu & Jason Zubia
// Date Created: 3/6/17 
// Last Modified: 5/3/21
// Lab number: 10
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********
// PE0 for Start Button & Spanish Language select
// PE2 for Shoot Button & English Language select
#ifndef _Switch_h
#define _Switch_h
#include <stdint.h>

// Header files contain the prototypes for public functions 
// this file explains what the module does
void PortE_init(void);

bool playPressed();

int langSelect();

bool checkFire();
#endif

