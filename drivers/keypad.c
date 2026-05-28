#include <lpc214x.h>
#include "keypad.h"
#include "delay.h"

// Row pins defined as Outputs (P0.16 - P0.19)
// Column pins defined as Inputs with internal pull-ups (P0.20 - P0.23)
#define ROW_MASK (0xF << 16)
#define COL_MASK (0xF << 20)

const unsigned char keypad_matrix[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void Keypad_Init(void) {
    // Configure row and column pins as GPIO
    PINSEL1 &= ~0x000FF000; 
    
    // Set Rows as Output, Columns as Input
    IO0DIR |= ROW_MASK;
    IO0DIR &= ~COL_MASK;
    
    // Set Rows High initially
    IO0SET = ROW_MASK;
}

char Keypad_GetKey(void) {
    int row, col;
    
    for (row = 0; row < 4; row++) {
        // Clear all rows, then ground one specific row at a time
        IO0SET = ROW_MASK;
        IO0CLR = (1 << (16 + row));
        
        // Read columns to check if a key is pressed (Active Low)
        for (col = 0; col < 4; col++) {
            if (!(IO0PIN & (1 << (20 + col)))) {
                // Key debounce delay
                Delay_ms(20);
                while (!(IO0PIN & (1 << (20 + col)))); // Wait for key release
                
                return keypad_matrix[row][col];
            }
        }
    }
    return '\0'; // Return null character if no key is pressed
}
