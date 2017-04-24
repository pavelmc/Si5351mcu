/*
 * si5351mcu - Si5351 library for Arduino MCU tuned for size and click-less
 *
 * Copyright (C) 2017 Pavel Milanes <pavelmc@gmail.com>
 *
 * Many chunk of codes are derived-from/copied from other libs
 * all GNU GPL licenced:
 *  - Linux Kernel (www.kernel.org)
 *  - Hans Summers libs and demo code (qrp-labs.com)
 *  - Etherkit (NT7S) Si5351 libs on github
 *  - DK7IH example.
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
 * CLK2 will use PLLB
 *
 * Lib defaults
 * - XTAL is 27 Mhz.
 * - Always put the internal 8pF across the xtal legs to GND
 * - lowest power output (2mA)
 * 
 * The correction procedure is not for the PPM as other libs, this
 * is just +/- Hz to the XTAL freq, you may get a click noise after
 * applying a correction
 *
 * This lib doesn't need an init if you use default values, but if you need
 * to change the default xtal... then there is one to pass the custom xtal
 *
 ****************************************************************************/

#ifndef SI5351MCU_H
#define SI5351MCU_H

// rigor includes
#include "Arduino.h"
#include "Wire.h"
#include <stdint.h>

// Xtal, 27.0 MHz
#define SIXTAL 27000000

// default I2C address of the Si5351
#define SIADDR 0x60

// register's power modifiers
#define SIOUT_2mA 0
#define SIOUT_4mA 1
#define SIOUT_6mA 2
#define SIOUT_8mA 3

// registers base (2mA by default)
#define SICLK0_R   76       // 0b1001100
#define SICLK12_R 108       // 0b1101100


class Si5351mcu {
    public:
        // custom init procedure (XTAL in Hz);
        void init(uint32_t);
        
        // reset all PLLs
        void reset(void);
        
        // set CLKx(0..2) to freq (Hz)
        void setFreq(uint8_t, uint32_t);
        
        // pass a correction factor
        void correction(int32_t);
        
        // enable some CLKx output
        void enable(uint8_t);
        
        // disable some CLKx output
        void disable(uint8_t);
        
        // disable all outputs
        void off(void);

        // set power output to a specific clk
        void setPower(uint8_t, uint8_t);
        
    private:
        // used to talk with the chip, via Arduino Wire lib
        void i2cWrite(uint8_t, uint8_t);
        
        // set an internal xtal reference, over this value will
        // be applied the correction factor
        uint32_t int_xtal = SIXTAL;
        
        // initialized state of the lib
        boolean inited = false;

        // clks power holders (2ma by default)
        uint8_t clk0_power = 0;
        uint8_t clk1_power = 0;
        uint8_t clk2_power = 0;
};


#endif //SI5351MCU_H
