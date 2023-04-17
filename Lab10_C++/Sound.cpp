// Sound.cpp
// Runs on any computer
// Sound assets based off the original Space Invaders 

// Jonathan Valvano
// 1/2/2023
#include <stdint.h>
#include "Sound.h"
#include "../inc/tm4c123gh6pm.h"
#include "../inc/DAC.h"
#include "Timer1.h"

uint32_t Length;
const uint8_t *sfxPt;

// 6-bit sounds, eampled at 11,025 kHz

