#ifndef __LCD_H__
#define __LCD_H__

// Function prototypes for the 4-bit LPC2148 LCD driver
void Lcd_Init(void);
void Lcd_Cmd(unsigned char cmd);
void Lcd_Data(unsigned char data);
void Lcd_DisplayString(const char *str);
void Lcd_SetCursor(unsigned char row, unsigned char col);
void Lcd_Clear(void);
void Lcd_ToggleEnable(void);
void Lcd_SendNibble(unsigned char nibble);

#endif
