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
  frame_timer.begin(update_ISR,1000000/(CUBE.framerate*(8*CUBE.hcathode_sr.num_bytes)));
  
  analogWrite(CUBE.hanode_sr.OE_pin, 255-bright);
  randomSeed(analogRead(14));
}

void loop() {
  // put your main code here, to run repeatedly:

  
  LEDcube_set_voxel(&CUBE,random(CUBE.bits_width),random(CUBE.bits_depth),random(CUBE.bits_height));
  LEDcube_clear_voxel(&CUBE,random(CUBE.bits_width),random(CUBE.bits_depth),random(CUBE.bits_height));
  LEDcube_clear_voxel(&CUBE,random(CUBE.bits_width),random(CUBE.bits_depth),random(CUBE.bits_height));
  LEDcube_clear_voxel(&CUBE,random(CUBE.bits_width),random(CUBE.bits_depth),random(CUBE.bits_height));

  delay(random(25));
}

void update_ISR(void)
{
  LEDcube_update_frame(&CUBE);
}
