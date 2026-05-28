#include <lpc214x.h>
#include "lcd.h"
#include "delay.h"

#define LCD_DATA_PINS  (0xF << 16) // Using P0.16 to P0.19 as D4-D7 (4-bit mode)
#define LCD_RS         (1 << 20)  // P0.20
#define LCD_EN         (1 << 21)  // P0.21

void Lcd_ToggleEnable(void) {
    IO0SET = LCD_EN;
    Delay_us(10);
    IO0CLR = LCD_EN;
    Delay_us(10);
}

void Lcd_SendNibble(unsigned char nibble) {
    IO0CLR = LCD_DATA_PINS; // Clear data lines
    IO0SET = ((nibble & 0x0F) << 16); // Shift 4-bit data to P0.16-P0.19
    Lcd_ToggleEnable();
}

void Lcd_Cmd(unsigned char cmd) {
    IO0CLR = LCD_RS; // RS = 0 for Command
    Lcd_SendNibble(cmd >> 4);   // Higher nibble
    Lcd_SendNibble(cmd & 0x0F); // Lower nibble
    Delay_ms(2);
}

void Lcd_Data(unsigned char data) {
    IO0SET = LCD_RS; // RS = 1 for Data
    Lcd_SendNibble(data >> 4);   // Higher nibble
    Lcd_SendNibble(data & 0x0F); // Lower nibble
    Delay_us(50);
}

void Lcd_Init(void) {
    PINSEL1 &= ~0x00000FF0; // Configure P0.16 to P0.21 as GPIO
    IO0DIR |= (LCD_DATA_PINS | LCD_RS | LCD_EN); // Set pins as Output
    
    IO0CLR = (LCD_RS | LCD_EN | LCD_DATA_PINS);

    Delay_ms(20); // Wait for LCD power-up
    
    // Initialize into 4-bit mode
    Lcd_SendNibble(0x03);
    Delay_ms(5);
    Lcd_SendNibble(0x03);
    Delay_us(200);
    Lcd_SendNibble(0x03);
    Lcd_SendNibble(0x02); // Set to 4-bit interface

    Lcd_Cmd(0x28); // 2 lines, 5x7 matrix in 4-bit mode
    Lcd_Cmd(0x0C); // Display ON, Cursor OFF
    Lcd_Cmd(0x06); // Auto-increment cursor
    Lcd_Cmd(0x01); // Clear Display
    Delay_ms(2);
}

void Lcd_DisplayString(const char *str) {
    while (*str) {
        Lcd_Data(*str++);
    }
}

void Lcd_SetCursor(unsigned char row, unsigned char col) {
    unsigned char address = 0;
    if (row == 1) {
        address = 0x80 + (col - 1);
    } else if (row == 2) {
        address = 0xC0 + (col - 1);
    }
    Lcd_Cmd(address);
}

void Lcd_Clear(void) {
    Lcd_Cmd(0x01);
    Delay_ms(2);
}
