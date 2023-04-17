// main.cpp
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the ECE319K Lab 10 in C++

// Last Modified: 1/2/2023 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1 
// 8*R resistor DAC bit 2 on PB2
// 4*R resistor DAC bit 1 on PB3
// 2*R resistor DAC bit 2 on PB4
// 1*R resistor DAC bit 3 on PB5 (most significant bit)
// LED on PB6
// LED on PB7

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "random.h"
#include "PLL.h"
#include "Sound.h"
#include "SlidePot.h"
#include "Images.h"
#include "UART.h"
#include "Timer0.h"
#include "Timer1.h"

SlidePot my(2000,0);

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);


// Creating a struct for the Sprite.
typedef enum {dead, dying, alive} status_t;
struct sprite{
  int32_t x;      // x coordinate
  int32_t y;      // y coordinate
	int32_t vx, vy;	 // pixels/frame -2, -1, 0, 1, 2 (for non-bullets)
  const unsigned short *image; // ptr->image
  status_t life;            // dead/alive
};          
typedef struct sprite sprite_t;
#define MAX 3

sprite_t bill = {60, 25, 0, 0, SmallEnemy20pointB, alive};
sprite_t player[MAX] =
{{52, 159, 0, 0, PlayerShip0, alive},
{52, 159, 0, 0, PlayerShip0, dying},
{52, 159, 0, 0, PlayerShip0, dead}};
sprite_t bullet = {0, 0, 0, -4, shoot3, dead};	// bullet 1 is also a sprite

uint32_t time = 0;
uint32_t score = 0;
uint32_t position;
int32_t direction = 1;
// global flags
// Switch flags used (extern keyword) in Timer0.cpp
volatile uint32_t needToRedraw;
volatile uint32_t lastSwitch = 0;
volatile uint32_t nowSwitch = 0;
volatile uint32_t startFire = 0;
volatile uint32_t pause;

void Shoot(void) {
	// only y value changes.
	if(bullet.life == alive && startFire == 1)
		bullet.y += bullet.vy;
}

void Move(void) {
	for(int i = 0; i < MAX; i++) {
		if(player[i].life == alive) {
			player[i].x += player[i].vx;
			
			// threshold control
			if(player[0].x < 0)
				player[0].x = 0;
			if(player[0].x > 109)
				player[0].x = 109;
		}
	}
}

// 30Hz Game Engine
// Data: player[MAX]
// Status: needToRedraw --> when do I check it? main thread!
// Draw() is called in main thread.
int debug = 0;
void background(void){
	// player kill status
	if(bill.y == 159){
			player[0].life = dead;
		}
	// enemy kill status
	// overlap with bullet
	if(bill.y >= bullet.y-10 && bill.y <= bullet.y && bullet.x >= bill.x-9 && bullet.x <= bill.x+15) {
		bill.life = dying;
		bullet.life = dying;
	}
	// missile
	if(bullet.y < 40) {
		bullet.life = dying;
		startFire = 0;
	}
	Shoot();
	lastSwitch = nowSwitch;
	
	// player + slidepot
	position = my.Convert(ADC_In());	// 0 to 4095
	if(player[0].life == alive) {
		player[0].vx = position/450 - 2;
	}
	Move();
	needToRedraw = 1; 	// semaphore
	
	// enemy 1 (bill) coming down
  if(bill.life == alive){
    bill.y++;
		bill.x += 1 * direction;
		if(bill.x == 0 || bill.x == 111)
			direction *= -1;
  }
	
	if (bill.life == dead)
			debug++;
}

void Draw(void) {
	for(int i = 0; i < MAX; i++) {
		// player
		if(player[i].life == alive)
			ST7735_DrawBitmap(player[i].x, player[i].y, player[i].image, 18, 8);
		
		// bullet
		if(bullet.life == alive)
			ST7735_DrawBitmap(bullet.x, bullet.y, shoot3, 12, 12);
		else if(bullet.life == dying) {	// dead
			ST7735_DrawBitmap(bullet.x, bullet.y, blackShoot3, 12, 12);
			bullet.life = dead;
			bullet.x = 0;
			bullet.y = 0;
		}
		
		// enemy 1- bill
		if(bill.life == alive) {
			ST7735_DrawBitmap(bill.x, bill.y, bill.image, 16, 10);
		}
		else if(bill.life == dying){	// when dead, draw once
			ST7735_DrawBitmap(bill.x, bill.y, blackBill, 16, 10);
			bill.life = dead;
			bill.x = 0;
			bill.y = 0;
		}
	}
}

// apparnetly every second
void clock(void){
  time++;
}

void switchInit(void) {
	volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x10;	// Activate clock for Port E
	delay = SYSCTL_RCGCGPIO_R;	// NOPs
	GPIO_PORTE_DIR_R &= ~0x06;	// Direction: Input (0)
	GPIO_PORTE_DEN_R |= 0x06;		// enable digital port
}

int main(void){
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  // TExaS_Init();
  Random_Init(1);
  Output_Init();
  //Timer0_Init(&background,1600000); // 50 Hz
	ADC_Init();
	switchInit();
	Timer0_Init(&background, 2666667);	// 30Hz
  Timer1_Init(&clock,80000000); // 1 Hz
	ST7735_InitR(INITR_REDTAB);
  EnableInterrupts();
	
	// Initial positions of everything
  //ST7735_DrawBitmap(52, 159, PlayerShip0, 18,8); // player ship middle bottom
  //ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(0, 25, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,25, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 25, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(80, 25, SmallEnemy30pointA, 16,10);
  ST7735_DrawBitmap(100, 25, SmallEnemy30pointB, 16,10);
  while(player[0].life == alive){
		ST7735_SetCursor(0, 0);
		ST7735_OutString((char*)"Task: Shoot them!!");

    while(needToRedraw==0){};
    needToRedraw = 0;
		Draw();
		
		// bullet control & regeneration
		if(nowSwitch == 1) {
			startFire = 1;
			
			if(bullet.life == dead) {
				bullet.life = alive;
				bullet.x = player[0].x + 4;
				bullet.y = player[0].y - 8;
			}
		}
	}

	if(bill.life == dead)
		score += 20;
	
  ST7735_FillScreen(0x0000);            // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString((char*)"GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_SetTextColor(ST7735_WHITE);
  ST7735_OutString((char*)"Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString((char*)"Score:");
  ST7735_SetCursor(2, 4);
	ST7735_OutUDec(score);
  ST7735_SetTextColor(ST7735_WHITE);
	
  //while(1){
   // while(needToRedraw==0){};
   // needToRedraw = 0;
   // ST7735_SetCursor(2, 4);
   // ST7735_OutUDec(time);
  //}

}




