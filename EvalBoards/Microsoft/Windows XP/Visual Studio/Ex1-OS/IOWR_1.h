#pragma once
#include <stdio.h>
#include "includes.h"

int LEDR_BASE_ADDR; // Base addresses
int LEDG_BASE_ADDR;
int HEX3_0_BASE_ADDR;
int HEX7_4_BASE_ADDR;

//void SetColor(int ForgC); // Sets colour for the text in terminal
//void update_led_display(int ledr_flag); // Updates the terminal output for the LEDs
//void update_hex_display();
//void allocate_mem(); // Allocates some memory for the I/Os
//void IOWR_em(int base_address, int offset, int value); // For writing to IO
//int IORD_em(int base_address, int offset); // For reading from IO