/* 

Demo of the LED_cube.h library.
Unfinished attempt at animating a ball bouncing around inside the LED cube. Mostly unfinished because it is choppy with only 8*8*8 resolution. Maybe we can do something neat to "fade" the ball between pixels???

Owen Lyke: April 2018

*/ 
#include "LED_cube.h"           // Include the required library, it takes care of including SPI as well

#define CATHODE_NUM_BYTES 1     // I use 8 ground planes (height = 8), making for a total of 1 byte in my cathode shift register. Even just one more bit would bump this number to 2
#define ANODE_NUM_BYTES 8       // With 8*8 (width*depth) I have 64 bits in my anode shift register, or 8 bytes. Even just one more bit would bump this number to 9

LEDcube_HandleTypeDef CUBE;     // I make an instance of an LEDcube_HandleTypeDef to make working with the cube easier. I call it CUBE.
IntervalTimer frame_timer;      // I also use Paul S' IntervalTimer to simplify timer-based interrupts. We use this to control the framerate.

Center_TypeDef center1;
Extent_TypeDef extent1;

typedef struct{
  int8_t X;
  int8_t Y;
  int8_t Z;
}Velocity_TypeDef;

Velocity_TypeDef velo;

uint8_t cathode_buff[CATHODE_NUM_BYTES];                        // These two lines make data buffers for the cube. This is for the cathode, or ground plane control. This buffer will not be used directly by the user
uint8_t anode_buff[ANODE_NUM_BYTES*(8*CATHODE_NUM_BYTES)];      // This one is for the anode, or high side control. Note that the length is greater than ANODE_NUM_BYTES. This is because you encode height information in this array as well.

uint8_t bright = 255;       //  x/255                           // You can also perform PWM dimming of the whole cube by using the output enable (OE) pin on the shift registers.

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  delay(1000);

  // Setup the handles:
  // CUBE: set the size and desired framerate for the cube. Your size parameters must be compatible with the sizes of the anode and cathode data buffers
  CUBE.framerate = 64;  
  CUBE.bits_width = 8;
  CUBE.bits_depth = 8;
  CUBE.bits_height = 8;

  
  // Anode: setup the anode shift register
  CUBE.hanode_sr.num_bytes = ANODE_NUM_BYTES;
  CUBE.hanode_sr.OE_pin = 23;
  CUBE.hanode_sr.latch_pin = 22;
  CUBE.hanode_sr.clear_pin = 21;
  CUBE.hanode_sr.SPI_preferences.use_spi = 1;
  CUBE.hanode_sr.SPI_preferences.which = 0;
  CUBE.hanode_sr.SPI_preferences.freq = 1000000;
  CUBE.hanode_sr.pbuffer = &anode_buff[0];

  // Cathode setup the cathode shift register
  CUBE.hcathode_sr.num_bytes = CATHODE_NUM_BYTES;
  CUBE.hcathode_sr.OE_pin = 2;
  CUBE.hcathode_sr.latch_pin = 3;
  CUBE.hcathode_sr.clear_pin = 4;
  CUBE.hcathode_sr.SPI_preferences.use_spi = 1;
  CUBE.hcathode_sr.SPI_preferences.which = 1;
  CUBE.hcathode_sr.SPI_preferences.freq = 100000;
  CUBE.hcathode_sr.pbuffer = &cathode_buff[0];


  center1.X = 0;
  center1.Y = 0;
  center1.Z = 0;
  extent1.X = 3;

  velo.X = 0;
  velo.Y = 0;
  velo.Z = 0;

  // Now begin the cube and frame update timer. Beginning the CUBE starts SPI if needed
  LEDcube_begin(&CUBE);
  frame_timer.begin(update_ISR,1000000/(CUBE.framerate*(8*CUBE.hcathode_sr.num_bytes)));    // The cube is actually updated faster than the specified update rate, because it must loop through all ground planes and rely on Persistence of Vision (POV) to create the final image
  
  // And start PWM dimming of the LEDs if desired. Then provide a psuedo-random seed
  analogWrite(CUBE.hanode_sr.OE_pin, 255-bright);
  randomSeed(analogRead(14));
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
