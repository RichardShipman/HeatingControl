/*
  User Interface with touch pad and lcd display
  BV4612
  Copyright (c) 2013 Jim Spence.  All right reserved.
  www.byvac.com - see terms and conditions for using hardware
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  Release Notes:
  June 27 2014 PCB BV4612_b, Firmware 1.5  

*/  

#include "Arduino.h"
#include "Print.h"
#include "bv4612_I.h"

// =============================================================================
// SV3B Type serial control
// =============================================================================
// =============================================================================
// *****************************************************************************
// constructor associates i2c addresses
// *****************************************************************************
BV4612::BV4612(int8_t pad)
{
  _PADi2adr=pad;
  Wire.begin(); // join i2c bus
}

// =============================================================================
// Input
// =============================================================================
// =============================================================================

// **************************************************************
// gets a 8 bit value from the i2c bus
// **************************************************************
uint8_t BV4612::i2_8bit(int8_t i2adr)
{
uint8_t rv;
  Wire.requestFrom(i2adr, 1); // returns 1 byte
  rv = Wire.read(); // byte
//  Wire.endTransmission();
  return rv;
}

// **************************************************************
// gets a 16 bit value from the i2c bus
// **************************************************************
uint16_t BV4612::i2_16bit(int8_t i2adr)
{
uint16_t rv;
  Wire.requestFrom(i2adr, 2); // returns 2 bytes
  rv=Wire.read()*256; // high byte
  rv+=Wire.read(); // low byte
//  Wire.endTransmission();
  return rv;
}

// =============================================================================
// =============================================================================
// =============================================================================
// Keypad section
// =============================================================================
// *****************************************************************************
// send a key commands
// *****************************************************************************
void BV4612::sendKeyCmd(uint8_t cmd)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(cmd); // command
    Wire.endTransmission();
    delayMicroseconds(30); // probably not needed
}
// *****************************************************************************
// Clear keypad buffer
// *****************************************************************************
void BV4612::clrBuf()
{
    sendKeyCmd(1); // cl buf
    delayMicroseconds(250);
}

// *****************************************************************************
// Gets number of keys in buffer
// *****************************************************************************
uint8_t BV4612::keysBuf()
{
    sendKeyCmd(2); //
    return (i2_8bit(_PADi2adr));
}

// *****************************************************************************
// Gets a key returns 0 if no keys in buffer
// *****************************************************************************
uint8_t BV4612::key()
{
    sendKeyCmd(3); // command
    return (i2_8bit(_PADi2adr));
}

// *****************************************************************************
// See if a partiular key is in the buffer, returns 0 if not and it it is
// the position of the key within the buffer
// *****************************************************************************
uint8_t BV4612::keyIn(uint8_t k)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(4); // command
    Wire.write(k);
    Wire.endTransmission();
    return (i2_8bit(_PADi2adr));
}

// *****************************************************************************
// returns a key scan code 0 of no key pressed
// *****************************************************************************
uint8_t BV4612::scan()
{
    sendKeyCmd(5); // command
    return (i2_8bit(_PADi2adr));
}

// *****************************************************************************
// returns 8 average values in the buffer provided, values are 16 bits
// *****************************************************************************
void BV4612::avg(uint16_t *b)
{
uint16_t v;
    sendKeyCmd(10); // command
    Wire.requestFrom(_PADi2adr, 16); // returns 16 bytes
    for(uint8_t j=0;j < 16; j++) {
        v=Wire.read()*256; // high byte
        v+=Wire.read(); // low byte
        *(b++)=v;
    }
}

// *****************************************************************************
// returns 8 delta values in the buffer provided, values are 16 bits
// *****************************************************************************
void BV4612::delta(uint16_t *b)
{
uint16_t v;
    sendKeyCmd(11); // command
    Wire.requestFrom(_PADi2adr, 16); // returns 16 bytes
    for(uint8_t j=0;j < 16; j++) {
        v=Wire.read()*256; // high byte
        v+=Wire.read(); // low byte
        *(b++)=v;
    }
}

// *****************************************************************************
// Resets eeprom values
// *****************************************************************************
void BV4612::EEreset()
{
    sendKeyCmd(0xa2); // command
}

// *****************************************************************************
// Sleep mode, the keypad will not function in this mode but the device can 
// be awakend vie a read or write to I2C
// *****************************************************************************
void BV4612::sleep()
{
    sendKeyCmd(21); // command
}

// =============================================================================
// EEPROM settings
// =============================================================================
// *****************************************************************************
// sets new trigger value
// *****************************************************************************
void BV4612::trigger(uint16_t value)
{
    EEwrite(EE_TRIGGERH,(value >> 8) & 0xff);
    EEwrite(EE_TRIGGERL,value & 0xff);
}

// *****************************************************************************
// sets new hysteresis value
// *****************************************************************************
void BV4612::hyst(uint8_t value)
{
    EEwrite(EE_HYST,value);
}

// *****************************************************************************
// sets new keytable pointer
// *****************************************************************************
void BV4612::keyPtr(uint8_t value)
{
    EEwrite(EE_KEYPTR,value);
}

// *****************************************************************************
// sets new key table size
// *****************************************************************************
void BV4612::keySize(uint8_t value)
{
    EEwrite(EE_KEYSIZE,value);
}

// *****************************************************************************
// sets new debounce
// *****************************************************************************
void BV4612::debounce(uint8_t value)
{
    EEwrite(EE_DEBOUNCE,value);
}

// *****************************************************************************
// sets new repeat value
// *****************************************************************************
void BV4612::repeat(uint16_t value)
{
    EEwrite(EE_REPEATH,(value >> 8) & 0xff);
    EEwrite(EE_REPEATL,value & 0xff);
}

// *****************************************************************************
// sets new timebase
// *****************************************************************************
void BV4612::timebase(uint8_t value)
{
    EEwrite(EE_TIMEBASE,value);
}

// *****************************************************************************
// sets new defailt bl
// *****************************************************************************
void BV4612::defaultBL(uint8_t value)
{
    EEwrite(EE_BL,value);
}

// =============================================================================
// SV3 system section
// =============================================================================
// *****************************************************************************
// Writes a byte to the eeprom at a given address
// *****************************************************************************
void BV4612::EEwrite(uint8_t adr, uint8_t value)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(0x91); // command
    Wire.write(adr);
    Wire.write(value);
    Wire.endTransmission();
}

// *****************************************************************************
// Reads a byte from eeprom at given address
// *****************************************************************************
uint8_t BV4612::EEread(uint8_t adr)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(0x90); // command
    Wire.write(adr);
    Wire.endTransmission();
    return (i2_8bit(_PADi2adr));
}

// *****************************************************************************
// Get device ID as a 16 bit number
// *****************************************************************************
uint16_t BV4612::ID()
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(0xa1); // command
    Wire.endTransmission();
    return (i2_16bit(_PADi2adr));
}

// *****************************************************************************
// Get firmware version as two bytes
// *****************************************************************************
void BV4612::Version(uint8_t *b)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(0xa1); // command
    Wire.endTransmission();
    Wire.requestFrom(_PADi2adr, 2); // returns 16 bytes
    *(b++)=Wire.read(); // high byte
    *(b)=Wire.read(); // low byte   
}

// =============================================================================
// =============================================================================
// =============================================================================
// LCD Section
// This is for the ST7032i controller and follows the same commands used 
// for LiquidCrystal; to this end column is specified first i.e 16x2
// =============================================================================
// 
// *****************************************************************************
// Initialise display
// *****************************************************************************
void BV4612::reset()
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(30);
    Wire.endTransmission();
    delay(450);
}

// *****************************************************************************
// sends a command to the display, bus is opened and closed useful for
// single commands
// *****************************************************************************
void BV4612::cmd(uint8_t cmd)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(31);
    Wire.write(cmd);
    Wire.endTransmission();
}

// *****************************************************************************
// sends raw data to the display, bus open at this point
// *****************************************************************************
void BV4612::data(uint8_t data)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(32);
    Wire.write(data);
    Wire.endTransmission();
}

// *****************************************************************************
// LCD string must terminate with 0
// *****************************************************************************
void BV4612::puts(uint8_t *s)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(33); // command
    while(*s) Wire.write(*(s++));
    Wire.write(13); // EOT for device 
    Wire.endTransmission();
}

//*****************************************************************************
// outputs a string of bytes
//*****************************************************************************
void BV4612::dataLine(uint8_t *s, uint8_t row, uint8_t col, uint8_t nBytes)
{
uint8_t j;
    Wire.beginTransmission(_PADi2adr);
    Wire.write(38); // command
    Wire.write(row);
    Wire.write(col);
    Wire.write(nBytes);
    for(j=0;j<nBytes;j++) {
      Wire.write(s[j]);
    }
    Wire.endTransmission();
}

//*****************************************************************************
// LCD image, is in the form pages, bytes per page, data
// row, col is the start of image top left
// arduino is limited to outputting about 20-30 bytes so need to do it in
// bits
//*****************************************************************************
void BV4612::image(uint8_t *s, uint8_t row, uint8_t col)
{
uint8_t jp, pages, bpp, nBits, rem, cRow=row, cCol, cBpp;
    pages = s[0]; // pages
    bpp = s[1]; // bytes per page
    s+=2;
    for(jp=0;jp<pages;jp++) {
      cCol = col;
      cBpp = bpp;
      while(1) {
        if(cBpp <= 0) {
          break;
        }
        if(cBpp > 20) nBits = 20; else nBits = cBpp;
        dataLine(s, cRow+jp, cCol, nBits);
        cCol+=nBits;
        cBpp-=nBits;
        s+=nBits;
      }
    }
}

// *****************************************************************************
// Displays the current sign on message
// *****************************************************************************
void BV4612::startMsg()
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(35); // command
    Wire.endTransmission();
    delay(10);
}

// *****************************************************************************
// sets back light is either on (1) or off (0)
// *****************************************************************************
void BV4612::bl(uint8_t v)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(36); // command
    if(v) Wire.write(1); // byte
    else Wire.write(0); 
    Wire.endTransmission();
}

// *****************************************************************************
// contrast from 0 to 63 
// *****************************************************************************
void BV4612::contrast(uint8_t m)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(37); // command
    Wire.write(m);
    Wire.endTransmission();
}

// *****************************************************************************
// font 1 to 3
// *****************************************************************************
void BV4612::font(uint8_t m)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(40); // command
    if(m > 3) m = 3;
    Wire.write(m);
    Wire.endTransmission();
}

// *****************************************************************************
// home and clear
// *****************************************************************************
void BV4612::clear()
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(41); // command
    Wire.endTransmission();
    delay(30); // extra big delay
}

// *****************************************************************************
// sets column address 0 to 128
// *****************************************************************************
void BV4612::column(uint8_t col)
{
    Wire.beginTransmission(_PADi2adr);
    if(col > 128) col = 128;
    Wire.write(42); // column
    Wire.write(col); // 
    Wire.endTransmission();
}

// *****************************************************************************
// sets page address 0 to 7
// *****************************************************************************
void BV4612::page(uint8_t row)
{
    Wire.beginTransmission(_PADi2adr);
    if(row > 7) row = 7;
    Wire.write(43); // page
    Wire.write(row); // 
    Wire.endTransmission();
}

// *****************************************************************************
// sets initial scrol line
// *****************************************************************************
void BV4612::scroll(uint8_t s)
{
    Wire.beginTransmission(_PADi2adr);
    if(s > 63) s = 63;
    Wire.write(44); // page
    Wire.write(s); // 
    Wire.endTransmission();
}

// *****************************************************************************
// writes a single char using current font
// *****************************************************************************
// void BV4612::putc(uint8_t c)
// {
//     Wire.beginTransmission(_PADi2adr);
//     Wire.write(45); // page
//     Wire.write(c); // 
//     Wire.endTransmission()
// }
// for print inteface
inline size_t BV4612::write(uint8_t data)
{
    Wire.beginTransmission(_PADi2adr);
    Wire.write(45); // page
    Wire.write(data); // 
    Wire.endTransmission();
    return 1;
}

// =============================================================================
// Matches LiquidCrystal Class   
// *****************************************************************************
// *****************************************************************************
// writes a single char using current font
// *****************************************************************************
void BV4612::setCursor(int8_t col, int8_t row)
{
    if(col > 128) col = 128;
    column(col);
    if(row > 7) row = 7;
    page(row);
}

