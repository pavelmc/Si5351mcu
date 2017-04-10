/*
 * si5351mcu - Si5351 library for Arduino MCU tuned for size and click-less
 *
 * This is the packed example.
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


/***************************************************************************
 * This example is meant to be monitored with an RTL-SDR receiver
 * but you can change the frequency and other vars to test with your hardware
 *
 * Take into account your XTAL error, see pll.correction(###) below
 *
 ***************************************************************************/

#include "si5351mcu.h"
#include "Wire.h"

// lib instantiation as "pll"
Si5351mcu pll;

// Stop every X Mhz for Y seconds to measure
#define EVERY       1000000   // stop every 1Mhz for...
#define SECONDS       15000   // 15 seconds

// some variables
long freqStart =   26000000;  //  26.0 MHz
long freqStop  =  150000000;  // 150.0 MHz
long step      =      10000;  //  10.0 kHz
long freq      = freqStart;
long newStop   = freq + EVERY;


void setup() {
    // init the wire lib
    Wire.begin();

    // apply my calculated correction factor
    pll.correction(-1250);

    // set some freq
    pll.setFreq(0, 25000000);       // CLK0 output
    pll.setFreq(1, 145000000);      // CLK1 output

    // force the first reset
    pll.reset();

    // 30 seconds delay for calibration
    delay(30000);

    // serial welcome
    Serial.begin(115200);
    Serial.println("Test for Si5351 click-less lib optimized for MCU");
}


void loop() {
    // check for the stop to measure
    if (freq >= newStop) {
        // yes it's time to measure
        
        // alert via serial
        Serial.print("Frequency: ");
        Serial.print(freq);
        Serial.println(" Hz");
        
        // stop for this time to measure
        delay(SECONDS);

        // reset the flag
        newStop = freq + EVERY;
    } else {
        // time to move

        // check if we are on the limits
        if (freq <= freqStop) {
            // no, set and increment
            pll.setFreq(0,freq);        // but it can be with CLK1 instead

            // set it for the new cycle
            freq += step;

            // a short delay to slow things a little.
            delay(10);
        } else {
            // we reached the limit, reset to start
            freq = freqStart;
        }
    }
}
