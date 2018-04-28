#include "LED_cube.h"

#define CATHODE_NUM_BYTES 1
#define ANODE_NUM_BYTES 8

LEDcube_HandleTypeDef CUBE;
IntervalTimer frame_timer;

Center_TypeDef center1;
Extent_TypeDef extent1;

typedef struct{
  int8_t X;
  int8_t Y;
  int8_t Z;
}Velocity_TypeDef;

Velocity_TypeDef velo;

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

  center1.X = 0;
  center1.Y = 0;
  center1.Z = 0;
  extent1.X = 3;

  velo.X = 0;
  velo.Y = 0;
  velo.Z = 0;
}

void loop() {
  // put your main code here, to run repeatedly:

  
  LEDcube_clear_all(&CUBE);
  LEDcube_set_sphere_CE(&CUBE, center1, extent1 );
  delay(250);

  center1.X = center1.X + velo.X; 
  center1.Y = center1.Y + velo.Y;
  center1.Z = center1.Z + velo.Z;

  if(center1.X <= 1)
  {
    center1.X = 1;
    velo.X *= -1;
    if(random(256) > 220)
    {
      velo.X += 1;
    }
    if(random(256) > 220)
    {
      velo.Y += 1;
    }
    if(random(256) > 220)
    {
      velo.Z += 1;
    }
  }
  if(center1.X >= CUBE.bits_width-2)
  {
    center1.X = CUBE.bits_width -2;
    velo.X *= -1;
    if(random(256) > 220)
    {
      velo.X += 1;
    }
    if(random(256) > 220)
    {
      velo.Y += 1;
    }
    if(random(256) > 220)
    {
      velo.Z += 1;
    }
  }

  
  if(center1.Y <= 1)
  {
    center1.Y = 1;
    velo.Y *= -1;
    if(random(256) > 220)
    {
      velo.Y += 1;
    }
    if(random(256) > 245)
    {
      velo.Z += 1;
    }
    if(random(256) > 245)
    {
      velo.X += 1;
    }
  }
  if(center1.Y >= CUBE.bits_depth-2)
  {
    center1.Y = CUBE.bits_depth -2;
    velo.Y *= -1;
    if(random(256) > 220)
    {
      velo.Y += 1;
    }
    if(random(256) > 245)
    {
      velo.Z += 1;
    }
    if(random(256) > 245)
    {
      velo.X += 1;
    }
  }

  
  if(center1.Z <= 1)
  {
    center1.Z = 1;
    velo.Z *= -1;
    if(random(256) > 220)
    {
      velo.Z += 1;
    }
    if(random(256) > 245)
    {
      velo.Y += 1;
    }
    if(random(256) > 245)
    {
      velo.X += 1;
    }
  }
  if(center1.Z >= CUBE.bits_height-2)
  {
    center1.Z = CUBE.bits_height -2;
    velo.Z *= -1;
    if(random(256) > 220)
    {
      velo.Z += 1;
    }
    if(random(256) > 245)
    {
      velo.Y += 1;
    }
    if(random(256) > 245)
    {
      velo.X += 1;
    }
  }

}

void update_ISR(void)
{
  LEDcube_update_frame(&CUBE);
}
