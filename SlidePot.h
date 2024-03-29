// SlidePot.h
// Runs on TM4C123
// Provide functions that initialize ADC0 and use a slide pot to measure distance
// Created: 1/14/2021 
// Student names: Jason Zubia & Anthony Liu
// Last modification date: 5/3/2021


#ifndef SLIDEPOT_H
#define SLIDEPOT_H
#include <stdint.h>

class SlidePot{ private:
  uint32_t data;     // raw ADC value
  int32_t flag;      // 0 if data is not valid, 1 if valid
  uint32_t distance; // distance in 0.01cm

// distance = (slope*data+offset)/4096
  uint32_t slope;    // calibration coeffients
  uint32_t offset;
public:
  SlidePot(uint32_t m, uint32_t b); // initialize slide pot
  void Save(uint32_t n);        // save ADC, set semaphore
  void Sync(void);              // wait for semaphore
  uint32_t Convert(uint32_t n); // convert ADC to raw sample
  uint32_t ADCsample(void);     // return ADC sample value (0 to 4095)
  uint32_t Distance(void);      // return distance value (0 to 200), 0.01cm
};

// ADC initialization function, channel 5, PD2
// Input: none
// Output: none
void ADC_Init(void);

//------------ADC_In------------
// Busy-wait Analog to digital conversion, channel 5, PD2
// Input: none
// Output: 12-bit result of ADC conversion
uint32_t ADC_In(void);

#endif
