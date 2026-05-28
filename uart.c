#include <lpc214x.h>
#include "uart.h"

// Volatile variables to handle background ISR parsing for EM-18
volatile unsigned char rfid_buffer[12];
volatile unsigned int rfid_count = 0;
volatile unsigned char rfid_ready_flag = 0;

void Uart0_Init(unsigned int baudrate) {
    // Calibrated for PCLK = 60MHz (VPBDIV = 1)
    unsigned int dl_value = 60000000 / (16 * baudrate);
    
    PINSEL0 |= 0x00000005; // Enable TXD0 (P0.0) and RXD0 (P0.1)
    
    U0LCR = 0x83;          // 8-bit data, 1 stop bit, Enable DLAB
    U0DLL = dl_value & 0xFF;
    U0DLM = (dl_value >> 8) & 0xFF;
    U0LCR = 0x03;          // Disable DLAB, lock configurations
    
    U0IER = 0x01;          // Enable Receive Data Available Interrupt
    
    // Configure VIC for UART0 (Slot 0)
    VICVectAddr0 = (unsigned int)Uart0_Isr;
    VICVectCntl0 = 0x20 | 6; // Channel 6 is UART0, enable slot
    VICIntEnable |= (1 << 6);
}

void Uart0_TxChar(char ch) {
    while (!(U0LSR & 0x20)); // Wait until THR is empty
    U0THR = ch;
}

void Uart0_TxString(const char *str) {
    while (*str) {
        Uart0_TxChar(*str++);
    }
}

void __irq Uart0_Isr(void) {
    unsigned char ch;
    unsigned int iir = U0IIR; // Read IIR to acknowledge interrupt
    
    if ((iir & 0x0E) == 0x04) { // RDA (Receive Data Available) triggered
        ch = U0RBR;
        
        // Frame parsing structure for EM-18 RFID (0x02 to 0x03)
        if (ch == 0x02) {
            rfid_count = 0; // Start of frame detected
            rfid_ready_flag = 0;
        } else if (ch == 0x03) {
            rfid_buffer[rfid_count] = '\0'; // Null terminate string payload
            rfid_ready_flag = 1;            // Full ID captured successfully
        } else if (rfid_count < 10) {
            rfid_buffer[rfid_count++] = ch; // Append ASCII byte
        }
    }
    VICVectAddr = 0; // Acknowledge Vector Interrupt Controller
}
