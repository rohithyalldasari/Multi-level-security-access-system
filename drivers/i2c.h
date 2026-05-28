#ifndef __I2C_H__
#define __I2C_H__

// Low-level LPC2148 I2C physical peripheral functions
void I2c_Init(void);
void I2c_Start(void);
void I2c_Stop(void);
void I2c_Write(unsigned char data);
unsigned char I2c_ReadAck(void);
unsigned char I2c_ReadNoAck(void);

// High-level AT24C256 storage abstractions for password strings
void Eeprom_WriteByte(unsigned short address, unsigned char data);
unsigned char Eeprom_ReadByte(unsigned short address);

#endif
