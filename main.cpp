// main.cpp
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10 in C++

// Last Modified: 1/16/2021 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* 
 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 47k  resistor DAC bit 0 on PB0 (least significant bit)
// 24k  resistor DAC bit 1 on PB1
// 12k  resistor DAC bit 2 on PB2
// 6k   resistor DAC bit 3 on PB3 
// 3k   resistor DAC bit 4 on PB4 
// 1.5k resistor DAC bit 5 on PB5 (most significant bit)

// VCC   3.3V power to OLED
// GND   ground
// SCL   PD0 I2C clock (add 1.5k resistor from SCL to 3.3V)
// SDA   PD1 I2C data

//************WARNING***********
// The LaunchPad has PB7 connected to PD1, PB6 connected to PD0
// Option 1) do not use PB7 and PB6
// Option 2) remove 0-ohm resistors R9 R10 on LaunchPad
//******************************

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include "../inc/tm4c123gh6pm.h"
#include "TExaS.h"
#include "SSD1306.h"
#include "Random.h"
#include "Switch.h"
#include "Sound.h"
#include "SlidePot.h"
#include "Images.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Timer2.h"

//********************************************************************************
// debuging profile, pick up to 7 unused bits and send to Logic Analyzer
#define PA54                  (*((volatile uint32_t *)0x400040C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// use for debugging profile
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PA5       (*((volatile uint32_t *)0x40004080)) 
#define PA4       (*((volatile uint32_t *)0x40004040)) 
	
#define MAX 4
// TExaSdisplay logic analyzer shows 7 bits 0,PA5,PA4,PF3,PF2,PF1,0 
void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PA54; // sends at 10kHz
}
void ScopeTask(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
void Profile_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x21;      // activate port A,F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 
  GPIO_PORTF_DEN_R |=  0x0E;   // enable digital I/O on PF3,2,1
  GPIO_PORTA_DIR_R |=  0x30;   // output on PA4 PA5
  GPIO_PORTA_DEN_R |=  0x30;   // enable on PA4 PA5  
}

//void PortE_init(void){
//  volatile uint32_t delay;
//  SYSCTL_RCGCGPIO_R |= 0x10;    // activate clock for Port E
//  delay = SYSCTL_RCGCGPIO_R;    // wait for clock to stabilize
//  delay = SYSCTL_RCGCGPIO_R;
//	GPIO_PORTE_DIR_R &= ~(0x0F);
//	GPIO_PORTE_DEN_R |= 0x0F;
//}

//********************************************************************************
 
// **************SysTick_Init*********************
// Initialize Systick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(unsigned long period){
  //*** students write this ******
	NVIC_ST_CTRL_R = 0; // Disables 
	NVIC_ST_RELOAD_R = period; // Reload 
	NVIC_ST_CURRENT_R = 0; // Clears 
	NVIC_ST_CTRL_R = 7; // Enable Clock and Interrupts
}

void playercollision(void);
void playerexplode();

typedef enum {dead,alive} status_t;
class sprite{
	public:
	int32_t x;
	int32_t y;
	int32_t vx;
	int32_t vy;
	const uint8_t *image;
	status_t life;
};
sprite player;
sprite missile[18];
sprite enemy;
sprite enemy1;
sprite explosion;

int count = rand() % 800000;
int count1 = rand() % 800000;
int scoreCount = 0;

void init(void){int i;
	enemy.x = 126; 
	enemy.y = -12;
	enemy.image = PlayerShip0;
	enemy.life = alive;
	enemy.vx = -1;
	enemy.vy = 1;
	enemy1.x = 0; 
	enemy1.y = -12;
	enemy1.image = PlayerShip1;
	enemy1.life = alive;
	enemy1.vx = 1;
	enemy1.vy = 1;
	player.x = 50;
	player.y = 50;
	player.image = MazePlayer;
	player.life = alive;
	for(i=0; i<18; i++){
		missile[i].life = dead;
	}
}

// **************Maze Pieces Class*********************
class mazePiece{
	public:
		int32_t ypos1;
	  int32_t ypos2;
		int32_t ypos3;
	  int32_t ypos4;
		int32_t vy; 
	  const unsigned char *first; // bottom of maze piece
	  const unsigned char *second; // top of maze piece
		const unsigned char *third; // bottom of maze piece
	  const unsigned char *fourth; // top of maze piece
    
		mazePiece(const uint8_t *buff1, const uint8_t *buff2, const uint8_t *buff3, const uint8_t *buff4){
			ypos1 = 0;
			ypos2 = 0;
			ypos3 = 0;
			ypos4 = 0;
			vy = 1;
			first = buff1;
			second = buff2;
			third = buff3;
			fourth = buff4;
			
		}
		// mazePiece() : ypos1(0), ypos2(-30), vy(0), bottom(NULL), top(NULL){}	
		~mazePiece(){
			delete first;
			delete second;
			delete third;
			delete fourth;
		}	
};
int VY = 1;
class mazeSegment{
	public:
		int32_t ypos1;
	  int32_t ypos2;
		int32_t ypos3;
	  int32_t ypos4;
		int32_t ypos5;
	  int32_t ypos6;
		int32_t ypos7;
	  int32_t ypos8;
		int32_t ypos9;
	  int32_t ypos10;
		int32_t ypos11;
	  int32_t ypos12;
		int32_t ypos13;
	  int32_t ypos14;
		int32_t ypos15;
	  int32_t ypos16;
		int32_t vy; 
	  const unsigned char *first; // bottom of maze piece
	  const unsigned char *second; // top of maze piece
		const unsigned char *third; // bottom of maze piece
	  const unsigned char *fourth; // top of maze piece
		const unsigned char *fifth; // bottom of maze piece
	  const unsigned char *sixth; // top of maze piece
		const unsigned char *seventh; // bottom of maze piece
	  const unsigned char *eight; // top of maze piece
		const unsigned char *ninth; // bottom of maze piece
	  const unsigned char *tenth; // top of maze piece
		const unsigned char *eleventh; // bottom of maze piece
	  const unsigned char *twelveth; // top of maze piece
		const unsigned char *thirteenth; // bottom of maze piece
	  const unsigned char *fourteenth; // top of maze piece
		const unsigned char *fifteenth; // bottom of maze piece
	  const unsigned char *sixteenth; // top of maze piece
		
		mazeSegment(const uint8_t *buff1, const uint8_t *buff2, const uint8_t *buff3, const uint8_t *buff4, const uint8_t *buff5, const uint8_t *buff6, const uint8_t *buff7, const uint8_t *buff8, const uint8_t *buff9, const uint8_t *buff10, const uint8_t *buff11, const uint8_t *buff12, const uint8_t *buff13, const uint8_t *buff14, const uint8_t *buff15, const uint8_t *buff16){
			ypos1 = 0;
			ypos2 = 0;
			ypos3 = 0;
			ypos4 = 0;
			ypos5 = 0;
			ypos6 = 0;
			ypos7 = 0;
			ypos8 = 0;
			ypos9 = 0;
			ypos10 = 0;
			ypos11 = 0;
			ypos12 = 0;
			ypos13 = 0;
			ypos14 = 0;
			ypos15 = 0;
			ypos16 = 0;
			vy = VY;
			first = buff1;
			second = buff2;
			third = buff3;
			fourth = buff4;
			fifth = buff5;
			sixth = buff6;
			seventh = buff7;
			eight = buff8;
			ninth = buff9;
			tenth = buff10;
			eleventh = buff11;
			twelveth = buff12;
			thirteenth = buff13; // bottom of maze piece
			fourteenth = buff14; // top of maze pie
			fifteenth = buff15; // bottom of maze piece
			sixteenth = buff16; // top of maze piece
			
		}
		// mazePiece() : ypos1(0), ypos2(-30), vy(0), bottom(NULL), top(NULL){}	
		~mazeSegment(){
			delete first;
			delete second;
			delete third;
			delete fourth;
			delete fifth;
			delete sixth;
			delete seventh;
			delete eight;
			delete ninth;
			delete tenth;
			delete eleventh;
			delete twelveth;
			delete thirteenth;
			delete fourteenth;
			delete fifteenth;
			delete sixteenth;
		}
};


mazePiece piece1(MazePiece1_1, MazePiece1_2, MazePiece1_3, MazePiece1_4); // MazePiece1 Complete in one class
mazePiece piece2(MazePiece2_1, MazePiece2_2, MazePiece2_3, MazePiece2_4); // MazePiece2 Complete in one class
mazePiece piece3(MazePiece3_1, MazePiece3_2, MazePiece3_3, MazePiece3_4); // MazePiece2 Complete in one class
mazeSegment segment1(MazeSegment1_1, MazeSegment1_2, MazeSegment1_3, MazeSegment1_4, MazeSegment1_5, MazeSegment1_6, MazeSegment1_7, MazeSegment1_8, MazeSegment1_9, MazeSegment1_10, MazeSegment1_11, MazeSegment1_12, MazeSegment1_13, MazeSegment1_14, MazeSegment1_15, MazeSegment1_16);
mazeSegment segment2(MazeSegment2_1, MazeSegment2_2, MazeSegment2_3, MazeSegment2_4, MazeSegment2_5, MazeSegment2_6, MazeSegment2_7, MazeSegment2_8, MazeSegment2_9, MazeSegment2_10, MazeSegment2_11, MazeSegment2_12, MazeSegment2_13, MazeSegment2_14, MazeSegment2_15, MazeSegment2_16);
mazeSegment segment3(MazeSegment3_1, MazeSegment3_2, MazeSegment3_3, MazeSegment3_4, MazeSegment3_5, MazeSegment3_6, MazeSegment3_7, MazeSegment3_8, MazeSegment3_9, MazeSegment3_10, MazeSegment3_11, MazeSegment3_12, MazeSegment3_13, MazeSegment3_14, MazeSegment3_15, MazeSegment3_16);
mazeSegment segment4(MazeSegment4_1, MazeSegment4_2, MazeSegment4_3, MazeSegment4_4, MazeSegment4_5, MazeSegment4_6, MazeSegment4_7, MazeSegment4_8, MazeSegment4_9, MazeSegment4_10, MazeSegment4_11, MazeSegment4_12, MazeSegment4_13, MazeSegment4_14, MazeSegment4_15, MazeSegment4_16);
mazePiece MazePieces[] = {piece1, piece2, piece3};
mazeSegment MazeSegments[] = {segment3, segment4, segment1, segment2};
bool update = false;
int index = rand() % MAX;

void Draw(){
	SSD1306_ClearBuffer();
	SSD1306_DrawBMP(2, 62, Maze0, 0, SSD1306_WHITE); // background
	if(MazePieces[index].ypos4 == 80){
		MazePieces[index].ypos1 = 0;
		MazePieces[index].ypos2 = 0;
		MazePieces[index].ypos3 = 0;
		MazePieces[index].ypos4 = 0;
		index++;
		if(index == MAX){
			index = 0;
		}
	}
	SSD1306_DrawBMP(2, MazePieces[index].ypos1, MazePieces[index].first, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazePieces[index].ypos2, MazePieces[index].second, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazePieces[index].ypos3, MazePieces[index].third, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazePieces[index].ypos4, MazePieces[index].fourth, 0, SSD1306_WHITE); // maze walls 
	SSD1306_OutBuffer(); // output to LCD
	update = false; // Semaphore = 0
}

void draw(void){
	// SSD1306_ClearBuffer();
	if(enemy.life == alive){
		SSD1306_DrawBMP(enemy.x, enemy.y, enemy.image, 0, SSD1306_WHITE);
	}
	if(enemy1.life == alive){
		SSD1306_DrawBMP(enemy1.x, enemy1.y, enemy1.image, 0, SSD1306_WHITE);
	}
	for(int i=0; i<18; i++){
		if(missile[i].life){
			SSD1306_DrawBMP(missile[i].x, missile[i].y, missile[i].image, 0, SSD1306_INVERSE);
		}
	}
	playercollision();
	if(player.life == alive){
		SSD1306_DrawBMP(player.x, player.y, player.image, 0, SSD1306_INVERSE);
	}
	// SSD1306_OutBuffer();
}

void Draw1(){
	SSD1306_ClearBuffer();
	SSD1306_DrawBMP(2, 62, Maze0, 0, SSD1306_WHITE); // background
	if(MazeSegments[index].ypos16 == 74){
		MazeSegments[index].ypos1 = 0;
		MazeSegments[index].ypos2 = 0;
		MazeSegments[index].ypos3 = 0;
		MazeSegments[index].ypos4 = 0;
		MazeSegments[index].ypos5 = 0;
		MazeSegments[index].ypos6 = 0;
		MazeSegments[index].ypos7 = 0;
		MazeSegments[index].ypos8 = 0;
		MazeSegments[index].ypos9 = 0;
		MazeSegments[index].ypos10 = 0;
		MazeSegments[index].ypos11 = 0;
		MazeSegments[index].ypos12 = 0;
		MazeSegments[index].ypos13 = 0;
		MazeSegments[index].ypos14 = 0;
		MazeSegments[index].ypos15 = 0;
		MazeSegments[index].ypos16 = 0;
		int curr = index;
		while(curr == index){
			index = rand() % MAX;
		}
	}
	SSD1306_DrawBMP(2, MazeSegments[index].ypos1, MazeSegments[index].first, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos2, MazeSegments[index].second, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos3, MazeSegments[index].third, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos4, MazeSegments[index].fourth, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos5, MazeSegments[index].fifth, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos6, MazeSegments[index].sixth, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos7, MazeSegments[index].seventh, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos8, MazeSegments[index].eight, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos9, MazeSegments[index].ninth, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos10, MazeSegments[index].tenth, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos11, MazeSegments[index].eleventh, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos12, MazeSegments[index].twelveth, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos13, MazeSegments[index].thirteenth, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos14, MazeSegments[index].fourteenth, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos15, MazeSegments[index].fifteenth, 0, SSD1306_WHITE); // maze walls 
	SSD1306_DrawBMP(2, MazeSegments[index].ypos16, MazeSegments[index].sixteenth, 0, SSD1306_WHITE); // maze walls 
	draw();
	// playercollision();
	SSD1306_OutBuffer(); // output to LCD
	update = false; // Semaphore = 0
}



extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);
void Delay100ms(uint32_t count); // time delay in 0.1 seconds


int main1(void){uint32_t time=0;
  DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
  Random_Init(1);
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
	SysTick_Init(80000000/20); // 20Hz
  SSD1306_ClearBuffer();
	SSD1306_DrawBMP(2, 62, Maze_Escape_Main, 0, SSD1306_WHITE);
	SSD1306_OutBuffer();
  // SSD1306_DrawBMP(2, 62, SpaceInvadersMarquee, 0, SSD1306_WHITE);
  // SSD1306_OutBuffer();
  Delay100ms(20);
  SSD1306_ClearBuffer(); // clears buffer
	SSD1306_OutClear();
	EnableInterrupts();
  // SSD1306_DrawBMP(47, 63, PlayerShip0, 0, SSD1306_WHITE); // player ship bottom
  // SSD1306_DrawBMP(53, 55, Bunker0, 0, SSD1306_WHITE);

  // SSD1306_DrawBMP(0, 9, Alien10pointA, 0, SSD1306_WHITE);
  // SSD1306_DrawBMP(20,9, Alien10pointB, 0, SSD1306_WHITE);
  // SSD1306_DrawBMP(40, 9, Alien20pointA, 0, SSD1306_WHITE);
  // SSD1306_DrawBMP(60, 9, Alien20pointB, 0, SSD1306_WHITE);
  // SSD1306_DrawBMP(80, 9, Alien30pointA, 0, SSD1306_WHITE);
  // SSD1306_DrawBMP(50, 19, AlienBossA, 0, SSD1306_WHITE);
  // SSD1306_OutBuffer(); // 25ms
  // Delay100ms(30);
  // SSD1306_SetCursor(1, 1);
  // SSD1306_OutString((char *)"GAME OVER");
  // SSD1306_SetCursor(1, 2);
  // SSD1306_OutString((char *)"Nice try,");
  // SSD1306_SetCursor(1, 3);
  // SSD1306_OutString((char *)"Earthling!");
  // SSD1306_SetCursor(2, 4);
  while(1){
		if(update){
			// Draw();
			Draw1();
		}
		// SSD1306_OutBuffer();
		// Delay100ms(10);
		// i++;
    // Delay100ms(10);
    // SSD1306_SetCursor(19,0);
    // SSD1306_OutUDec2(time);
    // time++;
		
    PF2 ^= 0x04;
  }
	
}

void Move(){
	MazePieces[index].ypos1 += MazePieces[index].vy; // move maze down
	if(MazePieces[index].ypos1 >= 15){
		MazePieces[index].ypos2 += MazePieces[index].vy;
		if(MazePieces[index].ypos2 >= 13){
			MazePieces[index].ypos3 += MazePieces[index].vy; // move maze down
			if(MazePieces[index].ypos3 >= 15){
				MazePieces[index].ypos4 += MazePieces[index].vy;
			}
		}
	}
	update = true; 
}

void Move1(){
	MazeSegments[index].ypos1 += MazeSegments[index].vy; // move maze down
	if(MazeSegments[index].ypos1 >= 15){
		MazeSegments[index].ypos2 += MazeSegments[index].vy;
		if(MazeSegments[index].ypos2 >= 13){
			MazeSegments[index].ypos3 += MazeSegments[index].vy; // move maze down
			if(MazeSegments[index].ypos3 >= 15){
				MazeSegments[index].ypos4 += MazeSegments[index].vy;
				if(MazeSegments[index].ypos4 >= 13){
					MazeSegments[index].ypos5 += MazeSegments[index].vy;
					if(MazeSegments[index].ypos5 >= 15){
						MazeSegments[index].ypos6 += MazeSegments[index].vy; // move maze down
						if(MazeSegments[index].ypos6 >= 13){
							MazeSegments[index].ypos7 += MazeSegments[index].vy;
							if(MazeSegments[index].ypos7 >= 15){
								MazeSegments[index].ypos8 += MazeSegments[index].vy;
								if(MazeSegments[index].ypos8 >= 13){
									MazeSegments[index].ypos9 += MazeSegments[index].vy; // move maze down
									if(MazeSegments[index].ypos9 >= 15){
										MazeSegments[index].ypos10 += MazeSegments[index].vy;
										if(MazeSegments[index].ypos10 >= 13){
											MazeSegments[index].ypos11 += MazeSegments[index].vy;
											if(MazeSegments[index].ypos11 >= 15){
												MazeSegments[index].ypos12 += MazeSegments[index].vy; // move maze down
												if(MazeSegments[index].ypos12 >= 13){
													MazeSegments[index].ypos13 += MazeSegments[index].vy;
													if(MazeSegments[index].ypos13 >= 15){
														MazeSegments[index].ypos14 += MazeSegments[index].vy;
														if(MazeSegments[index].ypos14 >= 13){
															MazeSegments[index].ypos15 += MazeSegments[index].vy; // move maze down
															if(MazeSegments[index].ypos15 >= 15){
																MazeSegments[index].ypos16 += MazeSegments[index].vy;
															}
														}
													}
												}
											}	
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	update = true; 
}

void move(void){
	uint32_t ADCdata = ADC_In();
	player.x = (120*ADCdata)/4000;
	for(int i=0; i<18; i++){
		if(missile[i].life == alive){
			if(missile[i].x < 0){
				missile[i].x = 2;
				missile[i].vx = -missile[i].vx;
			}
			if(missile[i].x > 126){
				missile[i].x = 124;
				missile[i].vx = -missile[i].vx;
			}
			if((missile[i].y < 62) && (missile[i].y > 0)){
				missile[i].x += missile[i].vx;
				missile[i].y += missile[i].vy;
			}else{
				missile[i].life = dead;
			}
		}
	}
//	if(enemy.life == alive){
//			if(enemy.x < 0){
//				enemy.x = 2;
//				enemy.vx = -enemy.vx;
//			}
//			if(enemy.x > 126){
//				enemy.x = 124;
//				enemy.vx = -enemy.vx;
//			}
//			if((enemy.y < 62) && (enemy.y > 0)){
//				enemy.x += enemy.vx;
//				enemy.y += enemy.vy;
//			}else{
//				enemy.life = dead;
//				count = rand() % 100;
//			}
//		}
	
	if(enemy.life == alive){
		if(enemy.x < 0 || enemy.y > 62){
			enemy.life = dead;
			enemy.x = 126;
			enemy.y = -12;
			count = rand() % 800000;
			return;
		} else {
			enemy.x += enemy.vx;
			enemy.y += enemy.vy;
		}
	}
	if(enemy1.life == alive){
		if(enemy1.x < 0 || enemy1.y > 62){
			enemy1.life = dead;
			enemy1.x = 0;
			enemy1.y = -12;
			count1 = rand() % 800000;
			return;
		} else {
			enemy1.x += enemy1.vx;
			enemy1.y += enemy1.vy;
		}
	}
}

void collisions(void){
	int i;
	uint32_t x1, y1, x2, y2, x3, y3;
	x2 = enemy.x +8;
	y2 = enemy.y +4;
	x3 = enemy1.x +8;
	y3 = enemy1.y +4;
	for(i=0; i<18; i++){
		if(missile[i].life == alive){
			x1 = missile[i].x;
			y1 = missile[i].y;
			if(((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)) < 49){
				enemy.life = dead;
				Sound_Bomb();
				enemy.x = 126;
				enemy.y = -12;
				count = rand() % 800000;
				scoreCount += 5;
				return;
			}
			if(((x1-x3)*(x1-x3)+(y1-y3)*(y1-y3)) < 49){
				enemy1.life = dead;
				Sound_Bomb();
				enemy1.x = 0;
				enemy1.y = -12;
				count1 = rand() % 800000;
				scoreCount += 5;
				return;
			}
		}
	}
}

void playercollision(void){		
	if(	
		(SSD1306_GetPixel(player.x+3, player.y-2) == true) ||
		(SSD1306_GetPixel(player.x+3, player.y-1) == true) ||
		(SSD1306_GetPixel(player.x+3, player.y) == true) ||
		(SSD1306_GetPixel(player.x+3, player.y+1) == true) ||
		(SSD1306_GetPixel(player.x+3, player.y+2) == true))
		{
			player.life = dead;
			Sound_Bomb();
			playerexplode();
		}
		
}

//bool playPressed(){
//	if((GPIO_PORTE_DATA_R & 0x01) == 1){
//		while((GPIO_PORTE_DATA_R & 0x01) == 1){}
//		return true;
//	}
//	return false;
//}

//int langSelect(){
//	if((GPIO_PORTE_DATA_R & 0x01) == 1){
//		while((GPIO_PORTE_DATA_R & 0x01) == 1){}
//		return 1; // for Spanish
//	} else if ((GPIO_PORTE_DATA_R & 0x04) > 1) {
//		while((GPIO_PORTE_DATA_R & 0x04) > 1){}
//		return 0; // for English
//	} else {
//		return 2; // No button pressed
//	}
//	
//}

//void SSD1306_DrawUDec2(int16_t x, int16_t y, uint32_t n, uint16_t color){
//	char buf[4];
//	if(n >= 100){
//		buf[0] = '*'; /* illegal */
//		buf[1] = '*'; /* illegal */
//	}else {
//		if(n>=10){
//			buf[0]= n/10+'0'; /* 10 digit */
//			buf[1] = n%10+'0'; /* ones digit */
//		}	else{
//			buf[0]=' ';
//			buf[1] = n+'0'; /* ones digit */
//		}
//	}
//	buf[2] = 0; // null
//	SSD1306_DrawString(x,y,buf,color);
//}

// Intro screen and game play
int glob = 800000;
int lang = 2;
bool pause;
int main(){
	DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
  Random_Init(1);
	PortE_init();
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
	init();
	ADC_Init();  
	Sound_Init();
	SysTick_Init(8000000);
  SSD1306_ClearBuffer();
	SSD1306_DrawBMP(2, 62, Maze_Escape_Main, 0, SSD1306_WHITE);
	SSD1306_OutBuffer();
  Delay100ms(30);
  SSD1306_ClearBuffer(); // clears buffer
	SSD1306_OutClear();
	Delay100ms(5);
	// EnableInterrupts();
	// NVIC_ST_CTRL_R = 5; // will stop systick
	// SSD1306_ClearBuffer();
	SSD1306_SetCursor(3, 2);
	SSD1306_OutString((char *)"Press Shoot for:");
	SSD1306_SetCursor(5, 3);
	SSD1306_OutString((char *)"English");
	SSD1306_SetCursor(3, 5);
	SSD1306_OutString((char *)"Oprime Start para:");
	SSD1306_SetCursor(5, 6);
	SSD1306_OutString((char *)"Espa\xA4ol");
	while(1){
		lang = langSelect();
		if(lang != 2){
			break;
		}
	}
	if(lang == 0){
		SSD1306_ClearBuffer();
		SSD1306_DrawBMP(2, 20, WelcomeScreen, 0, SSD1306_WHITE);
		SSD1306_OutBuffer();
		SSD1306_SetCursor(0, 4);
		SSD1306_OutString((char *)"Press Button to Start");
	}
	if(lang == 1){
		SSD1306_ClearBuffer();
		SSD1306_DrawBMP(2, 20, WelcomeScreen_Span, 0, SSD1306_WHITE);
		SSD1306_OutBuffer();
		SSD1306_SetCursor(3, 4);
		SSD1306_OutString((char *)"Oprime Start para");
		SSD1306_SetCursor(7, 5);
		SSD1306_OutString((char *)"Iniciar");
	}
	while(1){
		if(playPressed()){ // if play is pressed move screens 
			break;
		}
		PF3 ^= 0x08;
	}
	EnableInterrupts();
	// SSD1306_SetCursor(1, 1);
  // SSD1306_OutString((char *)"GAME OVER");
  // SSD1306_SetCursor(1, 2);
  // SSD1306_OutString((char *)"Nice try,");
  // SSD1306_SetCursor(1, 3);
  // SSD1306_OutString((char *)"Earthling!");
  // SSD1306_SetCursor(2, 4);
	// NVIC_ST_CTRL_R = 7; // will start systick
	while(1){
		if(playPressed()){ // pause has been pressed 
			enemy.vx = 0;
			enemy.vy = 0;
			enemy1.vx = 0;
			enemy1.vy = 0;
			MazeSegments[index].vy = 0;
			while(!playPressed()){}
				enemy.vx = -1;
				enemy.vy = 1;
				enemy1.vx = 1;
				enemy1.vy = 1;
				MazeSegments[index].vy = VY;
		}
		if(update){
		Draw1();
		}
		if(player.life == false){
			SSD1306_OutClear();
			break;
		}
		if(count <= 0){
			enemy.life = alive;
		}
		if(count1 <= 0){
			enemy1.life = alive;
		}
		count--;
		count1--;
		if(glob == 0){
			scoreCount++;
			glob = 800000;
		}
		glob--;
		PF2 ^= 0x04;
	}
	if(lang == 1){
		SSD1306_ClearBuffer();
		SSD1306_SetCursor(8, 2);
		SSD1306_OutString((char *)"FIN!");
		SSD1306_SetCursor(5, 3);
		SSD1306_OutString((char *)"\n");
		SSD1306_SetCursor(5, 4);
		SSD1306_OutString((char *)"Puntuaci\xA2n:");
		SSD1306_SetCursor(5, 5);
		SSD1306_OutString((char *)"\n");
		SSD1306_SetCursor(5, 6);
		SSD1306_OutUDec(scoreCount);
	} else {
		SSD1306_ClearBuffer();
		SSD1306_SetCursor(5, 2);
		SSD1306_OutString((char *)"GAME OVER!");
		SSD1306_SetCursor(5, 3);
		SSD1306_OutString((char *)"\n");
		SSD1306_SetCursor(7, 4);
		SSD1306_OutString((char *)"Score:");
		SSD1306_SetCursor(5, 5);
		SSD1306_OutString((char *)"\n");
		SSD1306_SetCursor(7, 6);
		SSD1306_OutUDec(scoreCount);
	}
//	std::stringstream score;
//	score<<scoreCount;
//	std::string Score;
//	score>>Score;
//	char buff[5];
//	for(int i = 0; i < 4; i++){
//		buff[i] = Score[i]; 
//	}
//	buff[4] = 0;
//	SSD1306_DrawString(5, 10, buff, SSD1306_WHITE);
}


// Has gameplay
int main2(){
	DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
  Random_Init(1);
	PortE_init();
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
	init();
	ADC_Init();  
	SysTick_Init(8000000);
  SSD1306_ClearBuffer();
	SSD1306_DrawBMP(2, 62, Maze_Escape_Main, 0, SSD1306_WHITE);
	SSD1306_OutBuffer();
  Delay100ms(30);
  SSD1306_ClearBuffer(); // clears buffer
	SSD1306_OutClear();
	EnableInterrupts();
	
	while(1){
		Draw1();
		PF2 ^= 0x04;
	}
}

void fire(int32_t vx1, int32_t vy1){int i;
	i = 0;
	while(missile[i].life == alive){
		i++;
		if(i==18){return;}
	}
	missile[i].x = player.x; //+7;
	missile[i].y = player.y-4;
	missile[i].vx = vx1;
	missile[i].vy = vy1;
	missile[i].image = Missile0;
	missile[i].life = alive;
	Sound_Shoot();
}

void playerexplode(){
	explosion.image = BigExplosion0;
	SSD1306_DrawBMP(player.x, player.y, explosion.image, 0, SSD1306_INVERSE);
	SSD1306_OutBuffer();
	for(int i = 0; i<10000000;i++){}
}

//bool checkFire(){
//	if((GPIO_PORTE_DATA_R & 0x04) > 0){
//		return true;
//	}
//	return false;
//}

void SysTick_Handler(void){ // every 100 ms
  PF1 ^= 0x02;     // Heartbeat
	if(checkFire()){
		fire(0,-1);
	}
//	if((GPIO_PORTE_DATA_R & 0x04) > 0){
//		fire(0,-1);
//	}
	Move1();
	move();
	collisions();
	// playercollision();
}

// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
      time--;
    }
    count--;
  }
}


