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

#include "Arduino.h"
#include "Wire.h"
#include "si5351mcu.h"

/*****************************************************************************
 * This is the custom init procedure, it's used to pass a custom xtal
 *****************************************************************************/
 void Si5351mcu::init(uint32_t nxtal) {
    // set the new xtal freq
    int_xtal = nxtal;
}


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
 * but it will carry on click noise...
 *
 * I think that +/- 3 Hz makes no difference in homebrew SSB equipment
 *
 * You can also implement a reset every X Khz/Mhz to be sure
 ****************************************************************************/
void Si5351mcu::reset(void) {
    // PLL resets

    // This soft-resets PLL A
    i2cWrite(177, 32);
    // This soft-resets PLL B
    i2cWrite(177, 128);
}


/*****************************************************************************
 * Function to disable all outputs
 *
 * The PLL are kept running, just the m-synths are powered off.
 *
 * This allows to keep the chip warm and exactly on freq the next time you
 * enable an output.
 *
 ****************************************************************************/
void Si5351mcu::off() {
    // This disable all the CLK outputs
    for (byte i=0; i<3; i++) disable(i);
}


/*****************************************************************************
 * Function to set the correction in Hz over the Si5351 XTAL.
 *
 * This will call a reset of the PLLs and multi-synths so it will produce a
 * click every time it's called
 *
 * You will get the correction applied in the next call o the setFreq()
 *
 ****************************************************************************/
void Si5351mcu::correction(int32_t diff) {
    // apply some corrections to the xtal
    int_xtal = base_xtal + diff;

    // reset the PLLs to apply the correction
    reset();
}


/*****************************************************************************
 * This function enables the selected output
 *
 * ZERO is clock output enabled
 *****************************************************************************/
void Si5351mcu::enable(uint8_t clk) {
    // selecting the registers depending on the clock
    switch (clk) {
        case 0:
            // setting register 16 to the correct values
            i2cWrite(16, SICLK0_R + clk0_power);
            break;
        case 1:
            // setting register 17 to the correct values
            i2cWrite(17, SICLK12_R + clk1_power);
            // disable CLK2 as it's mutually exclusive with this
            disable(2);
            break;
        case 2:
            // setting register 18 to the correct values
            i2cWrite(18, SICLK12_R + clk2_power);
            // disable CLK1 as it's mutually exclusive with this
            disable(1);
            break;
    }
}


/*****************************************************************************
 * This function disables the selected output
 *
 * ONE  is clock output disabled
 * *****************************************************************************/
void Si5351mcu::disable(uint8_t clk) {
    // send
    i2cWrite(16 + clk, 128);
}


/****************************************************************************
 * Set the power output for each output independently
 ***************************************************************************/
void Si5351mcu::setPower(byte clk, byte power) {
    // set the power to the correct var
    if (clk == 0) clk0_power = power;
    if (clk == 1) clk1_power = power;
    if (clk == 2) clk2_power = power;

    // now enable the output to get it applied
    enable(clk);
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
