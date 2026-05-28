#ifndef __UART_H__
#define __UART_H__

// Shared volatile variables for background RFID frame parsing
extern volatile unsigned char rfid_buffer[12];
extern volatile unsigned int rfid_count;
extern volatile unsigned char rfid_ready_flag;

// Function prototypes for UART0 peripheral management
void Uart0_Init(unsigned int baudrate);
void Uart0_TxChar(char ch);
void Uart0_TxString(const char *str);
void __irq Uart0_Isr(void);

#endif
