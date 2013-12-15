/*
 *  Open HR20
 *
 *  target:     ATmega169 @ 4 MHz in Honnywell Rondostat HR20E
 *
 *  compiler:    WinAVR-20071221
 *              avr-libc 1.6.0
 *              GCC 4.2.2
 *
 *  copyright:  2008 Dario Carluccio (hr20-at-carluccio-dot-de)
 *
 *  license:    This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU Library General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later version.
 *
 *              This program is distributed in the hope that it will be useful,
 *              but WITHOUT ANY WARRANTY; without even the implied warranty of
 *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *              GNU General Public License for more details.
 *
 *              You should have received a copy of the GNU General Public License
 *              along with this program. If not, see http:*www.gnu.org/licenses
 */

/*!
 * \file       lcd.h  
 * \brief      header file for lcd.c, functions to control the HR20 LCD
 * \author     Dario Carluccio <hr20-at-carluccio-dot-de>
 * \date       $Date$
 * $Rev$
 */

#pragma once

/*****************************************************************************
*   Macros
*****************************************************************************/

// Modes for LCD_SetSymbol
#define LCD_MODE_OFF        0    //!< (0b00) segment off
#define LCD_MODE_BLINK_1    1    //!< (0b01) segment on during 1. frame (blinking)
#define LCD_MODE_BLINK_2    2    //!< (0b10) segment on during 2. frame (blinking)
#define LCD_MODE_ON         3    //!< (0b11) segment permanent on

#define LCD_CONTRAST_INITIAL  14 //!< initial LCD contrast (0-15)
#define LCD_BLINK_FRAMES      12 //!< refreshes for each frame @ 48 frames/s
                                 //!< 12 refreshes -> 4Hz Blink frequency
#define LCD_BITPLANES          2   //!< \brief two bitplanes for blinking

/*****************************************************************************
*   Global Vars
*****************************************************************************/
extern volatile uint8_t LCD_used_bitplanes; //!< \brief number of used bitplanes / used for power
extern uint8_t LCD_force_update;        //!< \brief force update LCD

/*****************************************************************************
*   Prototypes
*****************************************************************************/

void LCD_Init(void);                       // Init the LCD Controller
void LCD_AllSegments(uint8_t);             // Set all segments to LCD_MODE
void LCD_ClearAll(void);                   // Clear all segments
void LCD_ClearHourBar(void);               // Clear 24 bar segments
void LCD_ClearSymbols(void);               // Clear AUTO MANU PROG SUN MOON SNOW
void LCD_ClearNumbers(void);               // Clear 7 Segments and Collumns

void LCD_PrintDec(uint8_t, uint8_t, uint8_t);  // Print DEC-val (0-99)
void LCD_PrintDec3(uint16_t value, uint8_t pos, uint8_t mode); // Print DEC-val (0-255)
void LCD_PrintDecW(uint16_t, uint8_t);         // Print DEC-val (0-9999)                       
void LCD_PrintHex(uint8_t, uint8_t, uint8_t);  // Print HEX-val (0-ff)
void LCD_PrintHexW(uint16_t, uint8_t);         // Print HEX-val (0-ffff) 
void LCD_PrintChar(uint8_t, uint8_t, uint8_t); // Print one digit 
void LCD_PrintTemp(uint8_t, uint8_t);          // Print temperature (val+4,9)°C
void LCD_PrintTempInt(int16_t , uint8_t);      // Print temperature (val/100)°C
void LCD_PrintDayOfWeek(uint8_t, uint8_t);     // Print Day of Week (german)
void LCD_PrintStringID(uint8_t id, uint8_t mode); // Print LCD string ID

void LCD_SetSeg(uint8_t, uint8_t);         // Set one Segment (0-69) 
void LCD_SetHourBarSeg(uint8_t, uint8_t);  // Set HBS (0-23) (Hour-Bar-Segment)
void LCD_HourBarBitmap(uint32_t bitmap);   // Set HBS like bitmap
void task_lcd_update(void);

#define  LCD_Update()  ((LCDCRA |= (1<<LCDIE)),(LCD_force_update=1))
	// Update at next LCD_ISR
    // Enable LCD start of frame interrupt
    


//***************************
// LCD Chars:
//***************************
#define LCD_CHAR_0       '0'  //!< char "0"
#define LCD_CHAR_1       '1'  //!< char "1"
#define LCD_CHAR_2       '2'  //!< char "2"
#define LCD_CHAR_3       '3'  //!< char "3"
#define LCD_CHAR_4       '4'  //!< char "4"
#define LCD_CHAR_5       '5'  //!< char "5"
#define LCD_CHAR_6       '6'  //!< char "6"

#define LCD_CHAR_7       '7'  //!< char "7"
#define LCD_CHAR_8       '8'  //!< char "8"
#define LCD_CHAR_9       '9'  //!< char "9"
#define LCD_CHAR_A       'A'  //!< char "A"
#define LCD_CHAR_b       'B'  //!< char "b"
#define LCD_CHAR_C       'C'  //!< char "C"
#define LCD_CHAR_d       'D'  //!< char "d"

#define LCD_CHAR_E       'E'  //!< char "E"
#define LCD_CHAR_F       'F'  //!< char "F"
#define LCD_CHAR_deg     '/'  //!< symbol degree
#define LCD_CHAR_n       'N'  //!< char "n"
#define LCD_CHAR_P       'P'  //!< char "P"
#define LCD_CHAR_H       'H'  //!< char "H"
#define LCD_CHAR_I       'I'  //!< char "I"
#define LCD_CHAR_neg     '-'  //!< char "-"
#define LCD_CHAR_2lines    //!< line on top, line on bottom
#define LCD_CHAR_3lines    //!< 3 horizontal lines
#define LCD_CHAR_r       'R'  //!< char "r"
#define LCD_CHAR_o       'O'  //!< char "r"
#define LCD_CHAR_L       'L'  //!< char "L"
#define LCD_CHAR_c       'C'  //!< char "c"
#define LCD_CHAR_U       'U'  //!< char "U"
#define LCD_CHAR_t       'T'  //!< char "t"
#define LCD_CHAR_y       'Y'  //!< char "y"
#define LCD_CHAR_S       'S'  //!< char "5" = "S"

#define LCD_CHAR_NULL    '.'  //!< space


#define LCD_SEG_PROG       0xff

#define LCD_SEG_AUTO       80
#define LCD_SEG_MANU       120
#define LCD_PADLOCK        143

#define LCD_SEG_COL1       63
#define LCD_SEG_COL2       135

#define LCD_SEG_RADIO      23
#define LCD_SEG_BAT        103

#define LCD_SEG_SNOW       144
#define LCD_SEG_MOON       104
#define LCD_SEG_INDOOR     24
#define LCD_SEG_OUTDOOR    64
#define LCD_SEG_BAG        40

#define LCD_SEG_BAR24      0



//***************************
// LCD Strings:
//***************************

#define LCD_STRING_bloc         8
#define LCD_STRING_4xminus      9
#define LCD_STRING_minusCminus 10
#define LCD_STRING_Err         11
#define LCD_STRING_OFF         12
#define LCD_STRING_On          13
#define LCD_STRING_OPEn        14
#define LCD_STRING_BAtt        15
#define LCD_STRING_E2          16
#define LCD_STRING_E3          17
#define LCD_STRING_E4          18
#define LCD_STRING_EEPr        19

