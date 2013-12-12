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
 * 				2008 Jiri Dobry (jdobry-at-centrum-dot-cz)
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
 * \file       lcd.c
 * \brief      functions to control the HR20 LCD
 * \author     Dario Carluccio <hr20-at-carluccio-dot-de>, Jiri Dobry <jdobry-at-centrum-dot-cz>
 * \date       $Date$
 * $Rev$
 */

// AVR LibC includes
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/version.h>

// redefine variable names for newer processor
#ifdef _AVR_IOM329_H_
#define LCDDR7 LCDDR07
#define LCDDR6 LCDDR06
#define LCDDR5 LCDDR05
#define LCDDR2 LCDDR02
#define LCDDR1 LCDDR01
#define LCDDR0 LCDDR00
#endif

// HR20 Project includes
#include "main.h"
#include "lcd.h"
#include "task.h"
#include "../common/rtc.h"
#include "eeprom.h"

// Local Defines
#define LCD_CONTRAST_MIN       0   //!< \brief contrast minimum
#define LCD_CONTRAST_MAX      15   //!< \brief contrast maxmum
#define LCD_MAX_POS            4   //!< \brief number of 7 segment chars
#define LCD_MAX_CHARS  (sizeof(LCD_CharTablePrgMem))   //!< \brief no. of chars in \ref LCD_CharTablePrgMem

#define LCD_REGISTER_COUNT    20   //!< \brief no. of registers each bitplane

// Vars
volatile uint8_t LCD_used_bitplanes = 1; //!< \brief number of used bitplanes / used for power save

//! segment data for the segment registers in each bitplane
volatile uint8_t LCD_Data[LCD_BITPLANES][LCD_REGISTER_COUNT];

#ifdef LCD_UPSIDE_DOWN
  #define LCD_upside_down 1
#else
  #define LCD_upside_down 0
#endif


#if LANG==LANG_uni
  // Look-up chars table for LCD strings (universal/numbers)
  const uint8_t LCD_StringTable[][4] PROGMEM =
  {
      {' ','1','-','7'},    //!<  " 1-7" 
      {' ','-','1','-'},    //!<  " -1-" MO
      {' ','-','2','-'},    //!<  " -2-" TU
      {' ','-','3','-'},    //!<  " -3-" WE
      {' ','-','4','-'},    //!<  " -4-" TH
      {' ','-','5','-'},    //!<  " -5-" FR
      {' ','-','6','-'},    //!<  " -6-" SA
      {' ','-','7','-'},    //!<  " -7-" SU
      {'B','L','O','C'},    //!<  "b1oc"    LCD_STRING_bloc
      {'-','-','-','-'},    //!<  "----"    LCD_STRING_4xminus
      {' ','-','C','-'},    //!<  " -C-"    LCD_STRING_minusCminus
      {' ','E','R','R'},    //!<  " Err"    LCD_STRING_Err
      {'O','F','F',' '},    //!<  "OFF "    LCD_STRING_OFF
      {'O','N',' ',' '},    //!<  "On  "    LCD_STRING_On
      {'O','P','E','N'},    //!<  "OPEn"    LCD_STRING_OPEn
      {'B','A','T','T'},    //!<  "BAtt"    LCD_STRING_BAtt
      {' ','E','2',' '},    //!<  " E2 "    LCD_STRING_E2
      {' ','E','3',' '},    //!<  " E3 "    LCD_STRING_E3
      {' ','E','4',' '},    //!<  " E4 "    LCD_STRING_E4
      {'E','E','P','R'},    //!<  "EEPr"    LCD_STRING_EEPr
  };
#elif LANG==LANG_de
  // Look-up chars table for LCD strings (german)
  const uint8_t LCD_StringTable[][4] PROGMEM =
  {
      {32, 1,22, 7},    //!<  " 1-7" 
      {33,34,31,32},    //!<  Montag:     'rno '
      {32,13,30,32},    //!<  Dienstag:   ' di '
      {33,34,30,32},    //!<  Mittwoch:   'rni '
      {32,13,31,32},    //!<  Donnerstag: ' do '
      {32,15,28,32},    //!<  Freitag:    ' Fr '
      {32, 5,10,32},    //!<  Samstag:    ' SA '
      {32, 5,31,32},    //!<  Sonntag:    ' So '
      {11, 1,31,36},    //!<  "b1oc"    LCD_STRING_bloc
      {22,22,22,22},    //!<  "----"    LCD_STRING_4xminus
      {32,22,12,22},    //!<  " -C-"    LCD_STRING_minusCminus
      {32,14,28,28},    //!<  " Err"    LCD_STRING_Err
      { 0,15,15,32},    //!<  "OFF "    LCD_STRING_OFF
      { 0,29,32,32},    //!<  "On  "    LCD_STRING_On
      { 0,18,14,29},    //!<  "OPEn"    LCD_STRING_OPEn
      {11,10,38,38},    //!<  "BAtt"    LCD_STRING_BAtt
      {32,14, 2,32},    //!<  " E2 "    LCD_STRING_E2
      {32,14, 3,32},    //!<  " E3 "    LCD_STRING_E3
      {32,14, 4,32},    //!<  " E4 "    LCD_STRING_E4
      {14,14,18,28},    //!<  "EEPr"    LCD_STRING_EEPr
  };
#elif LANG==LANG_cs
  // Look-up chars table for LCD strings (czech)
  const uint8_t LCD_StringTable[][4] PROGMEM =
  {
      {32, 1,22, 7},    //!<  " 1-7" 
      {32,18,31,22},    //!<  " Po "
      {32,37,38,22},    //!<  " Ut "
      {32, 5,38,22},    //!<  " St "
      {32,12,38,22},    //!<  " Ct "
      {32,18,10,22},    //!<  " PA "
      {32, 5,31,22},    //!<  " So "
      {32,29,14,22},    //!<  " nE "
      {11, 1,31,36},    //!<  "b1oc"    LCD_STRING_bloc
      {22,22,22,22},    //!<  "----"    LCD_STRING_4xminus
      {32,22,12,22},    //!<  " -C-"    LCD_STRING_minusCminus
      {32,14,28,28},    //!<  " Err"    LCD_STRING_Err
      { 0,15,15,32},    //!<  "OFF "    LCD_STRING_OFF
      { 0,29,32,32},    //!<  "On  "    LCD_STRING_On
      { 0,18,14,29},    //!<  "OPEn"    LCD_STRING_OPEn
      {11,10,38,38},    //!<  "BAtt"    LCD_STRING_BAtt
      {32,14, 2,32},    //!<  " E2 "    LCD_STRING_E2
      {32,14, 3,32},    //!<  " E3 "    LCD_STRING_E3
      {32,14, 4,32},    //!<  " E4 "    LCD_STRING_E4
      {14,14,18,28},    //!<  "EEPr"    LCD_STRING_EEPr
  };
#endif

/*
 * font 0-H taken from TravelRec., converted to hex
 * 	msb	mlkjih	 ggfedcba lsb
 */
#define FONT_ASCII_OFFSET '-'
static const uint16_t Font[] PROGMEM = {
			0x00C0, // -
			0, // .
			0x21c0, // /  - "°" sign
			0x003F,		//0
			0x0406,		//1
			0x00DB,		//2
			0x008F,		//3
			0x00E6,		//4
			0x00ED,		//5
			0x00FD,		//6
			0x1401,		//7
			0x00FF,		//8
			0x00EF,		//9
			0x1200,		//:
			0x0C00,		// //
			0x2400,		// <
			0x00C1,		//=
			0x0900,		//>
			0x1421,		//?
			0x2D3F,		//@
			0x00F7,		//A
			0x128F,		//B
			0x0039,		//C
			0x120F,		//D
			0x00F9,		//E a f e d g1 g2
			0x00F1,		//F a f e g1 g2
			0x00BD,		//G a f e d c g2
			0x00F6,		//H f e g1 g2 b c
			0x1209,		// I a i l d
			0x001E,		// J b c d e
			0x2470,		// K f e g1 j m
			0x0038,		// L f e d
			0x0536, 	// M f e h j b c
			0x2136,		// N f e h m c b
			0x003F,		// O a f e d b c
			0x00F3,		// P f e a b g1 g2
			0x203F,		// Q a f e d b c m
			0x20F3,		// R a f e b g1 g2 m
			0x018D,		// S a h g2 c d
			0x1201,		// T i l a
			0x003E,		// U f e d b c
			0x0C30,		// V f e k j
			0x2836,		// W f e k m c b
			0x2D00,		// X h m k j
			0x1500,		// Y h j l
			0x0C09		// Z a j k d

};

/*
 * ;		a	b	c	d	e	f	g1	g2	h	i	j	k	l	m
 * taken from TravelRec.
 */
#define SEGMENTS_PER_DIGIT 14
static const uint8_t DigitSegments[] PROGMEM = {
	126,124, 44,  5,  7,127, 47, 85, 87, 86,125, 6, 46,  45,	//left Digit
	123,121,  1,  2,  4, 84, 43, 81, 83, 82,122, 3, 42,  41,	//middle left Digit
	137,139, 59, 18, 16,136, 56, 98, 96, 97,138, 17, 57, 58,	//middle right Digit
	140,142, 22, 21, 19, 99, 60,102,100,101,141, 20, 61, 62		//right Digit
};

static const uint8_t BargraphSegments[] PROGMEM = {
	88, 48,  8,  9, 49, 89, 90, 50,
	10, 11, 51, 91, 92, 52, 12, 13,
	53, 93,134, 94, 54, 14, 15, 55
};

/* Mo - So, LCDDR16 */
static const uint8_t WeekdaySegments[] PROGMEM = {
	128, 129, 130, 131, 132, 133, 95
};


/* enum LCD_SYMBOLS */
/*static const uint8_t SymbolSegments[] PROGMEM = {
	80, 120, 40, 23, 24, 64, 104, 144, 103, 143, 135, 0
};*/


static void LCD_calc_used_bitplanes(uint8_t mode);

/*!
 *******************************************************************************
 *  Init LCD
 *
 *  \note
 *  - Initialize LCD Global Vars
 *  - Set up the LCD (timing, contrast, etc.)
 ******************************************************************************/
void LCD_Init(void)
{
	// Clear segment buffer.
	LCD_AllSegments(LCD_MODE_OFF);

	LCDCRB = (1<<LCDCS)|(0<<LCD2B)|(1<<LCDMUX1)|(1<<LCDMUX0)|(1<<LCDPM2)|(1<<LCDPM1)|(1<<LCDPM0);
	/*
				(1<<LCDCS)                            // Das LCD wird im asynchronen Modus (LCDCS-Bit=1)
																								 mit der Frequenz des Quarzes TOSC1 = 32.768Hz als LCD Clock betrieben.
				|(0<<LCD2B)                           // 1/3 bias is used
				|(1<<LCDMUX1)|(1<<LCDMUX0)            // 1/4 Duty; COM0:3;
				|(1<<LCDPM2)|(1<<LCDPM1)|(1<<LCDPM0); // SEG0:24
	*/

	LCDFRR = (0<<LCDPS2)|(0<<LCDPS1)|(0<<LCDPS0)|(0<<LCDCD2)|(0<<LCDCD1)|(1<<LCDCD0);
	/*
				(0<<LCDPS2)|(0<<LCDPS1)|(0<<LCDPS0)    // N = 16
				|(0<<LCDCD2)|(0<<LCDCD1)|(1<<LCDCD0);  // D = 2
				// ergo f(frame) = 128Hz
				eventuell D=1, N=64 (LCDCD0 = 0) LCDPS0=1 für 64 Hz, ausprobieren
	*/

	LCDCCR = (1<<LCDDC2)|(0<<LCDDC1)|(0<<LCDDC0)|(/*config.lcd_contrast*/ 10 << LCDCC0);
	/*
				(1<<LCDDC2)|(0<<LCDDC1)|(0<<LCDDC0)   // 575 µs
				// 3,1V
				|(config.lcd_contrast << LCDCC0);     // Set the initial LCD contrast level
	*/

	LCDCRA = (1<<LCDEN)|(1<<LCDAB)|(0<<LCDIE)|(0<<LCDBL);
	/*
				(1<<LCDEN)    // Enable LCD
				|(1<<LCDAB)   // Low Power Waveform
				|(0<<LCDIE)   // disable Interrupt
				|(0<<LCDBL);  // No Blanking
	*/

	// Enable LCD start of frame interrupt
	LCDCRA |= (1<<LCDIE);

	LCD_used_bitplanes = 1;
}


/*!
 *******************************************************************************
 *  Switch LCD on/off
 *
 *  \param mode
 *       -      0: clears all digits
 *       -  other: set all digits
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF
 ******************************************************************************/
void LCD_AllSegments(uint8_t mode)
{
	uint8_t i;
	uint8_t val = (mode==LCD_MODE_ON)?0xff:0x00;

	for (i = 0; i < LCD_REGISTER_COUNT * LCD_BITPLANES; i++){
					((uint8_t *)LCD_Data)[i] = val;
	}

	LCD_used_bitplanes=1;
	LCD_Update();
}

/*!
 *******************************************************************************
 *  Print char in LCD field
 *
 *  \note  segments inside one 7 segment array are adressed using address of
 *         segment "F" \ref LCD_FieldOffsetTablePrgMem[] as base address adding
 *         \ref LCD_SegOffsetTablePrgMem[] *
 *
 *  \note  You have to call \ref LCD_Update() to trigger update on LCD if not
 *         it is triggered automatically at change of bitframe
 *
 *  \param value char to print see \ref LCD_CharTablePrgMem[]
 *  \param pos   position in lcd 0=right to 3=left <tt> 32 : 10 </tt>
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 *  \param value
 *        - 0: clears all digits,
 *        - other: set all digits
 ******************************************************************************/
void LCD_PrintChar(uint8_t value, uint8_t pos, uint8_t mode)
{
	uint16_t segments;
	uint8_t segmentOffset;
	uint8_t mask;
	uint8_t i;

	if (pos >= LCD_MAX_POS)
		return;

	if (LCD_upside_down)
		pos = 3 - pos;

	segments = pgm_read_word(&Font[value - FONT_ASCII_OFFSET]);
	segmentOffset = pos * SEGMENTS_PER_DIGIT;
	mask = 1;

	for(i = 0; i < SEGMENTS_PER_DIGIT; ++i)
	{
		LCD_SetSeg(pgm_read_byte(&DigitSegments[segmentOffset + i]), (segments & mask)?mode:LCD_MODE_OFF);
		mask <<= 1;
	}
}

static uint8_t num2ch(uint8_t dec)
{
	if (dec > 16) {
		return ' ';
	} else if (dec > 10) {
		return 'A' + (dec - 10);
	} else {
		return '0' + dec;
	}
}

/*!
 *******************************************************************************
 *  Print Hex value in LCD field
 *
 *  \note  You have to call \ref LCD_Update() to trigger update on LCD if not
 *         it is triggered automatically at change of bitframe
 *
 *  \param value value to be printed (0-0xff)
 *  \param pos   position in lcd 0:left, 1:right
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
void LCD_PrintHex(uint8_t value, uint8_t pos, uint8_t mode)
{
	char ch;
	uint8_t i;
	
	if (pos > 2) {
		return;
	}
	
	for (i = 0; i < 2; i++) {
		ch = num2ch(value & 0x0f);
		LCD_PrintChar(ch, pos + i, mode);
		value >>= 8;
	}
}


/*!
 *******************************************************************************
 *  Print decimal value in LCD field (only 2 digits)
 *
 *  \note You have to call \ref LCD_Update() to trigger update on LCD if not
 *        it is triggered automatically at change of bitframe
 *
 *  \param value value to be printed (0-99)
 *  \param pos   position in lcd 
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
void LCD_PrintDec(uint8_t value, uint8_t pos, uint8_t mode)
{
	char ch;
	uint8_t i;

	// Boundary Check
	if ((pos > 2) || (value >= 100)) {
		return;
	}
	
	for (i = 0; i < 2; i++) {
		ch = num2ch(value % 10);
		LCD_PrintChar(ch, pos + i, mode);
		value /= 10;
	}
}
/*!
 *******************************************************************************
 *  Print decimal value in LCD field (3 digits)
 *
 *  \note You have to call \ref LCD_Update() to trigger update on LCD if not
 *        it is triggered automatically at change of bitframe
 *
 *  \param value value to be printed (0-999)
 *  \param pos   position in lcd
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
void LCD_PrintDec3(uint16_t value, uint8_t pos, uint8_t mode)
{
	if (value>999) {
		value = 999;
	}

	if (pos > 1) {
		return;
	}

	LCD_PrintChar(num2ch(value / 100), pos + 2, mode);
	LCD_PrintDec(value % 100, pos, mode);
}


/*!
 *******************************************************************************
 *  Print decimal uint16 value in LCD field
 *
 *  \note You have to call \ref LCD_Update() to trigger update on LCD if not
 *        it is triggered automatically at change of bitframe
 *
 *  \param value value to be printed (0-9999)
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
void LCD_PrintDecW(uint16_t value, uint8_t mode)
{
	uint8_t tmp;

	// Boundary Check
	if (value > 9999){
		value = 9999;        
	}

	// Print     
	tmp = (uint8_t) (value / 100);
	LCD_PrintDec(tmp, 2, mode);
	tmp = (uint8_t) (value % 100);
	LCD_PrintDec(tmp, 0, mode);
}


/*!
 *******************************************************************************
 *  Print hex uint16 value in LCD field
 *
 *  \note You have to call \ref LCD_Update() to trigger update on LCD if not
 *        it is triggered automatically at change of bitframe
 *
 *  \param value value to be printed (0-0xffff)
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
void LCD_PrintHexW(uint16_t value, uint8_t mode)
{
	uint8_t tmp;

	// Print     
	tmp = (uint8_t) (value >> 8);
	LCD_PrintHex(tmp, 2, mode);
	tmp = (uint8_t) (value & 0xff);
	LCD_PrintHex(tmp, 0, mode);
}


/*!
 *******************************************************************************
 *  Print BYTE as temperature on LCD (desired temperature)
 *
 *  \note  You have to call \ref LCD_Update() to trigger update on LCD if not
 *         it is triggered automatically at change of bitframe
 *
 *  \note  range for desired temperature 5,0°C - 30°C, OFF and ON 
 *
 *  \param temp<BR>
 *     - TEMP_MIN-1          : \c OFF <BR>
 *     - TEMP_MIN to TEMP_MAX : temperature = temp/2  [5,0°C - 30°C]
 *     - TEMP_MAX+1          : \c ON  <BR>
 *     -    other: \c invalid <BR>
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
void LCD_PrintTemp(uint8_t temp, uint8_t mode)
{
    if (temp == TEMP_MIN - 1) {
        // OFF
        LCD_PrintStringID(LCD_STRING_OFF,mode); 
    } else if (temp == TEMP_MAX + 1) {
        // On
        LCD_PrintStringID(LCD_STRING_On,mode); 
    } else if (temp > TEMP_MAX + 1) {
        // Error -E rr
        LCD_PrintStringID(LCD_STRING_Err,mode); 
    } else {
        #define START_POS 1
        LCD_PrintChar(LCD_CHAR_C, 0, mode);// Print C on last segment
        LCD_SetSeg(LCD_SEG_COL1, mode);    // decimal point

        LCD_PrintDec(temp>>1, START_POS + 1, mode);
        LCD_PrintChar(((temp&1)?5:0), START_POS, mode);
        if (temp < (100/5)) {
            LCD_PrintChar(LCD_CHAR_NULL, START_POS + 2, mode);
        }
    }
}


/*!
 *******************************************************************************
 *  Print INT as temperature on LCD (measured temperature)
 *
 *  \note  You have to call \ref LCD_Update() to trigger update on LCD if not
 *         it is triggered automatically at change of bitframe
 *
 *
 *  \param temp temperature in 1/100 deg C<BR>
 *     min:  -999 => -9,9°C
 *     max:  9999 => 99,9°C
  *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
void LCD_PrintTempInt(int16_t temp, uint8_t mode)
{
    bool neg;

    // check min / max
    if (temp < -999) {
        temp = -999;
    }
 
    // negative ?    
    neg = (temp < 0); 
    if (neg){        
        temp = -temp;    
    } 

    #define START_POS 1
    LCD_PrintChar(LCD_CHAR_C, 0, mode);// Print C on last segment
    LCD_SetSeg(LCD_SEG_COL1, mode);    // decimal point

    // 1/100°C not printed
    LCD_PrintDec3(temp/10, START_POS, mode);
    
    if (neg) {
        // negative Temp      
        LCD_PrintChar(LCD_CHAR_neg, START_POS + 2, mode);
    } else if (temp < 1000){
        // Temp < 10°C
        LCD_PrintChar(LCD_CHAR_NULL, START_POS + 2, mode);
    }                             
}

/*!
 *******************************************************************************
 *  Print LCD string from table
 *
 *  \note  something weird due to 7 Segments
 *
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
void LCD_PrintStringID(uint8_t id, uint8_t mode) {
    uint8_t i;
    uint8_t tmp;

    // Put 4 chars
    for (i = 0; i < 4; i++) {
        tmp = pgm_read_byte(&LCD_StringTable[id][i]);
        LCD_PrintChar(tmp, 3-i, mode);
    }
    LCD_SetSeg(LCD_SEG_COL1, LCD_MODE_OFF);
}

void LCD_PrintDayOfWeek(uint8_t dow, uint8_t mode)
{
	LCD_SetSeg(pgm_read_byte(&WeekdaySegments[dow]), mode);
}

/*!
 *******************************************************************************
 *  Set segment of the hour-bar
 *
 *  \note  You have to call \ref LCD_Update() to trigger update on LCD if not
 *         it is triggered automatically at change of bitframe
 *
 *  \param seg No of the hour bar segment 0-23
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
void LCD_SetHourBarSeg(uint8_t seg, uint8_t mode)
{
    // Set segment 
		LCD_SetSeg(pgm_read_byte(&BargraphSegments[seg]), mode);
}

/*!
 *******************************************************************************
 *  Set all segments of the hour-bar (ON/OFF) like bitmap
 *
 *  \note  You have to call \ref LCD_Update() to trigger update on LCD if not
 *         it is triggered automatically at change of bitframe
 *
 *  \param bitmap of hour bar segment 0-23 (bit0 is segment0 etc.)
 *  \note blink is not supported
 ******************************************************************************/
void LCD_HourBarBitmap(uint32_t bitmap)
{
	uint8_t i;
	for(i = 0; i < sizeof(BargraphSegments); ++i)
	{
		LCD_SetSeg(pgm_read_byte(&BargraphSegments[i]), ((uint8_t)bitmap & 1)? LCD_MODE_ON : LCD_MODE_OFF );
		bitmap = bitmap>>1;
	}
}


/*!
 *******************************************************************************
 *  Set all segments from left up to val and clear all other segments
 *
 *  \note  You have to call \ref LCD_Update() to trigger update on LCD if not
 *         it is triggered automatically at change of bitframe
 *
 *  \param seg No of the last hour bar segment to be set 0-23
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
#if 0
void LCD_SetHourBarBar(uint8_t val, uint8_t mode)
{
    uint8_t i;
    // Only Segment 0:23
    if (val > 23){
        val = 23;
    }
    // For each Segment 0:23
    for (i=0; i<24; i++) {
        if (i > val){
            LCD_SetHourBarSeg(i, LCD_MODE_OFF);
        } else {
            LCD_SetHourBarSeg(i, mode);
        } 
    }
}
#endif


/*!
 *******************************************************************************
 *  Set only one segment and clear all others
 *
 *  \note You have to call \ref LCD_Update() to trigger update on LCD if not
 *         it is triggered automatically at change of bitframe
 *
 *  \param seg No of the hour bar segment to be set 0-23
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
#if 0
void LCD_SetHourBarVal(uint8_t val, uint8_t mode)
{
    uint8_t i;
    // Only Segment 0:23
    if (val > 23){
        val = 23;
    }
    // For each Segment 0:23
    for (i=0; i<24; i++) {
        if (i == val){
            LCD_SetHourBarSeg(i, mode);
        } else {
            LCD_SetHourBarSeg(i, LCD_MODE_OFF);
        }
    }
}
#endif


/*!
 *******************************************************************************
 *  Clear LCD Display
 *
 *  \note Sets all Segments of the to \ref LCD_MODE_OFF
 ******************************************************************************/
#if 0
void LCD_ClearAll(void)
{
    LCD_AllSegments(LCD_MODE_OFF);
}
#endif

/*!
 *******************************************************************************
 *  Clear hour-bar 
 *
 *  \note Sets all Hour-Bar Segments to \ref LCD_MODE_OFF
 ******************************************************************************/
#if 0
void LCD_ClearHourBar(void)
{
    LCD_SetHourBarVal(23, LCD_MODE_OFF);
    LCD_Update();
}
#endif

/*!
 *******************************************************************************
 *  Clear all Symbols 
 *
 *  \note  Sets Symbols <tt> AUTO MANU PROG SUN MOON SNOW</tt>
 *         to \ref LCD_MODE_OFF
 ******************************************************************************/
#if 0
void LCD_ClearSymbols(void)
{
    LCD_SetSeg(LCD_SEG_AUTO, LCD_MODE_OFF);
    LCD_SetSeg(LCD_SEG_MANU, LCD_MODE_OFF);
    LCD_SetSeg(LCD_SEG_PROG, LCD_MODE_OFF);
    LCD_SetSeg(LCD_SEG_SUN, LCD_MODE_OFF);
    LCD_SetSeg(LCD_SEG_MOON, LCD_MODE_OFF);
    LCD_SetSeg(LCD_SEG_SNOW, LCD_MODE_OFF);

    LCD_Update();
}
#endif

/*!
 *******************************************************************************
 *  Clear all 7 segment fields
 *
 *  \note  Sets the four 7 Segment and the Columns to \ref LCD_MODE_OFF
 ******************************************************************************/
#if 0
void LCD_ClearNumbers(void)
{
    LCD_PrintChar(LCD_CHAR_NULL, 3, LCD_MODE_OFF);
    LCD_PrintChar(LCD_CHAR_NULL, 2, LCD_MODE_OFF);
    LCD_PrintChar(LCD_CHAR_NULL, 1, LCD_MODE_OFF);
    LCD_PrintChar(LCD_CHAR_NULL, 0, LCD_MODE_OFF);
    LCD_SetSeg(LCD_SEG_COL1, LCD_MODE_OFF);
    LCD_SetSeg(LCD_SEG_COL2, LCD_MODE_OFF);

    LCD_Update();
}
#endif

/*!
 *******************************************************************************
 *  Set segment of LCD
 *
 *  \note  You have to call \ref LCD_Update() to trigger update on LCD if not
 *         it is triggered automatically at change of bitframe
 *
 *  \param seg No of the segment to be set see \ref LCD_SEG_B0 ...
 *  \param mode  \ref LCD_MODE_ON, \ref LCD_MODE_OFF, \ref LCD_MODE_BLINK_1
 ******************************************************************************/
void LCD_SetSeg(uint8_t seg, uint8_t mode)
{
    uint8_t r;
    uint8_t b;

    // Register = segment DIV 8
    r = seg / 8;
    // Bitposition = segment mod 8
    b = 1<<(seg % 8);

	if (r > LCD_REGISTER_COUNT) {
		return;
	}
    // Set bits in each bitplane
	#if LCD_BITPLANES == 2
        if (mode & 1){
            // Set Bit in Bitplane if ON (0b11) or Blinkmode 1 (0b01)
            LCD_Data[0][r] |= b;
        } else {
            // Clear Bit in Bitplane if OFF (0b00) or Blinkmode 2 (0b10)
            LCD_Data[0][r] &= ~b;
        } 
        if (mode & 2){
            // Set Bit in Bitplane if ON (0b11) or Blinkmode 2 (0b10)
            LCD_Data[1][r] |= b;
        } else {
            // Clear Bit in Bitplane if OFF (0b00) or Blinkmode 1 (0b01)
            LCD_Data[1][r] &= ~b;
        } 
    #else
      {
        uint8_t bp;
        for (bp=0; bp<LCD_BITPLANES;  bp++){
            if (mode & (1<<bp)){
                // Set Bit in Bitplane if ON (0b11) or Blinkmode 1 (0b01)
                LCD_Data[bp][r] |= b;
            } else {
                // Clear Bit in Bitplane if OFF (0b00) or Blinkmode 2 (0b10)
                LCD_Data[bp][r] &= ~b;
            }
        }
      }
    #endif
	LCD_calc_used_bitplanes(mode);
}

/*!
 *******************************************************************************
 *  Calculate used bitplanes
 *
 *	\note used only for update LCD, in any other cases intterupt is disabled
 *  \note copy LCD_Data to LCDREG
 *
 ******************************************************************************/
static void LCD_calc_used_bitplanes(uint8_t mode) {
	uint8_t i;

	if ((mode == LCD_MODE_BLINK_1) || (mode == LCD_MODE_BLINK_2)) {
		LCD_used_bitplanes = 2;
		return; // just optimalization
	} 

	// mode must be LCD_MODE_ON or LCD_MODE_OFF
	if (LCD_used_bitplanes == 1) {
		return; // just optimalization, nothing to do
	} 

    for (i = 0; i < LCD_REGISTER_COUNT; i++){
		#if LCD_BITPLANES != 2
			#error optimized for 2 bitplanes // TODO?
		#endif
		if (LCD_Data[0][i] != LCD_Data[1][i]) {
			LCD_used_bitplanes=2;
			return; // it is done
		}
	}
	LCD_used_bitplanes=1;
}


/*!
 *******************************************************************************
 *
 *	LCD_BlinkCounter and LCD_Bitplane for LCD blink
 *
 ******************************************************************************/
static uint8_t LCD_BlinkCounter;   //!< \brief counter for bitplane change
static uint8_t LCD_Bitplane;       //!< \brief currently active bitplane
uint8_t LCD_force_update=0;        //!< \brief force update LCD


/*!
 *******************************************************************************
 *  LCD Interrupt Routine
 *
 *	\note used only for update LCD, in any other cases intterupt is disabled
 *  \note copy LCD_Data to LCDREG
 *
 ******************************************************************************/

void task_lcd_update(void) {
		uint8_t volatile *lcd_regs = &LCDDR0;
		uint8_t i;

    if (++LCD_BlinkCounter > LCD_BLINK_FRAMES){
		#if LCD_BITPLANES == 2
			// optimized version for LCD_BITPLANES == 2
			LCD_Bitplane = (LCD_Bitplane +1) & 1;
		#else
			LCD_Bitplane = (LCD_Bitplane +1) % LCD_BITPLANES;
		#endif
        LCD_BlinkCounter=0;
		LCD_force_update=1;
    }


	if (LCD_force_update) {
		LCD_force_update = 0;
		for (i = 0; i < LCD_REGISTER_COUNT; i++) {
			lcd_regs[i] = LCD_Data[LCD_Bitplane][0];
		}
	}



	if (LCD_used_bitplanes == 1) {
		// only one bitplane used, no blinking
		// Updated; disable LCD start of frame interrupt
		LCDCRA &= ~(1<<LCDIE);
	}
}

/*!
 *******************************************************************************
 *  LCD Interrupt Routine
 *
 *	\note used only for update LCD, in any other cases intterupt is disabled
 *  \note copy LCD_Data to LCDREG
 *
 ******************************************************************************/
#if ! TASK_IS_SFR
// not optimized
ISR(LCD_vect) {
    task |= TASK_LCD;
}
#else
// optimized
ISR_NAKED ISR (LCD_vect) {
    asm volatile(
        // prologue and epilogue is not needed, this code  not touch flags in SREG
        "	sbi %0,%1" "\t\n"
        "	reti" "\t\n"
        ::"I" (_SFR_IO_ADDR(task)) , "I" (TASK_LCD_BIT)
    );
}
#endif 
