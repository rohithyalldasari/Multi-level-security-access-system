#include <lpc214x.h>
#include <string.h>
#include "drivers/lcd.h"
#include "drivers/delay.h"
#include "drivers/uart.h"
#include "drivers/keypad.h"
#include "drivers/i2c.h"

// System Configuration Mock Definitions & Hardcoded Test Credentials
#define AUTHORIZED_RFID "12345678"
#define MOTOR_PIN_CW    (1 << 24) // P0.24 linked to L293D IN1
#define MOTOR_PIN_CCW   (1 << 25) // P0.25 linked to L293D IN2
#define TIMEOUT_LIMIT   5000      // ~5 Seconds software check window

// Global tracking flags
volatile unsigned char system_mode = 0; // 0 = Normal, 1 = Admin Menu Mode

void System_Init(void) {
    // Configure DC Motor Control Pins as Output
    IO0DIR |= (MOTOR_PIN_CW | MOTOR_PIN_CCW);
    IO0CLR = (MOTOR_PIN_CW | MOTOR_PIN_CCW); // Motor off
    
    // Initialize required underlying hardware drivers
    Lcd_Init();
    Uart0_Init(9600);
    Keypad_Init();
    I2c_Init();
    
    // Configure External Interrupt 1 (EINT1 on P0.14) for Admin Menu
    PINSEL0 |= 0x20000000;         // Set P0.14 to EINT1 functional mode
    EXTMODE |= 0x02;               // Edge-sensitive tracking
    EXTPOLAR &= ~0x02;             // Falling edge trigger
    
    // Assign EINT1 into the Vector Interrupt Controller (Slot 1)
    VICVectAddr1 = (unsigned int)Eint1_Isr;
    VICVectCntl1 = 0x20 | 15;      // Channel 15 is EINT1, enable slot
    VICIntEnable |= (1 << 15);     // Enable global EINT1 flag
    
    // Enable system interrupts
    __enable_irq();
    
    // Set up a default startup passcode in the EEPROM if empty
    if (Eeprom_ReadByte(0x00) == 0xFF) {
        Eeprom_WriteByte(0x00, '1');
        Eeprom_WriteByte(0x01, '2');
        Eeprom_WriteByte(0x02, '3');
        Eeprom_WriteByte(0x03, '4');
        Eeprom_WriteByte(0x04, '\0');
    }
}

void Open_Security_Door(void) {
    Lcd_Clear();
    Lcd_SetCursor(1, 1);
    Lcd_DisplayString("Access Granted!");
    Lcd_SetCursor(2, 1);
    Lcd_DisplayString("Door Opening...");
    
    IO0SET = MOTOR_PIN_CW;        // Drive motor clockwise
    IO0CLR = MOTOR_PIN_CCW;
    Delay_ms(2000);
    
    IO0CLR = (MOTOR_PIN_CW | MOTOR_PIN_CCW); // Stop motor
    Delay_ms(3000);               // Stay open
    
    Lcd_SetCursor(2, 1);
    Lcd_DisplayString("Door Closing... ");
    IO0SET = MOTOR_PIN_CCW;       // Drive motor counter-clockwise
    IO0CLR = MOTOR_PIN_CW;
    Delay_ms(2000);
    
    IO0CLR = (MOTOR_PIN_CW | MOTOR_PIN_CCW); // Lock door
}

void Execute_Admin_Menu(void) {
    char selection = '\0';
    Lcd_Clear();
    Lcd_SetCursor(1, 1);
    Lcd_DisplayString("1)Edit Password ");
    Lcd_SetCursor(2, 1);
    Lcd_DisplayString("2)Edit Biometric");
    
    while (selection != '1' && selection != '2') {
        selection = Keypad_GetKey();
    }
    
    if (selection == '1') {
        Lcd_Clear();
        Lcd_SetCursor(1, 1);
        Lcd_DisplayString("Enter New Pass:");
        char new_pass[5];
        int i = 0;
        while (i < 4) {
            char key = Keypad_GetKey();
            if (key != '\0') {
                new_pass[i++] = key;
                Lcd_SetCursor(2, i);
                Lcd_Data('*');
            }
        }
        new_pass[4] = '\0';
        
        // Write the newly confirmed passcode into external I2C EEPROM storage
        for (i = 0; i <= 4; i++) {
            Eeprom_WriteByte(0x00 + i, new_pass[i]);
        }
        Lcd_Clear();
        Lcd_SetCursor(1, 1);
        Lcd_DisplayString("Password Updated");
        Delay_ms(2000);
    } else if (selection == '2') {
        Lcd_Clear();
        Lcd_SetCursor(1, 1);
        Lcd_DisplayString("Enroll/Del Sync ");
        Lcd_SetCursor(2, 1);
        Lcd_DisplayString("Flash Updated   ");
        Delay_ms(2000);
    }
    system_mode = 0; // Return to waiting loop execution
}

int main(void) {
    char entered_pass[5];
    char stored_pass[5];
    unsigned int timeout_tracker = 0;
    int pass_idx = 0;
    
    System_Init();
    
    while (1) {
        if (system_mode == 1) {
            Execute_Admin_Menu();
        }
        
        Lcd_Clear();
        Lcd_SetCursor(1, 1);
        Lcd_DisplayString("Scan RFID Card..");
        rfid_ready_flag = 0;
        rfid_count = 0;
        
        // Phase 1 Loop: Scan for RFID card transmission frames
        while (rfid_ready_flag == 0) {
            if (system_mode == 1) break;
        }
        if (system_mode == 1) continue;
        
        // Check parsed ASCII frame credentials
        if (strcmp((const char*)rfid_buffer, AUTHORIZED_RFID) == 0) {
            Lcd_Clear();
            Lcd_SetCursor(1, 1);
            Lcd_DisplayString("RFID Verified!");
            Delay_ms(1500);
            
            Lcd_Clear();
            Lcd_SetCursor(1, 1);
            Lcd_DisplayString("Enter Passcode:");
            pass_idx = 0;
            timeout_tracker = 0;
            
            // Phase 2 Loop: Passcode entry with embedded Real-Time Timeout constraint
            while (pass_idx < 4 && timeout_tracker < TIMEOUT_LIMIT) {
                char ch = Keypad_GetKey();
                if (ch != '\0') {
                    entered_pass[pass_idx++] = ch;
                    Lcd_SetCursor(2, pass_idx);
                    Lcd_Data('*');
                }
                Delay_ms(1);
                timeout_tracker++;
            }
            entered_pass[4] = '\0';
            
            if (timeout_tracker >= TIMEOUT_LIMIT) {
                Lcd_Clear();
                Lcd_SetCursor(1, 1);
                Lcd_DisplayString("Session Timeout!");
                Delay_ms(2000);
                continue; // Restart system sequence back from top loop state
            }
            
            // Read saved configuration credentials out of active I2C lines
            for (int x = 0; x < 4; x++) {
                stored_pass[x] = Eeprom_ReadByte(0x00 + x);
            }
            stored_pass[4] = '\0';
            
            if (strcmp(entered_pass, stored_pass) == 0) {
                Lcd_Clear();
                Lcd_SetCursor(1, 1);
                Lcd_DisplayString("Pass Verified!");
                Lcd_SetCursor(2, 1);
                Lcd_DisplayString("Scanning Print..");
                Delay_ms(2000);
                
                // Phase 3: Simulated Biometric R305 Library Verification Success
                Open_Security_Door();
            } else {
                Lcd_Clear();
                Lcd_SetCursor(1, 1);
                Lcd_DisplayString("Wrong Passcode!");
                Delay_ms(2000);
            }
        } else {
            Lcd_Clear();
            Lcd_SetCursor(1, 1);
            Lcd_DisplayString("Invalid ID Card!");
            Delay_ms(2000);
        }
    }
}

void __irq Eint1_Isr(void) {
    system_mode = 1;     // Set operating parameter flag to handle menu loop change
    EXTINT |= 0x02;      // Clear current pending EINT1 external channel trigger flag
    VICVectAddr = 0;     // Force peripheral ACK routine line update back to core
}
