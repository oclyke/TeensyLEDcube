/* 
LED_cube.cpp

Header: LED_cube.h

Written by Owen Lyke

Updated: April 2018

Description:



*/


/*
-----------------------------------
Obligatory includes
-----------------------------------
*/
#include "LED_cube.h"	



void SN74HC_init(SN74HC_HandleTypeDef * hSN74HC)
{
	// Set OE and CLEAR pins to proper states	
	pinMode(hSN74HC->clear_pin, OUTPUT);	digitalWrite(hSN74HC->clear_pin, HIGH);		// Keeping CLEAR high ensures the data is not cleared	
	pinMode(hSN74HC->OE_pin, OUTPUT);		digitalWrite(hSN74HC->OE_pin, LOW);			// OE is a low-active pin. 
	pinMode(hSN74HC->latch_pin, OUTPUT);	digitalWrite(hSN74HC->latch_pin, LOW);		// Latch will take effect on rising edges, so get prepared

	if(hSN74HC->SPI_preferences.use_spi)
	{
		if(hSN74HC->SPI_preferences.which)
		{
			hSN74HC->SPI_preferences.hSPI1->begin();
		}
		else
		{
			hSN74HC->SPI_preferences.hSPI->begin();
		}
	}
	else
	{
		pinMode(hSN74HC->data_pin, OUTPUT);
		pinMode(hSN74HC->clock_pin, OUTPUT);
	}
}


void SN74HC_output_bytes(SN74HC_HandleTypeDef *hSN74HC, uint8_t *pdata, uint16_t num_data_bytes)
{
	digitalWrite(hSN74HC->latch_pin, LOW);				// Ensure that the latch pin is low in order to make a rising edge later

	if(hSN74HC->SPI_preferences.use_spi)
	{
		if(hSN74HC->SPI_preferences.which)
		{
			hSN74HC->SPI_preferences.hSPI1->beginTransaction(SPISettings(hSN74HC->SPI_preferences.freq, MSBFIRST, SPI_MODE3));
			//hSN74HC->SPI_preferences.hSPI1->transfer(pdata, num_data_bytes); // Apparently Arduino is not the samrtest... this function overwrites the data in this buffer with the received data, effectively forcing me to write my own loop.
			for(uint16_t indi = 0; indi < num_data_bytes; indi++)
			//for(uint16_t indi = (num_data_bytes-1); indi >= 0; indi--)
			{
				hSN74HC->SPI_preferences.hSPI1->transfer(*(pdata + indi));
			}
		}
		else
		{
			hSN74HC->SPI_preferences.hSPI->beginTransaction(SPISettings(hSN74HC->SPI_preferences.freq, MSBFIRST, SPI_MODE3));
			//hSN74HC->SPI_preferences.hSPI->transfer(pdata, num_data_bytes); // Apparently Arduino is not the samrtest... this function overwrites the data in this buffer with the received data, effectively forcing me to write my own loop.
			for(uint16_t indi = 0; indi < num_data_bytes; indi++)
			{
				hSN74HC->SPI_preferences.hSPI->transfer(*(pdata + indi));
			}
		}
	}
	else
	{
		for(uint16_t indi = 0; indi < num_data_bytes; indi++)
		{
			for(uint8_t indj = 0; indj < 8; indj++)
			{
				digitalWrite(hSN74HC->clock_pin, LOW);
				digitalWrite(hSN74HC->data_pin, ((*(pdata + indi) >> (7-indj)) & 0x01));
				// delayMicroseconds(SN74HC_delay_micros); // Need a delay?
				digitalWrite(hSN74HC->clock_pin, HIGH);				
			}
		}
	}
	digitalWrite(hSN74HC->latch_pin, HIGH);				// Latch the data to the output
}


void SN74HC_clear(SN74HC_HandleTypeDef *hSN74HC)
{
	digitalWrite(hSN74HC->clear_pin, LOW);
	// delayMicroseconds(SN74HC_delay_micros); // Need a delay?
	digitalWrite(hSN74HC->clear_pin, HIGH);
}
void SN74HC_output_clear(SN74HC_HandleTypeDef *hSN74HC)
{
	digitalWrite(hSN74HC->latch_pin, LOW);
	SN74HC_clear(hSN74HC);
	digitalWrite(hSN74HC->latch_pin, HIGH);
}





















void LEDcube_begin(LEDcube_HandleTypeDef *hLEDcube)
{
	// Initialize the shift registers
	SN74HC_init(&hLEDcube->hanode_sr);
  	SN74HC_init(&hLEDcube->hcathode_sr);

  	// Clear out the data
  	for(uint16_t indi = 0; indi < hLEDcube->hcathode_sr.num_bytes; indi++){ *(hLEDcube->hcathode_sr.pbuffer + indi) = 0x00; }
  	LEDcube_clear_all(hLEDcube);

  	hLEDcube->GPCounter_bits = 0;
  	hLEDcube->GPCounter_bytes = 0;
}


void LEDcube_update_frame(LEDcube_HandleTypeDef *hLEDcube)
{
  
  hLEDcube->GPCounter_bits++;
  if(hLEDcube->GPCounter_bits >= 8)
  {
    hLEDcube->GPCounter_bits = 0;
    hLEDcube->GPCounter_bytes++;
  }
  if((hLEDcube->GPCounter_bytes*8 + hLEDcube->GPCounter_bits) >= hLEDcube->bits_height)
  {
  	hLEDcube->GPCounter_bytes = 0;
  	hLEDcube->GPCounter_bits = 0;
  }

  *(hLEDcube->hcathode_sr.pbuffer + hLEDcube->GPCounter_bytes) = 1<<hLEDcube->GPCounter_bits;
  
  SN74HC_output_clear(&hLEDcube->hcathode_sr);
  SN74HC_output_bytes(&hLEDcube->hcathode_sr, hLEDcube->hcathode_sr.pbuffer, hLEDcube->hcathode_sr.num_bytes);

  SN74HC_output_clear(&hLEDcube->hanode_sr);
  SN74HC_output_bytes(&hLEDcube->hanode_sr, hLEDcube->hanode_sr.pbuffer+(hLEDcube->GPCounter_bytes*8 + hLEDcube->GPCounter_bits)*hLEDcube->hanode_sr.num_bytes, hLEDcube->hanode_sr.num_bytes);
}




























uint32_t LEDcube_get_index_offset(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z )
{
	if((X >= hLEDcube->bits_width) || (Y >= hLEDcube->bits_depth) || (Z >= hLEDcube->bits_height)){ return 0; }
	uint32_t bit_number = Z*(hLEDcube->bits_width*hLEDcube->bits_depth) + Y*(hLEDcube->bits_width) + X;
	return bit_number/8; 
}
uint8_t LEDcube_get_bit_offset(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z )
{
	if((X >= hLEDcube->bits_width) || (Y >= hLEDcube->bits_depth) || (Z >= hLEDcube->bits_height)){ return 0; }
	uint32_t bit_number = Z*(hLEDcube->bits_width*hLEDcube->bits_depth) + Y*(hLEDcube->bits_width) + X;
	return bit_number - (8*LEDcube_get_index_offset(hLEDcube, X, Y, Z));
}





void LEDcube_set_voxel(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z)
{
	uint32_t index = LEDcube_get_index_offset(hLEDcube,X,Y,Z);
	uint8_t bit = LEDcube_get_bit_offset(hLEDcube,X,Y,Z);
	uint8_t	mask = (( 0xFF << (bit+1)) | ( 0xFF >> (8 - bit)) );
	*(hLEDcube->hanode_sr.pbuffer + index) &= mask;
	*(hLEDcube->hanode_sr.pbuffer + index) |= ~mask;
}

void LEDcube_clear_voxel(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t Z)
{
	uint32_t index = LEDcube_get_index_offset(hLEDcube,X,Y,Z);
	uint8_t bit = LEDcube_get_bit_offset(hLEDcube,X,Y,Z);
	uint8_t	mask = (( 0xFF << (bit+1)) | ( 0xFF >> (8 - bit)) );
	*(hLEDcube->hanode_sr.pbuffer + index) &= mask;
}










void LEDcube_clear_all(LEDcube_HandleTypeDef *hLEDcube)
{
	for(uint8_t indi = 0; indi < hLEDcube->bits_width; indi++){
		for(uint8_t indj = 0; indj < hLEDcube->bits_depth; indj++){
			for(uint8_t indk = 0; indk < hLEDcube->bits_height; indk++){
				LEDcube_clear_voxel(hLEDcube, indi, indj, indk);
			}
		}
	}
}

uint32_t LEDcube_sum_all_bits(LEDcube_HandleTypeDef *hLEDcube)
{
	uint32_t num_bits = (hLEDcube->bits_width*hLEDcube->bits_depth*hLEDcube->bits_height);
	uint16_t byte_count = 0;
	uint32_t count = 0;
	uint8_t byte = 0;
	for(uint16_t indi = 0; indi < (num_bits/8); indi++){
		byte = *(hLEDcube->hanode_sr.pbuffer + byte_count);
		if(byte & 0x01){ count++; }
		if(byte & 0x02){ count++; }
		if(byte & 0x04){ count++; }
		if(byte & 0x08){ count++; }
		if(byte & 0x10){ count++; }
		if(byte & 0x20){ count++; }
		if(byte & 0x40){ count++; }
		if(byte & 0x80){ count++; }
		byte_count++;
	}
	for(uint8_t indi = 0; indi < (num_bits - byte_count*8); indi++){
		byte = *(hLEDcube->hanode_sr.pbuffer + byte_count);
		if(byte & (0x01 << indi)){ count++; }
	}
	return count;
}










void LEDcube_set_line_X(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end)
{
	for(uint8_t indi = start; indi <= end; indi++){
		LEDcube_set_voxel(hLEDcube, indi, Y, Z);
	}
}
void LEDcube_set_line_Y(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Z, uint8_t start, uint8_t end)
{
	for(uint8_t indi = start; indi <= end; indi++){
		LEDcube_set_voxel(hLEDcube, X, indi, Z);
	}
}
void LEDcube_set_line_Z(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t start, uint8_t end)
{
	for(uint8_t indi = start; indi <= end; indi++){
		LEDcube_set_voxel(hLEDcube, X, Y, indi);
	}
}

void LEDcube_clear_line_X(LEDcube_HandleTypeDef *hLEDcube, uint8_t Y, uint8_t Z, uint8_t start, uint8_t end)
{
	for(uint8_t indi = start; indi <= end; indi++){
		LEDcube_clear_voxel(hLEDcube, indi, Y, Z);
	}
}
void LEDcube_clear_line_Y(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Z, uint8_t start, uint8_t end)
{
	for(uint8_t indi = start; indi <= end; indi++){
		LEDcube_clear_voxel(hLEDcube, X, indi, Z);
	}
}
void LEDcube_clear_line_Z(LEDcube_HandleTypeDef *hLEDcube, uint8_t X, uint8_t Y, uint8_t start, uint8_t end)
{
	for(uint8_t indi = start; indi <= end; indi++){
		LEDcube_clear_voxel(hLEDcube, X, Y, indi);
	}
}


void LEDcube_set_rectangle_XY(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t ystart, uint8_t xend, uint8_t yend, uint8_t Z )
{
	for(uint8_t indi = ystart; indi <= yend; indi++){
		LEDcube_set_line_X(hLEDcube, indi, Z, xstart, xend);
	}
}
void LEDcube_set_rectangle_XZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t zstart, uint8_t xend, uint8_t zend, uint8_t Y )
{
	for(uint8_t indi = zstart; indi <= zend; indi++){
		LEDcube_set_line_X(hLEDcube, Y, indi, xstart, xend);
	}
}
void LEDcube_set_rectangle_YZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t ystart, uint8_t zstart, uint8_t yend, uint8_t zend, uint8_t X )
{
	for(uint8_t indi = zstart; indi <= zend; indi++){
		LEDcube_set_line_Y(hLEDcube, X, indi, ystart, yend);
	}
}

void LEDcube_clear_rectangle_XY(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t ystart, uint8_t xend, uint8_t yend, uint8_t Z )
{
	for(uint8_t indi = ystart; indi <= yend; indi++){
		LEDcube_clear_line_X(hLEDcube, indi, Z, xstart, xend);
	}
}
void LEDcube_clear_rectangle_XZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t xstart, uint8_t zstart, uint8_t xend, uint8_t zend, uint8_t Y )
{
	for(uint8_t indi = zstart; indi <= zend; indi++){
		LEDcube_clear_line_X(hLEDcube, Y, indi, xstart, xend);
	}
}
void LEDcube_clear_rectangle_YZ(LEDcube_HandleTypeDef *hLEDcube, uint8_t ystart, uint8_t zstart, uint8_t yend, uint8_t zend, uint8_t X )
{
	for(uint8_t indi = zstart; indi <= zend; indi++){
		LEDcube_clear_line_Y(hLEDcube, X, indi, ystart, yend);
	}
}


void LEDcube_set_rect_prism_CO(LEDcube_HandleTypeDef *hLEDcube, Coordinate_TypeDef corner1, Coordinate_TypeDef corner2 )
{
	// Arbitrarily I choose to loop over Z length and use the XY rectangle function. It really doesnt matter one way or another.
	uint8_t X1 = 0;
	uint8_t X2 = 0;
	uint8_t Y1 = 0;
	uint8_t Y2 = 0;
	uint8_t Z1 = 0;
	uint8_t Z2 = 0;

	if(corner1.X > corner2.X){ X2 = corner1.X; X1 = corner2.X; }
	else{ X2 = corner2.X; X1 = corner1.X; }
	if(corner1.Y > corner2.Y){ Y2 = corner1.Y; Y1 = corner2.Y; }
	else{ Y2 = corner2.Y; Y1 = corner1.Y; }
	if(corner1.Z > corner2.Z){ Z2 = corner1.Z; Z1 = corner2.Z; }
	else{ Z2 = corner2.Z; Z1 = corner1.Z; }

	for(uint8_t indi = Z1; indi <= Z2; indi++){
		LEDcube_set_rectangle_XY(hLEDcube,X1,Y1,X2,Y2,indi);
	}
}

void LEDcube_set_rect_prism_CE(LEDcube_HandleTypeDef *hLEDcube, Center_TypeDef center, Extent_TypeDef extent )
{
	// Arbitrarily I choose to loop over Z length and use the XY rectangle function. It really doesnt matter one way or another.

	uint8_t X1 = 0;
	uint8_t X2 = 0;
	uint8_t Y1 = 0;
	uint8_t Y2 = 0;
	uint8_t Z1 = 0;
	uint8_t Z2 = 0;


	if((extent.X & 0x01))
	{
		X1 = center.X - (extent.X/2);
		X2 = center.X + (extent.X/2);
	}
	else
	{
		X1 = center.X - (extent.X/2);
		X2 = center.X + (extent.X/2) - 1;
		if(extent.Xbias)
		{
			X1++; X2++;
		}
	}

	if((extent.Y & 0x01))
	{
		Y1 = center.Y - (extent.Y/2);
		Y2 = center.Y + (extent.Y/2);
	}
	else
	{
		Y1 = center.Y - (extent.Y/2);
		Y2 = center.Y + (extent.Y/2) - 1;
		if(extent.Ybias)
		{
			Y1++; Y2++;
		}
	}

	if((extent.Z & 0x01))
	{
		Z1 = center.Z - (extent.Z/2);
		Z2 = center.Z + (extent.Z/2);
	}
	else
	{
		Z1 = center.Z - (extent.Z/2);
		Z2 = center.Z + (extent.Z/2) - 1;
		if(extent.Zbias)
		{
			Z1++; Z2++;
		}
	}

	for(uint8_t indi = Z1; indi <= Z2; indi++){
		LEDcube_set_rectangle_XY(hLEDcube,X1,Y1,X2,Y2,indi);
	}
}


void LEDcube_set_sphere_CE(LEDcube_HandleTypeDef *hLEDcube, Center_TypeDef center, Extent_TypeDef extent )
{
	// This will only use the X portion of the extent to determine the size of the sphere
	// The Xbias, Ybias, and Zbias values will be used as well

	switch(extent.X)
	{
		case 0 : break;
		case 1 : 
			LEDcube_set_voxel(hLEDcube, center.X, center.Y, center.Z);
			break;
		case 2 :
			extent.X = 2;
			extent.Y = 2;
			extent.Z = 2;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent );
			break;
		case 3 : 
			extent.X = 3;
			extent.Y = 1;
			extent.Z = 1;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			extent.X = 1;
			extent.Y = 3;
			extent.Z = 1;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			extent.X = 1;
			extent.Y = 1;
			extent.Z = 3;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			break;
		case 4 :
			extent.X = 4;
			extent.Y = 2;
			extent.Z = 2;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			extent.X = 2;
			extent.Y = 4;
			extent.Z = 2;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			extent.X = 2;
			extent.Y = 2;
			extent.Z = 4;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			break;
		case 5 :
			extent.X = 3;
			extent.Y = 3;
			extent.Z = 3;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			extent.X = 5;
			extent.Y = 1;
			extent.Z = 1;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			extent.X = 1;
			extent.Y = 5;
			extent.Z = 1;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			extent.X = 1;
			extent.Y = 1;
			extent.Z = 5;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			break;
		case 6 :
			extent.X = 4;
			extent.Y = 4;
			extent.Z = 4;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			extent.X = 6;
			extent.Y = 2;
			extent.Z = 2;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			extent.X = 2;
			extent.Y = 6;
			extent.Z = 2;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			extent.X = 2;
			extent.Y = 2;
			extent.Z = 6;
			LEDcube_set_rect_prism_CE(hLEDcube, center, extent);
			break;

		default :
			LEDcube_set_voxel(hLEDcube, center.X, center.Y, center.Z);
			break;

	}
}
































