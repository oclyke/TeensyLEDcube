/* 

Demo of the LED_cube.h library.
Fills up the cube until a theshold, and then removes LEDs

Owen Lyke: April 2018

*/ 
#include "LED_cube.h"           // Include the required library, it takes care of including SPI as well

#define CATHODE_NUM_BYTES 1     // I use 8 ground planes (height = 8), making for a total of 1 byte in my cathode shift register. Even just one more bit would bump this number to 2
#define ANODE_NUM_BYTES 8       // With 8*8 (width*depth) I have 64 bits in my anode shift register, or 8 bytes. Even just one more bit would bump this number to 9

LEDcube_HandleTypeDef CUBE;     // I make an instance of an LEDcube_HandleTypeDef to make working with the cube easier. I call it CUBE.
IntervalTimer frame_timer;      // I also use Paul S' IntervalTimer to simplify timer-based interrupts. We use this to control the framerate.

uint8_t cathode_buff[CATHODE_NUM_BYTES];                        // These two lines make data buffers for the cube. This is for the cathode, or ground plane control. This buffer will not be used directly by the user
uint8_t anode_buff[ANODE_NUM_BYTES*(8*CATHODE_NUM_BYTES)];      // This one is for the anode, or high side control. Note that the length is greater than ANODE_NUM_BYTES. This is because you encode height information in this array as well.

uint8_t bright = 255;       //  x/255                           // You can also perform PWM dimming of the whole cube by using the output enable (OE) pin on the shift registers.
uint8_t direct = 1;         // Initial direction of the fill
uint8_t offset_high = 20;   // How far from full you can be before changing direction
uint8_t offset_low = 2;     // How far from empty you can be before changing direction

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

  // Now begin the cube and frame update timer. Beginning the CUBE starts SPI if needed
  LEDcube_begin(&CUBE);
  frame_timer.begin(update_ISR,1000000/(CUBE.framerate*(8*CUBE.hcathode_sr.num_bytes)));    // The cube is actually updated faster than the specified update rate, because it must loop through all ground planes and rely on Persistence of Vision (POV) to create the final image
  
  // And start PWM dimming of the LEDs if desired. Then provide a psuedo-random seed
  analogWrite(CUBE.hanode_sr.OE_pin, 255-bright);
  randomSeed(analogRead(14));
}

void loop() {
  // put your main code here, to run repeatedly:

  if(direct == 1)
  {
    LEDcube_set_voxel(&CUBE,random(CUBE.bits_width),random(CUBE.bits_depth),random(CUBE.bits_height));
  }
  else
  {
    LEDcube_clear_voxel(&CUBE,random(CUBE.bits_width),random(CUBE.bits_depth),random(CUBE.bits_height));
  }
  
  uint32_t num_pix_on = LEDcube_sum_all_bits(&CUBE);
  Serial.print("Number of voxels on: "); Serial.println(num_pix_on); 
  if((num_pix_on >= ((CUBE.bits_width * CUBE.bits_depth * CUBE.bits_height ) - offset_high)) || (num_pix_on <= 0 + offset_low))
  {
    direct *= -1;
  }

  delay(random(25));

}

// This function is required to use the IntervalTimer to udate the cube
void update_ISR(void)
{
  LEDcube_update_frame(&CUBE);
}
