#include <stdio.h>
#include "includes.h"


/* Definition of IO variables */

#define NUM_HEX 8 // Number of seven segments
#define NUM_HEX_BITS 8 // Number of bits for each seven segment display, last bit is for "." which is not implemented here
#define GREEN 2
#define RED 4
#define WHITE 15

int * ledr = 0; // 18 bits
int * ledg = 0; // 9 bits
int * hex3_0 = 0;
int * hex7_4 = 0;

int LEDR_BASE_ADDR; // Base addresses
int LEDG_BASE_ADDR;
int HEX3_0_BASE_ADDR;
int HEX7_4_BASE_ADDR;

int numbits[2] = { 9, 18 }; // for the LEDG and LEDR number of bits respectively



/* Prototype declarations */
void SetColor(int ForgC); // Sets colour for the text in terminal
void update_led_display(int ledr_flag); // Updates the terminal output for the LEDs
void update_hex_display();
void allocate_mem(); // Allocates some memory for the I/Os
void IOWR_em(int base_address, int offset, int value); // For writing to IO

int FPGA_IO(int base_address, int offset, int value, BOOL read_flag); // Corresponds to LEDR and LEDG I/Os on the FPGA
int IORD_em(int base_address, int offset); // For reading from IO


void SetColor(int ForgC)
{
	// This function will set the colour of the text in terminal

	// Colour scheme:
	//	Name | Value
	//	Black | 0
	//	Blue | 1
	//	Green | 2
	//	Cyan | 3
	//	Red | 4
	//	Magenta | 5
	//	Brown | 6
	//	Light Gray | 7
	//	Dark Gray | 8
	//	Light Blue | 9
	//	Light Green | 10
	//	Light Cyan | 11
	//	Light Red | 12
	//	Light Magenta | 13
	//	Yellow | 14
	//	White | 15

	WORD wColor;

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	//We use csbi for the wAttributes word.
	if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
	{
		//Mask out all but the background attribute, and add in the forgournd color
		wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
		SetConsoleTextAttribute(hStdOut, wColor);
	}
	return;
}

void update_led_display(int ledr_flag, int beep, int col)
{
	// This function will update the I/O for LEDR and LEDG
	// The ledr_flag is a either a 1 or 0 to determine if LEDR is chosen (1) or LEDG is chosen (0)
	// This function can be called from anywhere as long as you specify the ledr_flag

	// PLEASE READ:
	// This function is available to be edited for your LEDR display
	// To show that the alarm has been triggered

	int tmp = 0;

	char temp_string[5] = "";

	if (ledr_flag) sprintf(temp_string, "LEDR"); // Which string to print on terminal
	else  sprintf(temp_string, "LEDG");

	int base_address = ledr_flag ? LEDR_BASE_ADDR : LEDG_BASE_ADDR; // Which base address to use

	int value = IORD_em(base_address, 0); // Read from the base address

	int colour = ledr_flag ? RED : GREEN; // Colour of terminal text
	SetColor(colour);

	if (beep && ledr_flag)
	{
		SetColor(col+1);
	}

	// Pretty printing

	OS_Printf("%s:\t", temp_string, value);

	for (int i = numbits[ledr_flag] - 1; i >= 0; i--) OS_Printf("%5d", i); // I/O Numbers

	OS_Printf("\n\t");

	if (ledr_flag)
	{
		// LEDR
		for (int i = numbits[ledr_flag] - 1; i >= 0; i--) // I/O Values
			if (value & (1 << i)) OS_Printf("  [X]");
			else OS_Printf("  [ ]");
	}
	else
	{
		// LEDG
		for (int i = numbits[ledr_flag] - 1; i >= 0; i--) // I/O Values
			if (value & (1 << i)) OS_Printf("  [X]");
			else OS_Printf("  [ ]");
	}

	OS_Printf("\n");

	SetColor(WHITE); // Setting back to white text
}


void allocate_mem() // simulates the mapped register allocation
{

	ledr = (int *)malloc(8);	// Allocate memory
	ledg = (int *)malloc(8);
	hex3_0 = (int *)malloc(8);
	hex7_4 = (int *)malloc(8);

	*(&ledr) = 0;	// Clear memory value
	*(&ledg) = 0;
	*(&hex3_0) = 0;
	*(&hex7_4) = 0;

	(int *)LEDR_BASE_ADDR = (int *)&ledr; // Assign base addresses to variables
	(int *)LEDG_BASE_ADDR = (int *)&ledg;
	(int *)HEX3_0_BASE_ADDR = (int *)&hex3_0;
	(int *)HEX7_4_BASE_ADDR = (int *)&hex7_4;

}

int FPGA_IO(int base_address, int offset, int value, BOOL read_flag) // simulates FPGA I/O
{
	int *ptr = (int *)base_address;

	if (read_flag)
		return *(ptr + offset);
	else
		*(ptr + offset) = value;

	return 0;

}

void update_hex_display(int mode)
{
	// This prototype will update all the HEX displays on the terminal
	// Seven segment display looks like:
	//	 _
	//	|_|
	//	|_|
	//
	// Follows the convention of a,b,c,d,e,f,g (from top, to clockwise, to middle)
	// https://en.wikipedia.org/wiki/Seven-segment_display#/media/File:7_Segment_Display_with_Labeled_Segments.svg
	// Each HEX has 8 bits, the last bit is for the "." which will not be shown in this terminal output
	// HEX3 to HEX0 are stored in one address 
	// HEX7 to HEX4 are stored in another address

	// Printing corresponding HEX labels
	OS_Printf("\n");
	for (int i = NUM_HEX - 1; i >= 0; i--)
	{
		OS_Printf("\tHEX%d", i);
	}

	int hex3_0_val = IORD_em(HEX3_0_BASE_ADDR, 0);
	int hex7_4_val = IORD_em(HEX7_4_BASE_ADDR, 0);
	int temp_val = 0;
	int num_bits_to_shift = 0;

	OS_Printf("\n");

	if (mode == 0) SetColor(5);
	else SetColor(14);
	
	for (int i = 0; i < 3; i++) // For the 3 lines, this will for loop each of the HEX
	{
		for (int j = NUM_HEX - 1; j >= 0; j--)
		{
			char temp_string[4] = "   "; // Empty spaces
			temp_val = (j <= 3) ? hex3_0_val : hex7_4_val;
			num_bits_to_shift = (j % (NUM_HEX / 2)) *NUM_HEX_BITS;
			OS_Printf("\t");
			switch (i) {
			case 0:
				if ((temp_val >> num_bits_to_shift) & 0x01) OS_Printf(" _"); // look for the 0th bit
				break;
			case 1:
				if ((temp_val >> num_bits_to_shift) & 0x02) temp_string[2] = '|'; // look for the 1st bit
				if ((temp_val >> num_bits_to_shift) & 0x20) temp_string[0] = '|'; // look for the 5th bit
				if ((temp_val >> num_bits_to_shift) & 0x40) temp_string[1] = '_'; // look for the 6th bit
				OS_Printf("%s", temp_string);
				break;

			case 2:

				if ((temp_val >> num_bits_to_shift) & 0x04) temp_string[2] = '|'; // look for the 2nd bit
				if ((temp_val >> num_bits_to_shift) & 0x10) temp_string[0] = '|'; // look for the 4th bit
				if ((temp_val >> num_bits_to_shift) & 0x08) temp_string[1] = '_'; // look for the 3rd bit
				OS_Printf("%s", temp_string);
				break;
			}

		}

		OS_Printf("\n"); // Next line of seven seg
	}
	SetColor(WHITE);
	//OS_Printf(" _\n");
	//OS_Printf("|_|\n");
	//OS_Printf("|_|\n");
	OS_Printf("\n");


}

void IOWR_em(int base_address, int offset, int value) // Simulates IOWR
{
	// Base_address corresponds to the I/O address which you can use directly after calling allocate_mem()
	// Base address variables are: LEDR_BASE_ADDR and LEDG_BASE_ADDR

	BOOL read_flag = FALSE;

	FPGA_IO(base_address, offset, value, read_flag); // Talks to the I/O

	//if (base_address == LEDR_BASE_ADDR) update_led_display(1); // Chooses LEDR or LEDG or HEX
	//else if (base_address == LEDG_BASE_ADDR) update_led_display(0);
	//else update_hex_display(); 
}

int IORD_em(int base_address, int offset) // Simulates IORD
{
	BOOL read_flag = TRUE;

	return FPGA_IO(base_address, offset, 0, read_flag); // Reads from the I/O and returns an INT
}

