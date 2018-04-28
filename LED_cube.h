/* 
LED_cube.h

Written by Owen Lyke April 2018
Updated: April 2018


*/




#ifndef LED_CUBE_h
#define LED_CUBE_h


// Includes
// --------
#include "Arduino.h"
#include <SPI.h>












#define SN74HC_delay_micros 500

typedef struct{
	uint8_t 	use_spi;		// 0: don't use spi, use bit-banging instead. 1: use spi, choose which with "which"
	uint8_t		which;			// 0: SPI, 1: SPI1

	uint32_t	freq;			// Frequency of the SPI

	SPIClass 	*hSPI = &SPI;	// 
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



void SN74HC_init(SN74HC_HandleTypeDef *hSN74HC);
void SN74HC_output_bytes(SN74HC_HandleTypeDef *hSN74HC, uint8_t *pdata, uint16_t num_data_bytes);
void SN74HC_clear(SN74HC_HandleTypeDef *hSN74HC);
void SN74HC_output_clear(SN74HC_HandleTypeDef *hSN74HC);






typedef struct{
	uint8_t 	X;
	uint8_t		Y;
	uint8_t		Z;
}Coordinate_TypeDef;

typedef struct{
	uint8_t 	X;
	uint8_t		Y;
	uint8_t		Z;
}Center_TypeDef;

typedef struct{
	uint8_t 	X;
	uint8_t		Y;
	uint8_t		Z;
	uint8_t 	Xbias;
	uint8_t 	Ybias;
	uint8_t 	Zbias;
}Extent_TypeDef;













typedef struct{
	SN74HC_HandleTypeDef 	hanode_sr;
	SN74HC_HandleTypeDef 	hcathode_sr;

	uint8_t					bits_width;
	uint8_t					bits_depth;
	uint8_t 				bits_height;

	volatile uint16_t		GPCounter_bits;
	volatile uint16_t		GPCounter_bytes;

	uint8_t 				framerate;

}LEDcube_HandleTypeDef;


void LEDcube_begin(LEDcube_HandleTypeDef *hLEDcube);
void LEDcube_update_frame(LEDcube_HandleTypeDef *hLEDcube);




uint32_t LEDcube_get_index_offset(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z );
uint8_t LEDcube_get_bit_offset(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z );

void LEDcube_set_voxel(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z);
void LEDcube_clear_voxel(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z);

void LEDcube_clear_all(LEDcube_HandleTypeDef *hLEDcube);
uint32_t LEDcube_sum_all_bits(LEDcube_HandleTypeDef *hLEDcube);

void LEDcube_set_line_X(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);
void LEDcube_set_line_Y(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);
void LEDcube_set_line_Z(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);
void LEDcube_clear_line_X(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);
void LEDcube_clear_line_Y(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);
void LEDcube_clear_line_Z(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end);

void LEDcube_set_rectangle_XY(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t ystart, uint8_t xend, uint8_t yend, uint8_t Z );
void LEDcube_set_rectangle_XZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t zstart, uint8_t xend, uint8_t zend, uint8_t Y );
void LEDcube_set_rectangle_YZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t ystart, uint8_t zstart, uint8_t yend, uint8_t zend, uint8_t X );
void LEDcube_clear_rectangle_XY(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t ystart, uint8_t xend, uint8_t yend, uint8_t Z );
void LEDcube_clear_rectangle_XZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t zstart, uint8_t xend, uint8_t zend, uint8_t y );
void LEDcube_clear_rectangle_YZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t ystart, uint8_t zstart, uint8_t yend, uint8_t zend, uint8_t X );

void LEDcube_set_rect_prism_CO(LEDcube_HandleTypeDef *hLEDcube, Coordinate_TypeDef corner1, Coordinate_TypeDef corner2 );
void LEDcube_set_rect_prism_CE(LEDcube_HandleTypeDef *hLEDcube, Center_TypeDef center, Extent_TypeDef extent );

void LEDcube_set_sphere_CE(LEDcube_HandleTypeDef *hLEDcube, Center_TypeDef center, Extent_TypeDef extent );



#endif