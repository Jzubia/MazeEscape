// Switch.cpp
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

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

void PortE_init(void){
  volatile uint32_t delay;
  SYSCTL_RCGCGPIO_R |= 0x10;    // activate clock for Port E
  delay = SYSCTL_RCGCGPIO_R;    // wait for clock to stabilize
  delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTE_DIR_R &= ~(0x0F);
	GPIO_PORTE_DEN_R |= 0x0F;
}

bool playPressed(){
	if((GPIO_PORTE_DATA_R & 0x01) == 1){
		while((GPIO_PORTE_DATA_R & 0x01) == 1){}
		return true;
	}
	return false;
}

int langSelect(){
	if((GPIO_PORTE_DATA_R & 0x01) == 1){
		while((GPIO_PORTE_DATA_R & 0x01) == 1){}
		return 1; // for Spanish
	} else if ((GPIO_PORTE_DATA_R & 0x04) > 1) {
		while((GPIO_PORTE_DATA_R & 0x04) > 1){}
		return 0; // for English
	} else {
		return 2; // No button pressed
	}
	
}

bool checkFire(){
	if((GPIO_PORTE_DATA_R & 0x04) > 0){
		return true;
	}
	return false;
}
