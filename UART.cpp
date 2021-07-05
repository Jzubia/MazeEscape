// UART.cpp
// Runs on LM4F120/TM4C123
// This code runs on UART0 with interrupts and a simple FIFO
// EE319K tasks
// 1) Convert to UART1 PC4 PC5
// 2) Implement the FIFO as a class
// 3) Run transmitter with busy-wait synchronization
// 4) Run receiver with 1/2 full FIFO interrupt
// Daniel and Jonathan Valvano
// April 11, 2018




// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1
// EE319K Lab 9, use U1Rx connected to PC4 interrupt
// EE319K Lab 9, use U1Tx connected to PC5 busy wait
// EE319K Lab 9 hardware
// System 1        System 2
//   PC4 ----<<----- PC5
//   PC5 ---->>----- PC4
//   Gnd ----------- Gnd
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#include "FIFO.h"
#include "UART.h"

#define PF1  (*((volatile uint32_t *)0x40025008))


extern "C" void UART1_Handler(void);
#define NVIC_EN0_INT6   0x00000040  // Interrupt 6 enable
Queue RxFifo;
// Initialize UART0
// Baud rate is 1000 bits/sec
// Lab 9
void UART_Init(void){
  
// lab 9
}


// input ASCII character from UART
// spin if RxFifo is empty
// Lab 9
char UART_InChar(void){
  char letter;
// lab 9
  return(letter);
}
// Lab 9
bool UART_InStatus(void){
  // true if input data ready
// lab 9
  return 0;
}

// output ASCII character to UART
// spin if hardware not ready
// Lab 9
// in Lab 9 this will never wait
void UART_OutChar(char data){
// lab 9
}
// one thing has happened:
// hardware RX FIFO goes from 7 to 28 or more items
// UART receiver has timed out
// Lab 9
void UART1_Handler(void){
  // lab 9
}



#endif
