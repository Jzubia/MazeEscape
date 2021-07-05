// SlidePot.cpp
// Runs on TM4C123
// Provide functions that initialize ADC0
// copy code from Lab 8 or Lab 9
// Last Modified: 1/16/2021 
// Student names: Jason Zubia & Anthony Liu solutions

#include <stdint.h>
#include "SlidePot.h"
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
// lab 8
	SYSCTL_RCGCGPIO_R |= 0x08;
	while((SYSCTL_RCGCGPIO_R&0x08) == 0){
	}
	volatile uint32_t delay;
	GPIO_PORTD_DIR_R &= ~0x04;
	GPIO_PORTD_AFSEL_R |= 0x04;
	GPIO_PORTD_DEN_R &= ~0x04;
	GPIO_PORTD_AMSEL_R |= 0x04;
	SYSCTL_RCGCADC_R |= 0x01;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	ADC0_PC_R = 0x01;
	ADC0_SSPRI_R = 0x0123;
	ADC0_ACTSS_R &= ~0x0008;
	ADC0_EMUX_R &= ~0xF000;
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+5;
	ADC0_SSCTL3_R = 0x0006;
	ADC0_IM_R &= ~0x0008;
	ADC0_ACTSS_R |= 0x0008;
}

//------------ADCIn------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  

// lab 8
  uint32_t data;
	ADC0_PSSI_R = 0x0008;
	while((ADC0_RIS_R&0x08)==0){};
	data = ADC0_SSFIFO3_R&0xfff;
	ADC0_ISC_R = 0x0008;
	return data;
}

// constructor, invoked on creation of class
// m and b are linear calibration coeffients 
SlidePot::SlidePot(uint32_t m, uint32_t b){
// lab 8
	slope = m;
	offset = b;
}

void SlidePot::Save(uint32_t n){
// lab 9
	data = n;
	distance = Convert(data);
	flag = 1;
}
uint32_t SlidePot::Convert(uint32_t n){
  // lab 8
  return (166*n)/4096+15; // replace this with solution
}

void SlidePot::Sync(void){
// lab 8
	while(flag==0);
	flag=0;
}

uint32_t SlidePot::ADCsample(void){ // return ADC sample value (0 to 4095)

  return data;
}

uint32_t SlidePot::Distance(void){  // return distance value (0 to 2000), 0.001cm
// lab 8
  return distance;
}