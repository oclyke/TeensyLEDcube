/* 
LED_cube.h

Written by Owen Lyke April 2018
Updated: April 2018

Description:
This library is intended to be used on the Teensy3.6 development board with the Arduino evironment, in conjunction with SN74HC595N shift registers, to control an LED cube. 
It includes two main parts: shift register control and led cube operations.

Shift Register Control:
Create an instance of the SN74HC_HandleTypeDef type. Then set the appropriate fields:
- num_bytes : how many bytes long the shift register is. For example 8 SN74HC595N shift registers daisy-chained together is 8 bytes long
- *pbuffer : a pointer to the uint8_t data buffer that is the output. This buffer must be greater than or equal to num_bytes in length, but it can be longer of course. 
- X_pin : the pin number of the Teensy3.6 that you will connect to the corresponding pin on the first SR.
- SPI_preferences : this is another structure used to compactify setup of SPI or bit-banging methods
	- use_spi : set to '0' to use bit-banging, or any other value to use SPI ports. If you choose to use SPI then make sure to specify which of the two SPI ports to use
	- which : '0' for SPI, '1' for SPI1
	- freq : frequency of the SPI clock in Hz. I have used up to 10,000,000 and still gotten great response from the shift registers. This is good news for future (larger... 16^3) LED cube projects

*/

#ifndef LED_CUBE_h
#define LED_CUBE_h


// Includes
// --------
#include "Arduino.h"
#include <SPI.h>

//#define SN74HC_delay_micros 500 // Unused

typedef struct{
	uint8_t 	use_spi;		// 0: don't use spi, use bit-banging instead. 1: use spi, choose which with "which"
	uint8_t		which;			// 0: SPI, 1: SPI1

	uint32_t	freq;			// Frequency of the SPI

	SPIClass 	*hSPI = &SPI;	// You don't need to change these... They are part of Teensy3.6 code by Paul S.
	SPI1Class 	*hSPI1 = &SPI1;
}SN74HC_SPIPreferencesTypeDef;

typedef struct{
	uint8_t					num_bytes;			// Stores the number of bytes in the shift register. Treat daisy-chained SRs as one
	uint8_t					*pbuffer;			// Can associate a buffer with the shift register, if you so please

	uint8_t 				latch_pin;
	uint8_t 				data_pin; 			// This can be taken care of with SPI
	uint8_t 				clock_pin;			// This can be taken care of with SPI
	uint8_t 				clear_pin;
	uint8_t 				OE_pin;

	SN74HC_SPIPreferencesTypeDef 		SPI_preferences;	// Used to specify which, if any, SPI port to use to transmit the data
}SN74HC_HandleTypeDef;



void SN74HC_init(SN74HC_HandleTypeDef *hSN74HC);														// Initializes the pins and SPI ports (if applicable) that are specified in the handle
void SN74HC_output_bytes(SN74HC_HandleTypeDef *hSN74HC, uint8_t *pdata, uint16_t num_data_bytes);		// Shifts in data and latches it to the output
void SN74HC_clear(SN74HC_HandleTypeDef *hSN74HC);														// Shifts out zeros for the legnth of the shift register
void SN74HC_output_clear(SN74HC_HandleTypeDef *hSN74HC);												// Latches all zeros to the output






// Small structures used to be clear about the intent of overloaded functions X_CO vs X_CE
typedef struct{
	uint8_t 	X;
	uint8_t		Y;
	uint8_t		Z;
}Coordinate_TypeDef;		// Coordinate typedef is used to specify particular coordinates

typedef struct{
	uint8_t 	X;
	uint8_t		Y;
	uint8_t		Z;
}Center_TypeDef;			// Center typedef is used to specify the desired center of objects, it is a different type than the coordinate type because it overloads some shape-drawing functions

typedef struct{
	uint8_t 	X;
	uint8_t		Y;
	uint8_t		Z;
	uint8_t 	Xbias;
	uint8_t 	Ybias;
	uint8_t 	Zbias;
}Extent_TypeDef;			// Extent typedef is used to specify how large to make objects. It also includes the Nbias information which is used to decide where to shift the center if the extent is even (no exact center led exists)



// This is the LEDcube typedef. It contains all the required variables to implement a cube, using one shift register to control the ground plane activation and another to control the anode patterns.
typedef struct{
	SN74HC_HandleTypeDef 	hanode_sr;					// An instance of a shift register handle, used for the anode control
	SN74HC_HandleTypeDef 	hcathode_sr;				// An instance of a shift register handle, used for the cathode control

	uint8_t					bits_width;					// How many LEDs wide is the cube (width is the X direction)
	uint8_t					bits_depth;					// How many LEDs deep is the cube (depth is the Y direction)
	uint8_t 				bits_height;				// How many LEDs high is the cube (height is the Z direction)

	volatile uint16_t		GPCounter_bits;				// Used when determining the bit offset at the specified index to modify individual leds
	volatile uint16_t		GPCounter_bytes;			// Used when determining the index of the data array at which to modify the LED state

	uint8_t 				framerate;
}LEDcube_HandleTypeDef;


void LEDcube_begin(LEDcube_HandleTypeDef *hLEDcube);					// Call the begin function to start the shift registers, clear the cube, and initialize the GPCounter variables
void LEDcube_update_frame(LEDcube_HandleTypeDef *hLEDcube);				// This function is called by the frame update ISR so as to get consistent timing

uint32_t LEDcube_get_index_offset(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z );	// This function takes a desired (X,Y,Z) coordinate and returns the proper index offset for the anode data buffer. Mostly inteded to be used by other functions, not the user directly
uint8_t LEDcube_get_bit_offset(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z );		// Same as directly above, except finds the bit offset in the byte specified by the index offset. Use both to set or clear individual LEDs

void LEDcube_set_voxel(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z);				// The most basic user function for modifying LEDs
void LEDcube_clear_voxel(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z);

void LEDcube_clear_all(LEDcube_HandleTypeDef *hLEDcube);
uint32_t LEDcube_sum_all_bits(LEDcube_HandleTypeDef *hLEDcube);

void LEDcube_set_line_X(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);			// Drawing or erasing a line of arbitrary length along a given axis. Future work might include passing in a variable to specify the axis instead of using a different function.
void LEDcube_set_line_Y(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);
void LEDcube_set_line_Z(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);
void LEDcube_clear_line_X(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);
void LEDcube_clear_line_Y(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);
void LEDcube_clear_line_Z(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);

void LEDcube_set_rectangle_XY(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t ystart, uint8_t xend, uint8_t yend, uint8_t Z );		// Uses a series of lines to draaw or erase a 2D rectangle in any given plane. Same future work as for the line functions
void LEDcube_set_rectangle_XZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t zstart, uint8_t xend, uint8_t zend, uint8_t Y );
void LEDcube_set_rectangle_YZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t ystart, uint8_t zstart, uint8_t yend, uint8_t zend, uint8_t X );
void LEDcube_clear_rectangle_XY(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t ystart, uint8_t xend, uint8_t yend, uint8_t Z );
void LEDcube_clear_rectangle_XZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t zstart, uint8_t xend, uint8_t zend, uint8_t y );
void LEDcube_clear_rectangle_YZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t ystart, uint8_t zstart, uint8_t yend, uint8_t zend, uint8_t X );

void LEDcube_set_rect_prism_CO(LEDcube_HandleTypeDef *hLEDcube, Coordinate_TypeDef corner1, Coordinate_TypeDef corner2 );					// Draws a 3D recangular prism using a series of rectangles. Specify opposing corners and everything in between (and including those corners) is modified
void LEDcube_set_rect_prism_CE(LEDcube_HandleTypeDef *hLEDcube, Center_TypeDef center, Extent_TypeDef extent );								// Same as above except *overloaded* to use the center and extent syntax. Makes it easier to animate motion of objects.  *(not actually a C feature, and I want to keep this C friendly)

void LEDcube_set_sphere_CE(LEDcube_HandleTypeDef *hLEDcube, Center_TypeDef center, Extent_TypeDef extent );									// Draws my rendition of a sphere at 1 of 6 possible sizes. Uses the center coordinates, X extent (to set size) and all biases (to shift preference at even extents...)



#endif