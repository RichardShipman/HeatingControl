/*
  User Interface with touch pad and lcd display
  BV4242
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

*/  

// See cpp for release notes

#ifndef _H_bv4612_I_h
#define _H_bv4612_I_h

// EEPROM constants
#define EE_TRIGGERH 18
#define EE_TRIGGERL EE_TRIGGERH+1
#define EE_HYST 20
#define EE_KEYPTR 21
#define EE_KEYSIZE EE_KEYPTR+1
#define EE_DEBOUNCE 23
#define EE_REPEATH 24
#define EE_REPEATL EE_REPEATH+1
#define EE_TIMEBASE 26
#define EE_BL 27


#include "Arduino.h"
#include "Wire.h"
#include "Print.h"

// macro *** Bit # from 0 to 7
#define bitset(var,bitno) ((var) |= 1UL << (bitno))
#define bitclr(var,bitno) ((var) &= ~(1UL << (bitno)))

class BV4612  : public Print
{
    public:
        // non-device specific
        BV4612(int8_t pad);
        // keypad section
        void clrBuf();
        uint8_t keysBuf();
        uint8_t key();
        uint8_t keyIn(uint8_t k);
        uint8_t scan();
        void avg(uint16_t *b);
        void delta(uint16_t *b);
        void EEreset();
        void sleep();
        // EEPROM settings
        void trigger(uint16_t value);
        void hyst(uint8_t value);
        void keyPtr(uint8_t value);
        void keySize(uint8_t value);
        void debounce(uint8_t value);
        void repeat(uint16_t value);
        void timebase(uint8_t value);  
        void defaultBL(uint8_t value);      
        // system section
        void EEwrite(uint8_t adr, uint8_t value);
        uint8_t EEread(uint8_t adr);
        uint16_t ID();
        void Version(uint8_t *b);
        // lcd section
        void reset();
        void cmd(uint8_t cmd);
        void data(uint8_t);
        void puts(uint8_t *s);
        virtual size_t write(uint8_t); // for print interface
        void dataLine(uint8_t *s,uint8_t row,uint8_t col,uint8_t nBytes);
        void image(uint8_t *s, uint8_t row, uint8_t col);
        void startMsg();
        void bl(uint8_t v);
        void contrast(uint8_t m);
        void font(uint8_t m);
        void clear();
        void column(uint8_t col);
        void page(uint8_t row);
        void scroll(uint8_t s);
        void setCursor(int8_t col, int8_t row);
    private:
        int8_t _PADi2adr;
        void sendKeyCmd(uint8_t cmd);
        uint8_t i2_8bit(int8_t i2adr);
        uint16_t i2_16bit(int8_t i2adr);
};


#endif