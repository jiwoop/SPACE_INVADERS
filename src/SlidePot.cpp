// SlidePot.cpp
// Runs on TM4C123
// Provide functions that initialize ADC0 and use a slide pot to measure distance
// Modified: 1/11/2022 
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "SlidePot.h"
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
//*** students write this ******
	SYSCTL_RCGCADC_R |= 0x0001;		// 1) Activate ADC0
	SYSCTL_RCGCGPIO_R |= 0x08;		// 2) Activate clock for Port D (We are using PD2 as an input)
	while((SYSCTL_PRGPIO_R&0x08) != 0x08) {}	// 3) Wait for stabilization
	GPIO_PORTD_DIR_R &= ~0x04;		// 4) Make PD2 input
	GPIO_PORTD_AFSEL_R |= 0x04;		// 5) Enable alternate function
	GPIO_PORTD_DEN_R &= ~0x04;		// 6) Disable digital I/O
	GPIO_PORTD_AMSEL_R |= 0x04;		// 7) Enable analog functionality
	
	ADC0_PC_R &= ~0xF;
	ADC0_PC_R |= 0x1;					// 8) Configure for 125K samples/sec
	ADC0_SSPRI_R = 0x0123;		// 9) Sequencer 3 is highest priority
	ADC0_ACTSS_R &= ~0x0008;	// 10) Disable sample sequencer 3
	ADC0_EMUX_R &= ~0xF000;		// 11) seq3 is software trigger
	ADC0_SSMUX3_R &= ~0x000F;
	ADC0_SSMUX3_R += 5;				// 12) Set channel (Channel 5 in our case)
	ADC0_SSCTL3_R = 0x0006;		// 13) One and only seq. no TS0 D0, yes IE0 END0
	ADC0_IM_R &= ~0x0008;			// 14) Disable SS3 interrupts
	ADC0_ACTSS_R |= 0x0008;		// 15) Enable sample sequencer 3
	ADC0_SAC_R = 5;						// 16) 32-point hardware averaging
}

//------------ADCIn------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
//*** students write this ******
		uint32_t result;
		ADC0_PSSI_R = 0x0008;							// 1) Initiate SS3
		while((ADC0_RIS_R&0x08) == 0) {}; // 2) Wait for conversion done
		result = ADC0_SSFIFO3_R&0xFFF;		// 3) read result
		ADC0_ISC_R = 0x0008;							// 4) acknowledge completion
		return result;
}

// constructor, invoked on creation of class
// m and b are linear calibration coeffients 
SlidePot::SlidePot(uint32_t m, uint32_t b){
//*** students write this ******
// initialize all private variables
// make slope equal to m and offset equal to b
	this->slope = m;
	this->offset = b;
}

void SlidePot::Save(uint32_t n){
//*** students write this ******
// 1) save ADC sample into private variable
	this->data = n;
// 2) calculate distance from ADC, save into private variable
	this->distance = (this->slope * this->data) / 4096  + this->offset;
// 3) set semaphore flag = 1
	this->flag = 1;
}
uint32_t SlidePot::Convert(uint32_t x){
  //*** students write this ******
  // use calibration data to convert ADC sample to distance
	return (2007*x) / 4096;
}

void SlidePot::Sync(void){
// 1) wait for semaphore flag to be nonzero
	while(this->flag == 0) {};
// 2) set semaphore flag to 0
	this->flag = 0;
}

uint32_t SlidePot::ADCsample(void){ // return ADC sample value (0 to 4095)
  //*** students write this ******
  // return last calculated ADC sample
  return this->data; // replace this with solution
}

uint32_t SlidePot::Distance(void){  // return distance value (0 to 2000), 0.001cm
  //*** students write this ******
  // return last calculated distance in 0.001cm
  return this->distance; // replace this with solution
}


