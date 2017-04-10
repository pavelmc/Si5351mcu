/*
 * si5351mcu - Si5351 library for Arduino MCU tuned for size and click-less
 *
 * Copyright (C) 2017 Pavel Milanes <pavelmc@gmail.com>
 *
 * Many chunk of codes are derived-from/copied from other libs
 * all GNU GPL licenced:
 *  - Linux Kernel (www.kernel.org)
 *  - Hans Summers libs and demo code (qrp-labs.com)
 *  - Etherkit Si5351 libs on github
 *  - DK7IH examples.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/****************************************************************************
 *  This lib tricks:
 * 
 * CLK0 will use PLLA
 * CLK1 will use PLLB
 * CLK2 will not be used at all
 *
 * XTAL is 27 Mhz, but you can set your particular xtal in the define below
 *
 * This lib put the 8pF across the xtal by default, and will set full power
 * on the output of the Si5351.
 *
 * The correction procedure is not for the PPM as other libs, this is just
 * +/- Hz to the XTAL freq, and you must do a setFrequency() and reset()
 * after applying any correction.
 *  
 ****************************************************************************/

#ifndef SI5351MCU_H
#define SI5351MCU_H

// rigor includes
#include "Arduino.h"
#include "Wire.h"
#include <stdint.h>

// lib defines
#define XTAL 27000000   // Xtal, 27.0 MHz
#define SIADDR 0x60     // default I2C address of the Si5351

class Si5351mcu {
    public:
        void reset(void);
        void setFreq(uint8_t, uint32_t);
        void correction(long);
    private:
        void i2cWrite(uint8_t, uint8_t);
        uint32_t int_xtal = XTAL;
        boolean init = false;
};


#endif //SI5351MCU_H
