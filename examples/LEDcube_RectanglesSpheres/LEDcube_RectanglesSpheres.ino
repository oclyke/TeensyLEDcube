#include "LED_cube.h"

#define CATHODE_NUM_BYTES 1
#define ANODE_NUM_BYTES 8

LEDcube_HandleTypeDef CUBE;
IntervalTimer frame_timer;

uint8_t cathode_buff[CATHODE_NUM_BYTES];
uint8_t anode_buff[ANODE_NUM_BYTES*(8*CATHODE_NUM_BYTES)];

uint8_t bright = 255; //  x/255

void setup() {
  // put your setup code here, to run once:

  // Setup the handles:
  // CUBE
  CUBE.framerate = 64;
  CUBE.bits_width = 8;
  CUBE.bits_depth = 8;
  CUBE.bits_height = 8;

  
  // Anode
  CUBE.hanode_sr.num_bytes = ANODE_NUM_BYTES;
  CUBE.hanode_sr.OE_pin = 23;
  CUBE.hanode_sr.latch_pin = 22;
  CUBE.hanode_sr.clear_pin = 21;
  CUBE.hanode_sr.SPI_preferences.use_spi = 1;
  CUBE.hanode_sr.SPI_preferences.which = 0;
  CUBE.hanode_sr.SPI_preferences.freq = 1000000;
  CUBE.hanode_sr.pbuffer = &anode_buff[0];

  // Cathode
  CUBE.hcathode_sr.num_bytes = CATHODE_NUM_BYTES;
  CUBE.hcathode_sr.OE_pin = 2;
  CUBE.hcathode_sr.latch_pin = 3;
  CUBE.hcathode_sr.clear_pin = 4;
  CUBE.hcathode_sr.SPI_preferences.use_spi = 1;
  CUBE.hcathode_sr.SPI_preferences.which = 1;
  CUBE.hcathode_sr.SPI_preferences.freq = 100000;
  CUBE.hcathode_sr.pbuffer = &cathode_buff[0];

  LEDcube_begin(&CUBE);
  //frame_timer.begin(update_ISR,1000000/(CUBE.framerate*(8*CUBE.hcathode_sr.num_bytes)));
  frame_timer.begin(update_ISR,1000000/1024);
  
  analogWrite(CUBE.hanode_sr.OE_pin, 255-bright);
  randomSeed(analogRead(14));
}

void loop() {
  // put your main code here, to run repeatedly:
  Coordinate_TypeDef corner1, corner2;

  corner1.X = 2;
  corner1.Y = 0;
  corner1.Z = 2;

  corner2.X = 6;
  corner2.Y = 1;
  corner2.Z = 4;
  
  
  LEDcube_set_rect_prism_CO(&CUBE,corner1,corner2);

  delay(500);

  Extent_TypeDef extent1; // Use default to be sure everything is initialized
  extent1.X = 5;
  extent1.Y = 2;
  extent1.Z = 3;
  extent1.Xbias = 0;
  extent1.Ybias = 0;
  extent1.Zbias = 0;

  Center_TypeDef center1;
  center1.X = 4;
  center1.Y = 5;
  center1.Z = 3;

  LEDcube_set_rect_prism_CE(&CUBE, center1, extent1);

  delay(1500);
  LEDcube_clear_all(&CUBE);


  center1.X = 3;
  center1.Y = 3;
  center1.Z = 3;

  extent1.X = 3;
  
  LEDcube_set_sphere_CE(&CUBE, center1, extent1 );

  delay(1500);
  LEDcube_clear_all(&CUBE);

}

void update_ISR(void)
{
  LEDcube_update_frame(&CUBE);
}
