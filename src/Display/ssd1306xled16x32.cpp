/*
   ssd1306xLED - Drivers for ssd1306 controlled dot matrix OLED/PLED 128x34 displays

   @created: 2014-08-12
   @author: Neven Boyanov

   Source code available at: https://bitbucket.org/tinusaur/ssd1306xled

*/

// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <avr/io.h>

#include <avr/pgmspace.h>

#include "ssd1306xled.h"
#include "ssd1306xled16x32.h"
#include "font16x32.h"

// ----------------------------------------------------------------------------


void SSD1306Device::ssd1306_char_f16x32(uint8_t x, uint8_t y, const char ch[])
{
  uint8_t c, j, i = 0, yCursor;
  while (ch[j] != '\0')
  {
    c = ch[j] - 44;

    ssd1306_setpos(x, y);
	yCursor = y;
    for (uint8_t j = 3; j > 1 ; j--) {     // Send from top to bottom 2*16 bytes
      ssd1306_send_data_start();
      for (uint8_t i = 0; i < 16; i++) {  // Send from top to bottom 16 vertical bytes
        ssd1306_send_byte(pgm_read_byte(&ssd1306xled_font16x32[c * 64 + 4 * i + j]));
      }
      ssd1306_send_data_stop();
	  yCursor++;
      ssd1306_setpos(x, yCursor);    // Prepare position of next row of 16 bytes
    }
    
    for (uint8_t j = 3; j > 1 ; j--) {    // Send from the lower half 2*16 bytes
      ssd1306_send_data_start();
      for (uint8_t i = 0; i < 16; i++) {  // Send from top to bottom 2*16 bytes
        ssd1306_send_byte(pgm_read_byte(&ssd1306xled_font16x32[c * 64 + 4 * i + j - 2]));
      }
      ssd1306_send_data_stop();
	  yCursor++;
      ssd1306_setpos(x, yCursor);        // Prepare position of last row of 16 bytes
    }

	x+=16;
	j++;
  }
}

// ----------------------------------------------------------------------------
