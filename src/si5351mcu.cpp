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

#include "Arduino.h"
#include "Wire.h"
#include "si5351mcu.h"

/*****************************************************************************
 * This function set the freq of the corresponding clock.
 *
 * In my tests my Si5351 can work between 7,8 Khz and ~225 Mhz, as usual YMMV
 ****************************************************************************/
void Si5351mcu::setFreq(uint8_t clk, unsigned long freq) { 
    #define c 1048574;
    unsigned long fvco;
    unsigned long outdivider;
    uint8_t R = 1;
    uint8_t a;
    unsigned long b;
    float f;
    unsigned long MSx_P1;
    unsigned long MSNx_P1;
    unsigned long MSNx_P2;
    unsigned long MSNx_P3;
    uint8_t shifts = 0;

    // With 900 MHz beeing the maximum internal PLL-Frequency
    outdivider = 900000000 / freq;

    // If output divider out of range (>900) use additional Output divider
    while (outdivider > 900) {
        R = R * 2;
        outdivider = outdivider / 2;
    }

    // finds the even divider which delivers the intended Frequency
    if (outdivider % 2) outdivider--;

    // Calculate the PLL-Frequency (given the even divider)
    fvco = outdivider * R * freq;

    // Convert the Output Divider to the bit-setting required in register 44
    switch (R) {
        case 1:   R = 0; break;
        case 2:   R = 16; break;
        case 4:   R = 32; break;
        case 8:   R = 48; break;
        case 16:  R = 64; break;
        case 32:  R = 80; break;
        case 64:  R = 96; break;
        case 128: R = 112; break;
    }

    a = fvco / int_xtal;
    f = fvco - a * int_xtal;
    f = f * c;
    f = f / int_xtal;
    b = f;

    MSx_P1 = 128 * outdivider - 512;
    f = 128 * b / c;
    MSNx_P1 = 128 * a + f - 512;
    MSNx_P2 = f;
    MSNx_P2 = 128 * b - MSNx_P2 * c; 
    MSNx_P3 = c;

    if (clk > 0 ) shifts = 8;

    // plls
    i2cWrite(26 + shifts, (MSNx_P3 & 65280) >> 8);   // Bits [15:8] of MSNx_P3 in register 26
    if (clk == 0) {
        i2cWrite(27, MSNx_P3 & 255);
        i2cWrite(28, (MSNx_P1 & 196608) >> 16);
    } else {
        i2cWrite(35, MSNx_P1 & 255);
        i2cWrite(36, (MSNx_P2 & 0x00030000) >> 10);
    }
    i2cWrite(29 + shifts, (MSNx_P1 & 65280) >> 8);   // Bits [15:8]  of MSNx_P1 in register 29
    i2cWrite(30 + shifts, MSNx_P1 & 255);            // Bits [7:0]  of MSNx_P1 in register 30
    i2cWrite(31 + shifts, ((MSNx_P3 & 983040) >> 12) | ((MSNx_P2 & 983040) >> 16)); // Parts of MSNx_P3 and MSNx_P1
    i2cWrite(32 + shifts, (MSNx_P2 & 65280) >> 8);   // Bits [15:8]  of MSNx_P2 in register 32
    i2cWrite(33 + shifts, MSNx_P2 & 255);            // Bits [7:0]  of MSNx_P2 in register 33

    shifts = clk * 8;

    // multisynths
    i2cWrite(42 + shifts, 0);                        // Bits [15:8] of MS0_P3 (always 0) in register 42
    i2cWrite(43 + shifts, 1);                        // Bits [7:0]  of MS0_P3 (always 1) in register 43
    i2cWrite(44 + shifts, ((MSx_P1 & 196608) >> 16) | R);  // Bits [17:16] of MSx_P1 in bits [1:0] and R in [7:4]
    i2cWrite(45 + shifts, (MSx_P1 & 65280) >> 8);    // Bits [15:8]  of MSx_P1 in register 45
    i2cWrite(46 + shifts, MSx_P1 & 255);             // Bits [7:0]  of MSx_P1 in register 46
    i2cWrite(47 + shifts, 0);                        // Bits [19:16] of MS0_P2 and MS0_P3 are always 0
    i2cWrite(48 + shifts, 0);                        // Bits [15:8]  of MS0_P2 are always 0
    i2cWrite(49 + shifts, 0);                        // Bits [7:0]   of MS0_P2 are always 0
    if (outdivider == 4 and clk == 0) {
        i2cWrite(44, 12 | R);       // Special settings for R = 4 (see datasheet)
        i2cWrite(45, 0);            // Bits [15:8] of MSx_P1 must be 0
        i2cWrite(46, 0);            // Bits [7:0] of MSx_P1 must be 0
    }

    // default reset
    if (!init) {
        // reset the PLLs and synths
        reset();

        // clear the flag
        init = true;
    }
}


/*****************************************************************************
 * Reset of the PLLs and multisynths output enable
 *
 * This must be called to soft reset the PLLs and cycle the output of the
 * multisynths: this is the "click" noise source in the RF spectrum.
 *
 * So it must be avoided at all costs, so this lib just call it once at the
 * initialization of the PLLs
 *
 * Not calling this after each freq update can cause some freq error, at
 * least in theory, in practice with my instruments I can't detect any error
 * with +/- 5Hz of tolerance as an educated guess from my part.
 *
 * If you need super extra accuracy you must call it after each freq change,
 * I think that +/- 3 Hz makes no difference in homebrew SSB equipment
 ****************************************************************************/
void Si5351mcu::reset(void) {
    // PLL & synths reset

    // This soft-resets PLL A & and enable it's output
    i2cWrite(177, 32);
    i2cWrite(16, 79);

    // This soft-resets PLL B & and enable it's output
    i2cWrite(177, 128);
    i2cWrite(17, 111);
}


/*****************************************************************************
 * Function to set the correction in Hz over the Si5351 XTAL.
 *
 * It's set but not applied, so you need to make a setFreq & reset after
 * changing the correction factor.
 *
 * You has been warned.
 ****************************************************************************/
void Si5351mcu::correction(long diff) {
    // apply some corrections to the xtal
    int_xtal = XTAL + diff;

    // unset the init flag to force a reset in the next setFreq()
    init = false;
}


/****************************************************************************
 * Private function to send the register data to the Si5351, arduino way.
 ***************************************************************************/
void Si5351mcu::i2cWrite(byte regist, byte value){
    Wire.beginTransmission(SIADDR);
    Wire.write(regist);
    Wire.write(value);
    Wire.endTransmission();
}

