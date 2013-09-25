/*******************************************************************************
 *
 *  HAL_Dogs102x6.c - Driver for the DOGS 102x6 display
 *
 *  Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/* Adapted to TinkerPal */

#include "msp430.h"
#include "platform/msp430/dogs102x6.h"

// Store a copy of the lcd memory (8x102) = 816 bytes
// Since we cannot read from the lcd memory, this is a way to keep track of
// what is stored there Two additional byes are used for driver-
// internal purposes
uint8_t dogs102x6Memory[816 + 2];

// Macros
#ifndef abs
#    define abs(n)     (((n) < 0) ? -(n) : (n))
#endif

// For all commands, CD signal must = 0
#define SET_COLUMN_ADDRESS_MSB        0x10  //Set SRAM col. addr. before write, last 4 bits =
                                            // ca4-ca7
#define SET_COLUMN_ADDRESS_LSB        0x00  //Set SRAM col. addr. before write, last 4 bits =
                                            // ca0-ca3
#define SET_POWER_CONTROL             0x2F  //Set Power control - booster, regulator, and follower
                                            // on
#define SET_SCROLL_LINE               0x40  //Scroll image up by SL rows (SL = last 5 bits),
                                            // range:0-63
#define SET_PAGE_ADDRESS              0xB0  //Set SRAM page addr (pa = last 4 bits), range:0-8
#define SET_VLCD_RESISTOR_RATIO       0x27  //Set internal resistor ratio Rb/Ra to adjust contrast
#define SET_ELECTRONIC_VOLUME_MSB     0x81  //Set Electronic Volume "PM" to adjust contrast
#define SET_ELECTRONIC_VOLUME_LSB     0x0F  //Set Electronic Volume "PM" to adjust contrast (PM =
                                            // last 5 bits)
#define SET_ALL_PIXEL_ON              0xA4  //Disable all pixel on (last bit 1 to turn on all pixels
                                            // - does not affect memory)
#define SET_INVERSE_DISPLAY           0xA6  //Inverse display off (last bit 1 to invert display -
                                            // does not affect memory)
#define SET_DISPLAY_ENABLE            0xAF  //Enable display (exit sleep mode & restore power)
#define SET_SEG_DIRECTION             0xA1  //Mirror SEG (column) mapping (set bit0 to mirror
                                            // display)
#define SET_COM_DIRECTION             0xC8  //Mirror COM (row) mapping (set bit3 to mirror display)
#define SYSTEM_RESET                  0xE2  //Reset the system. Control regs reset, memory not
                                            // affected
#define NOP                           0xE3  //No operation
#define SET_LCD_BIAS_RATIO            0xA2  //Set voltage bias ratio (BR = bit0)
#define SET_CURSOR_UPDATE_MODE        0xE0  //Column address will increment with write operation
                                            // (but no wrap around)
#define RESET_CURSOR_UPDATE_MODE      0xEE  //Return cursor to column address from before cursor
                                            // update mode was set
#define SET_ADV_PROGRAM_CONTROL0_MSB  0xFA  //Set temp. compensation curve to -0.11%/C
#define SET_ADV_PROGRAM_CONTROL0_LSB  0x90

// Pins from MSP430 connected to LCD
#define CD              BIT6
#define CS              BIT4
#define RST             BIT7
#define BACKLT          BIT6
#define SPI_SIMO        BIT1
#define SPI_CLK         BIT3

// Ports
#define CD_RST_DIR      P5DIR
#define CD_RST_OUT      P5OUT
#define CS_BACKLT_DIR   P7DIR
#define CS_BACKLT_OUT   P7OUT
#define CS_BACKLT_SEL   P7SEL
#define SPI_SEL         P4SEL
#define SPI_DIR         P4DIR

// Variables

uint8_t currentPage = 0, currentColumn = 0;

uint8_t backlight  = 8;
uint8_t contrast = 0x0F;
uint8_t drawmode = DOGS102x6_DRAW_IMMEDIATE;

// Dog102-6 Initialization Commands
uint8_t Dogs102x6_initMacro[] = {
    SET_SCROLL_LINE,
    SET_SEG_DIRECTION,
    SET_COM_DIRECTION,
    SET_ALL_PIXEL_ON,
    SET_INVERSE_DISPLAY,
    SET_LCD_BIAS_RATIO,
    SET_POWER_CONTROL,
    SET_VLCD_RESISTOR_RATIO,
    SET_ELECTRONIC_VOLUME_MSB,
    SET_ELECTRONIC_VOLUME_LSB,
    SET_ADV_PROGRAM_CONTROL0_MSB,
    SET_ADV_PROGRAM_CONTROL0_LSB,
    SET_DISPLAY_ENABLE,
    SET_PAGE_ADDRESS,
    SET_COLUMN_ADDRESS_MSB,
    SET_COLUMN_ADDRESS_LSB
};

/***************************************************************************//**
 * @brief   Initialize LCD
 * @param   None
 * @return  None
 ******************************************************************************/

void Dogs102x6_init(void)
{
    // Port initialization for LCD operation
    CD_RST_DIR |= RST;
    // Reset is active low
    CD_RST_OUT &= RST;
    // Reset is active low
    CD_RST_OUT |= RST;
    // Chip select for LCD
    CS_BACKLT_DIR |= CS;
    // CS is active low
    CS_BACKLT_OUT &= ~CS;
    // Command/Data for LCD
    CD_RST_DIR |= CD;
    // CD Low for command
    CD_RST_OUT &= ~CD;

    // P4.1 option select SIMO
    SPI_SEL |= SPI_SIMO;
    SPI_DIR |= SPI_SIMO;
    // P4.3 option select CLK
    SPI_SEL |= SPI_CLK;
    SPI_DIR |= SPI_CLK;

    // Initialize USCI_B1 for SPI Master operation
    // Put state machine in reset
    UCB1CTL1 |= UCSWRST;
    //3-pin, 8-bit SPI master
    UCB1CTL0 = UCCKPH + UCMSB + UCMST + UCMODE_0 + UCSYNC;
    // Clock phase - data captured first edge, change second edge
    // MSB
    // Use SMCLK, keep RESET
    UCB1CTL1 = UCSSEL_2 + UCSWRST;
    UCB1BR0 = 0x02;
    UCB1BR1 = 0;
    // Release USCI state machine
    UCB1CTL1 &= ~UCSWRST;
    UCB1IFG &= ~UCRXIFG;

    Dogs102x6_writeCommand(Dogs102x6_initMacro, 13);

    // Deselect chip
    CS_BACKLT_OUT |= CS;

    dogs102x6Memory[0] = 102;
    dogs102x6Memory[1] = 8;
}

/***************************************************************************//**
 * @brief   Initialize Backlight
 * @param   None
 * @return  None
 ******************************************************************************/

void Dogs102x6_backlightInit(void)
{
    // Turn on Backlight
    CS_BACKLT_DIR |= BACKLT;
    CS_BACKLT_OUT |= BACKLT;
    // Uses PWM to control brightness
    CS_BACKLT_SEL |= BACKLT;

    // start at full brightness (8)
    TB0CCTL4 = OUTMOD_7;
    TB0CCR4 = TB0CCR0 >> 1;

    TB0CCR0 = 50;
    TB0CTL = TBSSEL_1 + MC_1;
}

/***************************************************************************//**
 * @brief  Set function for the backlight PWM's duty cycle
 *
 * @param  BackLightLevel The target backlight duty cycle - valued 0~11.
 *
 * @return none
 ******************************************************************************/

void Dogs102x6_setBacklight(uint8_t brightness)
{
    unsigned int dutyCycle = 0, i, dummy;

    if (brightness > 0)
    {
        TB0CCTL4 = OUTMOD_7;
        dummy = (TB0CCR0 >> 4);

        dutyCycle = 12;
        for (i = 0; i < brightness; i++)
            dutyCycle += dummy;

        TB0CCR4 = dutyCycle;

        //If the backlight was previously turned off, turn it on.
        if (!backlight)
            TB0CTL |= MC0;
    }
    else
    {
        TB0CCTL4 = 0;
        TB0CTL &= ~MC0;
    }
    backlight = brightness;
}

/***************************************************************************//**
 * @brief   Disable display
 * @param   None
 * @return  None
 ******************************************************************************/

void Dogs102x6_disable(void)
{
    uint8_t cmd[1] = { SYSTEM_RESET };

    Dogs102x6_writeCommand(cmd, 1);
    cmd[0] = SET_DISPLAY_ENABLE & 0xFE;
    Dogs102x6_writeCommand(cmd, 1);
}

/***************************************************************************//**
 * @brief   Sends commands to LCD via 3 wire SPI
 * @param   sCmd Pointer to the commands to be written to the LCD
 * @param   i Number of commands to be written to the LCD
 * @return  None
 ******************************************************************************/

void Dogs102x6_writeCommand(uint8_t *sCmd, uint8_t i)
{
    // Store current GIE state
    uint16_t gie = __get_SR_register() & GIE;

    // Make this operation atomic
    __disable_interrupt();

    // CS Low
    P7OUT &= ~CS;

    // CD Low
    P5OUT &= ~CD;
    while (i)
    {
        // USCI_B1 TX buffer ready?
        while (!(UCB1IFG & UCTXIFG)) ;

        // Transmit data
        UCB1TXBUF = *sCmd;

        // Increment the pointer on the array
        sCmd++;

        // Decrement the Byte counter
        i--;
    }

    // Wait for all TX/RX to finish
    while (UCB1STAT & UCBUSY) ;

    // Dummy read to empty RX buffer and clear any overrun conditions
    UCB1RXBUF;

    // CS High
    P7OUT |= CS;

    // Restore original GIE state
    __bis_SR_register(gie);
}

/***************************************************************************//**
 * @brief   Sends Data to LCD via 3 wire SPI
 * @param   sData Pointer to the Data to be written to the LCD
 * @param   i Number of data bytes to be written to the LCD
 * @return  None
 ******************************************************************************/

void Dogs102x6_writeData(uint8_t *sData, uint8_t i)
{
    // Store current GIE state
    uint16_t gie = __get_SR_register() & GIE;

    // Make this operation atomic
    __disable_interrupt();

    if (drawmode == DOGS102x6_DRAW_ON_REFRESH) 
    {
      while (i)
      {
          dogs102x6Memory[2 + (currentPage * 102) + currentColumn] = (uint8_t)*sData++;
          currentColumn++;
  
          // Boundary check
          if (currentColumn > 101)
          {
              currentColumn = 101;
          }
          
          // Decrement the Byte counter
          i--;
      }
    } 
    else 
    {
    
      // CS Low
      P7OUT &= ~CS;
      //CD High
      P5OUT |= CD;
  
      while (i)
      {
          dogs102x6Memory[2 + (currentPage * 102) + currentColumn] = (uint8_t)*sData;
          currentColumn++;
  
          // Boundary check
          if (currentColumn > 101)
          {
              currentColumn = 101;
          }
  
          // USCI_B1 TX buffer ready?
          while (!(UCB1IFG & UCTXIFG)) ;
  
          // Transmit data and increment pointer
          UCB1TXBUF = *sData++;
  
          // Decrement the Byte counter
          i--;
      }
  
      // Wait for all TX/RX to finish
      while (UCB1STAT & UCBUSY) ;
  
      // Dummy read to empty RX buffer and clear any overrun conditions
      UCB1RXBUF;
  
      // CS High
      P7OUT |= CS;
    }
    
    // Restore original GIE state
    __bis_SR_register(gie);
}

/***************************************************************************//**
 * @brief   Gets the current contrast level
 * @param   None
 * @return  Contrast level
 ******************************************************************************/

uint8_t Dogs102x6_getContrast(void)
{
    return contrast;
}

/***************************************************************************//**
 * @brief   Gets the current backlight level
 * @param   None
 * @return  Backlight level
 ******************************************************************************/

uint8_t Dogs102x6_getBacklight(void)
{
    return backlight;
}

/***************************************************************************//**
 * @brief   Sets Address of the LCD RAM memory
 *
 *          (0,0) is the upper left corner of screen.
 * @param   pa Page Address of the LCD RAM memory to be written (0 - 7)
 * @param   ca Column Address of the LCD RAM memory to be written (0 - 101)
 * @return  None
 ******************************************************************************/

void Dogs102x6_setAddress(uint8_t pa, uint8_t ca)
{
    uint8_t cmd[1];
    
    // Page boundary check
    if (pa > 7)
    {
        pa = 7;
    }

    // Column boundary check
    if (ca > 101)
    {
        ca = 101;
    }

    // Page Address Command = Page Address Initial Command + Page Address
    cmd[0] = SET_PAGE_ADDRESS + (7 - pa);
    uint8_t H = 0x00;
    uint8_t L = 0x00;
    uint8_t ColumnAddress[] = { SET_COLUMN_ADDRESS_MSB, SET_COLUMN_ADDRESS_LSB };

    currentPage = pa;
    currentColumn = ca;

    if (drawmode == DOGS102x6_DRAW_ON_REFRESH) return; // exit if drawmode on refresh

    // Separate Command Address to low and high
    L = (ca & 0x0F);
    H = (ca & 0xF0);
    H = (H >> 4);
    // Column Address CommandLSB = Column Address Initial Command
    //                             + Column Address bits 0..3
    ColumnAddress[0] = SET_COLUMN_ADDRESS_LSB + L;
    // Column Address CommandMSB = Column Address Initial Command
    //                             + Column Address bits 4..7
    ColumnAddress[1] = SET_COLUMN_ADDRESS_MSB + H;

    // Set page address
    Dogs102x6_writeCommand(cmd, 1);
    // Set column address
    Dogs102x6_writeCommand(ColumnAddress, 2);
}

/***************************************************************************//**
 * @brief   Sets the contrast
 * @param   contrast Contrast level (0~31, where 31 is darkest setting)
 * @return  None
 ******************************************************************************/

void Dogs102x6_setContrast(uint8_t newContrast)
{
    uint8_t cmd[2];

    cmd[0] = SET_ELECTRONIC_VOLUME_MSB;

    //check if parameter is in range
    if (newContrast > 0x1F)
        cmd[1] = 0x1F;
    else
        cmd[1] = newContrast;

    contrast = cmd[1];

    Dogs102x6_writeCommand(cmd, 2);

    // Save new contrast to initMacro
    Dogs102x6_initMacro[8] = newContrast;
}

/***************************************************************************//**
 * @brief   Inverts the screen (pixels on/off) - does not affect LCD memory.
 * @param   None
 * @return  None
 ******************************************************************************/

void Dogs102x6_setInverseDisplay(void)
{
    uint8_t cmd[] = {SET_INVERSE_DISPLAY + 0x01};

    Dogs102x6_writeCommand(cmd, 1);
}

/***************************************************************************//**
 * @brief   Uninverts the screen (pixels on/off) - does not affect LCD memory.
 * @param   None
 * @return  None
 ******************************************************************************/

void Dogs102x6_clearInverseDisplay(void)
{
    uint8_t cmd[] = {SET_INVERSE_DISPLAY};

    Dogs102x6_writeCommand(cmd, 1);
}

/***************************************************************************//**
 * @brief   Scrolls image down a number of lines. The scrolling wraps around the screen.
 * @param   lines number of lines to scroll (0~63)
 * @return  None
 ******************************************************************************/

void Dogs102x6_scrollLine(uint8_t lines)
{
    uint8_t cmd[] = {SET_SCROLL_LINE};

    //check if parameter is in range
    if (lines > 0x1F)
    {
        cmd[0] |= 0x1F;
    }
    else
    {
        cmd[0] |= lines;
    }

    Dogs102x6_writeCommand(cmd, 1);
}

/***************************************************************************//**
 * @brief   Sets all Pixels on
 * @param   None
 * @return  None
 ******************************************************************************/

void Dogs102x6_setAllPixelsOn(void)
{
    uint8_t cmd[] = {SET_ALL_PIXEL_ON + 0x01};

    Dogs102x6_writeCommand(cmd, 1);
}

/***************************************************************************//**
 * @brief   Returns pixels to normal functioning
 * @param   None
 * @return  None
 ******************************************************************************/

void Dogs102x6_clearAllPixelsOn(void)
{
    uint8_t cmd[] = {SET_ALL_PIXEL_ON};

    Dogs102x6_writeCommand(cmd, 1);
}

/****************************DRAWING FUNCTIONS*********************************/

/***************************************************************************//**
 * @brief   Sets All pixels Off (in memory as well)
 * @param   None
 * @return  None
 ******************************************************************************/

void Dogs102x6_clearScreen(void)
{
    uint8_t LcdData[] = {0x00};
    uint8_t p, c;

    // 8 total pages in LCD controller memory
    for (p = 0; p < 8; p++)
    {
        Dogs102x6_setAddress(p, 0);
        // 102 total columns in LCD controller memory
        for (c = 0; c < 102; c++)
        {
            Dogs102x6_writeData(LcdData, 1);
        }
    }
}

/***************************************************************************//**
 * @brief  Draws a pixel at (x,y).
 *
 *          (0,0) is the upper left corner of screen.
 * @param  x x-coordinate of the point
 * @param  y y-coordinate of the point
 * @param   style The style of the pixel
 *                - NORMAL = 0 = dark pixel
 *                - INVERT = 1 = inverts pixel
 * @return None
 ******************************************************************************/

void Dogs102x6_pixelDraw(uint8_t x, uint8_t y, uint8_t style)
{
    uint8_t p, temp;

    //make sure we won't be writing off the screen
    if (x > 101)
    {
        x = 101;
    }

    if (y > 63)
    {
        y = 63;
    }

    //determine the page
    p = y >> 3;                        // identical to: p = y / 8;

    //determine height of pixel within the page
    temp = 0x80 >> (y & 0x07);         // identical to: temp = 0x80 >> (y % 8);

    //update our array
    if (style == DOGS102x6_DRAW_NORMAL)
        dogs102x6Memory[2 + (p * 102) + x] |= temp;
    else
        dogs102x6Memory[2 + (p * 102) + x] &= ~temp;

    Dogs102x6_setAddress(p, x);

    //draw pixel
    Dogs102x6_writeData(dogs102x6Memory + (2 + (p * 102) + x), 1);
}
