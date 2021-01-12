/*
 * si5351mcu - Si5351 library for Arduino, MCU tuned for size and click-less
 *
 * Serial Console Test Harness.
 *
 * Copyright (C) 2017 Pavel Milanes <pavelmc@gmail.com>
 *
 * Many chunk of codes are derived-from/copied from other libs
 * all GNU GPL licenced:
 *  - Linux Kernel (www.kernel.org)
 *  - Hans Summers libs and demo code (qrp-labs.com)
 *  - Etherkit (NT7S) Si5351 libs on github
 *  - DK7IH example.
 *  - Jerry Gaffke integer routines for the bitx20 group
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
 
/* 
 * Serial Console Test Harness code added
 *    2021-10-01 - Iain W. Bird, 2E0WZR
 *    
 */

/* 
 * CAUTION:  As this is for testing purposes only, there is little 
 *           validation or error checking.
 * 
 */
 
/***************************************************************************
 * Type HELP into the Arduino Serial Console line for a list of commands.
 *
 * Take into account your XTAL error, see Si.correction(###) below
 ***************************************************************************/

#include "si5351mcu.h"

// lib instantiation as "Si"
Si5351mcu Si;

int   cmdHelp( void );
void  resetInput( void );
float getPllFraction( uint8_t pllIndex );
void  printDecimalByte( int val );

uint8_t devicePresent = 0;

void setup() {


    // init the Si5351 lib
    Si.init(25000000);

    // For a different xtal (from the default of 27.00000 Mhz)
    // just pass it on the init procedure, just like this
    // Si.init(26570000);

    // set & apply my calculated correction factor
    Si.correction(-1250);

    // Set frequencies
    Si.setFreq(0, 100000);
    Si.setFreq(1, 1000000);

    // reset the PLLs
    Si.reset();
    Si.off();
        
    Serial.begin(115200);
    while (!Serial);    // Wait for devices such as Leonardo to start Serial
    
    Serial.println(F(""));
    Serial.println(F("**********************************************************"));
    Serial.println(F(" Si5351a Serial Console Test Bench"));
    Serial.println(F(""));
    Serial.println(F("  Please refer to document AN619 for register definitions."));
    Serial.println(F(""));
    Serial.println(F("  If you choose to modify undocumented registers, "));
    Serial.println(F("  or go beyond specification, it is at your own risk."));
    Serial.println(F(""));
    Serial.println(F("**********************************************************"));
    Serial.println(F(""));

    if ( devicePresent ) {
      cmdHelp();
    }
    else {
      Serial.println(F("DEVICE MISSING"));
      
    }
    resetInput();
}

#define MAXINPUT 127

char inputBuffer[ MAXINPUT + 1] = { 0 };
int  inputIndex = 0;
char *args[ MAXINPUT + 1]  = { 0 };
uint8_t output_chan = 0;

void resetInput( void ) {
  memset( inputBuffer, 0, MAXINPUT + 1 );
  memset( args, 0, MAXINPUT + 1 );
  inputIndex = 0;
  Serial.print( output_chan, DEC );
  Serial.print(F(":> "));
}

void printError( char *str ) {
  Serial.print(F("ERROR: "));
  Serial.println( str );
}

void printWarning( char *str ) {
  Serial.print(F("WARNING: "));
  Serial.println( str );
}

int8_t displayClockSource( uint8_t clkIndex ) {
  int16_t   val;
  uint32_t  uval;
  int8_t    result = -1;
  uint8_t   intdiv = 1;
  bool      blnIntDiv = false;
  bool      blnIntDiv4 = false;
  float     pllMult = 1;
  float     pllFreq = 1;
  float     msynDiv = 1;
  float     active = 1;
  
  do { // while(0) - Artificial construct to allow break;
    
    uval = val = Si.i2cRead( 16 + clkIndex );
    if ( val < 0 ) {
      break;
    }
    Serial.print( clkIndex, DEC );
    Serial.print( F(": PD=") );
    Serial.print( (uval & 0x80) >> 7, DEC );    // Power down state
    active = !((uval & 0x80) >> 7);

    val = Si.i2cRead( 44 + clkIndex * 8 );
    if ( val >= 0 ) {
      switch ( ( val & 0x0c ) >> 2 ) {
        case 3:
          blnIntDiv4 = true;
          // drop through is intentional - no break;     
        case 0:
          intdiv = (val & 0x70) >> 4;
          intdiv = 1 << intdiv;
          blnIntDiv = true;
          break;
        default:
          intdiv = 0;   // Unknown state
          break;
      }
    }    
    Serial.print( F(", R=" ) );
    Serial.print( intdiv , DEC );    
    
    Serial.print( F(", Div=") );
    if ( uval & 0x40 || clkIndex >= 6 ) {
        Serial.print(F("Int, PLL="));
    }
    else {
      Serial.print( F("Frac, PLL=") );
    }
    // PLL
    Serial.print( (char) ('A' + (( uval & 0x20 ) >> 5) ));  // PLL 'A' or 'B'
    Serial.print( F(", INV=" ) );

    pllMult = getPllFraction( ( uval & 0x20 ) >> 5 );
    pllFreq = pllMult * Si.getXtalCurrent();
    
    Serial.print( (( uval & 0x10 ) >> 4), DEC );
    Serial.print( F(", Src=" ) );
    switch( ( uval & 0x0C ) >> 2 ) {
    case 0:
        Serial.print( F("XTAL, ") );
        active = 0;
        break;  
    case 1:
        Serial.print( F("CLKIN, ") );
        active = 0;
        break;  
    case 2:
        if ( clkIndex == 0 ) {
          Serial.print( F("MSYN0/Undefined:") );
        }
        else {
          Serial.print( F("MSYN0/R:") );
        }
        Serial.print( msynDiv = getMsynthFraction( 0 ));
        Serial.print( F(", ") );
        break;  
    case 3:
    default:
        Serial.print( F("MSYN" ) );
        Serial.print( clkIndex, DEC );
        Serial.print( F(":") );
        Serial.print( msynDiv = getMsynthFraction( clkIndex ));
        Serial.print( F(", ") );
        break;  
    }
    Serial.print( F("Pwr=") );
    Serial.print( uval & 0x03, DEC );
    Serial.print( F(", Frequency=") );
    if ( blnIntDiv4 ) {
      // Special divide by 4
      // msynDiv = 1.0;          
    }

    Serial.print( active * pllFreq / ( msynDiv * intdiv ));
    Serial.println(F(""));
     
  } while ( 0 );
  
  
  return result;
}

//
// CAUTION - USE OF DOUBLE OR FLOAT GREATLY INCREASES BUILD SIZE
// This is for diagnostic purposes only.
//
// 'float' does not have sufficient precision for values to be accurate
//

// The same register offsets work for both the PLL fractions
// and the multi-synth dividers (in integer mode)
// 

float getFractionalRatio( uint8_t register_base ) {
  uint32_t MSNx_P1;   // 18 bit
  uint32_t MSNx_P2;   // 20 bit
  uint32_t MSNx_P3;   // 20 bit
  int16_t  val;
  uint32_t uval;
  uint8_t  reg = register_base;
  float    result = -1.0;   // returns -1.0 on error
  float    abc;
  
  do { // while(0) - Artificial construct to allow break;
    
    uval = val = Si.i2cRead( reg++ );   // + 0
    if ( val < 0 ) {
      break;
    }
    MSNx_P3 = uval << 8;
    
    uval = val = Si.i2cRead( reg++ );   // + 1
    if ( val < 0 ) {
      break;
    }
    MSNx_P3 |= (uval & 0xFF);

    uval = val = Si.i2cRead( reg++ );   // + 2
    if ( val < 0 ) {
      break;
    }
    MSNx_P1 = ( uval & 0x03) << 16;

    uval = val = Si.i2cRead( reg++ );   // + 3
    if ( val < 0 ) {
      break;
    }
    MSNx_P1 |= ((uval & 0xFF) << 8);
    
    uval = val = Si.i2cRead( reg++ );   // + 4
    if ( val < 0 ) {
      break;
    }
    MSNx_P1 |= (uval & 0xFF);

    uval = val = Si.i2cRead( reg++ );   // + 5
    if ( val < 0 ) {
      break;
    }
    MSNx_P3 |= ((uval & 0xF0) << 12);
    MSNx_P2 = ((uval & 0x0F) << 16);

    uval = val = Si.i2cRead( reg++ );   // + 6
    if ( val < 0 ) {
      break;
    }
    MSNx_P2 |= ((uval & 0xFF) << 8);
    
    uval = val = Si.i2cRead( reg++ );   // + 7
    if ( val < 0 ) {
      break;
    }
    MSNx_P2 |= (uval & 0xFF);

    // Re-arranging equations to solve ( a + b/c )
    //    (a + b/c) = ( P1 + P2/c + 512.0 ) / 128.0

    if ( MSNx_P3 == 0 ) {
      // divide by zero error - c == 0
      break;
    }
    
    else // above is the last if() that can fail/break, calculate result.
    {
      abc = ( MSNx_P1 + ( (float) MSNx_P2 ) / MSNx_P3 + 512 ) / 128.0;
  
      result = abc;
    }
    
  } while(0);
  
  return result;
}

// PLL Fractions are Multipliers of the clock frequency
float getPllFraction( uint8_t pllIndex ) {
  // this is a test harness - no validation

  return getFractionalRatio( 26 + 8 * pllIndex );
}

// Multisynth fractions are Dividers of the providing PLL
float getMsynthFraction( uint8_t msIndex ) {
  // this is a test harness - no validation

  float result;
  if ( msIndex < 6 ) {
    result = getFractionalRatio( 42 + 8 * msIndex );
  }
  else {
    // different equations for integer only MSYTH 6 and 7 
    // registers 90/92 and 91/93
    result = -1.0;    // not implemented yet
  }
  return result;
}

//
void printByte( int16_t val ) {
  char buf[6] = { 0 };

  if ( val < 0 ) {
    sprintf( buf, "??" );
  }
  else {
    sprintf( buf, "%02X", (uint8_t) val);
  }
  Serial.print( buf );
}

//
void printDecimalByte( int16_t val ) {
  char buf[6] = { 0 };

  if ( val < 0 ) {
    sprintf( buf, "???" );
  }
  else {
    sprintf( buf, "%3d", (uint8_t) val);
  }
  Serial.print( buf );
}

//
// Displays a hexadecimal data dump of a byte range of registers
//
int cmdSubsetDump( int start, int count, int stride ) {
  int j, k;
  int val;
  
  for ( j = start; j < (start + stride * count); j+= stride ) {
    printByte( j );
    Serial.print(": ");
    for ( k = 0; k < stride; k++ ) {
      val = Si.i2cRead( j + k );
      printByte( val );
      Serial.print(" ");
    }
    Serial.println(F(""));
  }
  return 0;
}

//
// Show in HEX, all registers
//
int cmdRegDump( void ) {
  return cmdSubsetDump( 0, 16, 16 );
}

//
// Show in HEX, the registers for a PLL multiplier
//
int cmdPllDump( void ) {
  return cmdSubsetDump( 26, 2, 8 );
}

//
// Show in HEX, the registers for a MSYNTH divider
//
int cmdSynthDump( void ) {
  return cmdSubsetDump( 42, SICHANNELS, 8 );
}

//
// Show general characteristics from the register state;
// floats will be inaccurate and are for guidance only
//
int cmdShow( void ) {
  int j;
  float frq;
  
  Serial.print("Selected Command Channel Output: ");
  Serial.println( output_chan, DEC );
  Serial.println(F(""));
  Serial.println(F("XTAL:"));
  Serial.print(F("  Base:    "));
  Serial.println( Si.getXtalBase(), DEC );
  Serial.print(F("  Current: "));
  Serial.println( Si.getXtalCurrent(), DEC );
  Serial.println(F(""));
  Serial.println(F("PLL ratios:"));
  Serial.print(F("  A "));
  Serial.print( frq = getPllFraction( 0 ));
  Serial.print(F("  "));
  Serial.println( frq * Si.getXtalCurrent() );
  Serial.print(F("  B "));
  Serial.print( frq = getPllFraction( 1 ));
  Serial.print(F("  "));
  Serial.println( frq * Si.getXtalCurrent() );
  Serial.println(F(""));

  for( j = 0; j < SICHANNELS; j++ ) {
    displayClockSource( j );
  }
  return 0;
}

/*
int cmdCal() {
  // Does not seem to work
  Si.off();
  Si.enable(0);
  Si.i2cWrite( 16, 0 ); // Send XTAL direct to Output 0

  return 0;
}
*/

int cmdClkChanSet( int32_t chan ) {
  if ( chan < SICHANNELS && chan >= 0 ) {
    output_chan = chan;
  }
  else {
    printError("Invalid output channel chosen.");
  }
  return 0;
  
}

int cmdFrqSet( int32_t frq ) {
  if ( frq <= 0 ) {
    Si.disable(output_chan);
    printError("Disabled - Frequency <= 0.");
  }
  else {
    Si.setFreq(output_chan, frq);
    Serial.print(F("FRQ: Frequency set to "));
    Serial.print( frq , DEC );
    Serial.println(F(" Hz"));
    
    if ( frq < 8000 ) {
      printWarning(("Frequencies below around 8000 Hz may not function correctly."));
      printWarning(("The exact value depends on your XTAL frequency."));
    }
    if ( frq > 200000000 ) {
      printWarning(("Frequencies above 200MHz may not function correctly."));
      printWarning(("The exact value depends on your XTAL frequency and overclock settings."));
    }
  }
  return 0;
}

int cmdPwrSet( int32_t power ) {
  Si.setPower(output_chan, power);
  return 0;
}

int cmdEna( void ) {
  Si.enable(output_chan);
  return 0;
}

int cmdDis( void ) {
  Si.disable(output_chan);
  return 0;
}

int cmdOff( void ) {
  Si.off();
  return 0;
}

int cmdReset( void ) {
  Si.reset();
  return 0;
}

int cmdClkSetState( uint8_t clk, uint8_t state ) {
  return 0;
}

int cmdRegSet( int32_t val, int32_t val2 ) {
  int result = 0;

  if ( val < 0 || val > 255 || val2 < 0 || val2 > 255 ) {
    result = -1;
  }
  else
  {
    printWarning("Use of SET may cause the library and this tool to " \
                  "display results inconsistent with the actual output.");
                  
    Si.i2cWrite( val, val2 );
  }
  
  return result;
}

int cmdXtal( int32_t val, int32_t val2 ) {
  int result = -1;

  if ( val < 10000000 || val > 32000000 ) {
    printError("XTAL frequency should be between 10 MHz and 32 MHz."); 
  }
  else if ( val2 < -1000000 || val2 > 1000000 ) {
    printError("XTAL error should be +/- 1 MHz."); 
  }
  else
  {
    printWarning("Si.init() will be called."); 
    Si.init( val );
    Si.correction( val2 );
    result = 0;
  }
  
  return result;
}

int cmdHelp( void ) {
  Serial.println(F("Usage:"));
  Serial.println(F("  HELP         - Displays this help message."));
  // Serial.println(F("  CAL          - Calibration Check - Output direct XTAL on Channel 0."));
  Serial.println(F("  CHAN <0|1|2> - Select output parameter channel."));
  Serial.println(F("  DIS          - Disable selected output."));
  Serial.println(F("  ENA          - Enable selected output."));
  Serial.println(F("  FRQ <N>      - Set selected parameter channel frequency to N Hz."));
  Serial.println(F("  OFF          - Disable all outputs."));
  Serial.println(F("  PLLDUMP      - Display Si5351 PLL registers."));
  Serial.println(F("  PWR <0-3>    - Set output current for selected output channel."));
  Serial.println(F("  REGDUMP      - Display Si5351 all registers (hex)."));
  Serial.println(F("  RESET        - PLL Reset Si5351."));
  Serial.println(F("  SET <R> <N>  - Set Si5351 register 'R' to 'N' (decimal)."));
  Serial.println(F("  SHOW         - Show present parameters."));
  Serial.println(F("  MSYNDUMP     - Display Si5351 MSynth registers."));
  Serial.println(F("  XTAL <N> <E> - Set XTAL Clock Frequency, to <N> Hz, with error offset <E> Hz."));
  
  return 0;
}

//
// Take a command that came from the serial input, and
// execute it, if accepted.
//
int cmdProcess( int nargs ) {
    int done = -1;
    int32_t val = 0;
    int32_t val2 = 0;
    
    switch( nargs )
    {
      case 1:
        if ( !strcmp( "HELP", args[0] ) || !strcmp("?", args[0] ) ) {
          done = cmdHelp(); 
        }
        /*
        else if ( !strcmp( "CAL", args[0] ) ) {
          done = cmdCal(); 
        }
        */
        else if ( !strcmp( "ENA", args[0] ) ) {
          done = cmdEna(); 
        }
        else if ( !strcmp( "DIS", args[0] ) ) {
          done = cmdDis(); 
        }
        else if ( !strcmp( "OFF", args[0] ) ) {
          done = cmdOff(); 
        }
        else if ( !strcmp( "RESET", args[0] ) ) {
          done = cmdReset(); 
        }
        else if ( !strcmp( "SHOW", args[0] ) ) {
          done = cmdShow(); 
        }
        else if ( !strcmp( "REGDUMP", args[0] ) ) {
          done = cmdRegDump();
        }
        else if ( !strcmp( "PLLDUMP", args[0] ) ) {
          done = cmdPllDump();
        }
        else if ( !strcmp( "MSYNDUMP", args[0] ) ) {
          done = cmdSynthDump();
        }
        break;
        
      case 2:
        val = atol( args[1] );
        
        if ( !strcmp( "CHAN", args[0] ) ) {
          done = cmdClkChanSet( val );
        } 
        else if ( !strcmp( "FRQ", args[0] ) ) {
          done = cmdFrqSet( val );
        }
        else if ( !strcmp( "PWR", args[0] ) ) {
          done = cmdPwrSet( val );
        }
        break;
        
      case 3:
        val = atol( args[1] );
        val2 = atol( args[2] );
        if ( !strcmp( "SET", args[0] ) ) {
          done = cmdRegSet( val, val2 );
        } 
        else if ( !strcmp( "XTAL", args[0] ) ) {
          done = cmdXtal( val, val2 );
        } 
       
        break;
                
      default:
        break;
      
    }
    if ( done != 0 ) {
      printError("FAIL: Command not recognised.  Not processed.");
    }
    else {
      Serial.println(F("OK"));
    }
}

//
// take a command from serial input, and send to cmdProcess()
//
void cmdParseAndProcess(void) {
  char *p;
  int j = 0;
  int k;
  
  p = strtok( inputBuffer, " ");
  while ( *p )
  {
    args[j++] = p;
    p = strtok( 0, " ");
  }
  for ( k = 0; k < j; k++ )
  {
    p = args[k];
    while ( *p ) {
      *p++ = toupper( *p );
    }
  }
  if ( j ) {
    cmdProcess( j );
  } 
  resetInput();
}

void loop() {
    char c;

    if ( !devicePresent ) {
      return ;
    }
    
    while ( Serial.available() > 0 ) {
      c = Serial.read();
      if ( c == 13 )    
      {
        inputBuffer[ inputIndex ] = '\0';
        Serial.println(inputBuffer);
        cmdParseAndProcess();
      }
      else
      {
        inputBuffer[ inputIndex++ ] = c;
      }
      if ( inputIndex > MAXINPUT )
      {
        printError("Input overflow.");
      }
    }
}
