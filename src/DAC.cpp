// put implementations for functions, explain how it works
// put your names here, date
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#include "DAC.h"

// DAC.c
// This software configures DAC output
// Labs 6 and 10 requires 6 bits for the DAC
// Runs on LM4F120 or TM4C123
// Program written by: Cindey and Jiwoo
// Date Created: 3/6/17 
// Last Modified: 4/15/23 

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// **DAC_Init*
// Initialize 6-bit DAC, called once 
// Input: none
void DAC_Init(void){

    unsigned long volatile delay;
    SYSCTL_RCGCGPIO_R |= 0x02; //Port B
    while((SYSCTL_RCGCGPIO_R & 0x02) == 0){}
    //6 Bit DAC
    GPIO_PORTB_DIR_R |= 0x3F; //Ports PB0 - PB5 are output
    GPIO_PORTB_DEN_R |= 0x3F;
}

// DAC_Out
// output to DAC
// Input: 6-bit data, 0 to 63 
// Input=n is converted to n3.3V/63
// Output: none
void DAC_Out(uint8_t data){
    GPIO_PORTB_DATA_R = (GPIO_PORTB_DATA_R & 0xFFFFFFC0)|(data & 0x3F); //6-Bit DAC
}
