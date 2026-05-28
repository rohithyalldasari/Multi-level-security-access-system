#include <lpc214x.h>
#include "i2c.h"
#include "delay.h"

void I2c_Init(void) {
    // Configure P0.2 as SCL and P0.3 as SDA
    PINSEL0 &= ~0x000000F0;
    PINSEL0 |=  0x00000050;
    
    // Clear configuration flags
    I2CONCLR = 0x6C; 
    
    // Set Bit Rate for 100kHz standard I2C clock @ PCLK = 60MHz
    I2SCLH = 300;
    I2SCLL = 300;
    
    // Enable the I2C peripheral interface
    I2CONSET = 0x40; 
}

void I2c_Start(void) {
    I2CONSET = 0x20; // Set STA (Start bit)
    while (!(I2CONSET & 0x08)); // Wait for SI flag to set
}

void I2c_Stop(void) {
    I2CONSET = 0x10; // Set STO (Stop bit)
    I2CONCLR = 0x08; // Clear SI flag
}

void I2c_Write(unsigned char data) {
    I2DAT = data;
    I2CONCLR = 0x28; // Clear SI flag and STO bit
    while (!(I2CONSET & 0x08)); // Wait for SI flag to complete transfer
}

unsigned char I2c_ReadAck(void) {
    I2CONSET = 0x04; // Enable AA (Acknowledge)
    I2CONCLR = 0x08; // Clear SI flag
    while (!(I2CONSET & 0x08)); // Wait for data frame
    return I2DAT;
}

unsigned char I2c_ReadNoAck(void) {
    I2CONCLR = 0x0C; // Clear AA and SI flags (NACK)
    while (!(I2CONSET & 0x08)); // Wait for data frame
    return I2DAT;
}

// EEPROM Byte Write Logic (Targeting AT24C256 device)
void Eeprom_WriteByte(unsigned short address, unsigned char data) {
    I2c_Start();
    I2c_Write(0xA0); // Control byte + Write bit
    I2c_Write((address >> 8) & 0xFF); // High address byte
    I2c_Write(address & 0xFF);        // Low address byte
    I2c_Write(data);
    I2c_Stop();
    Delay_ms(10); // Hardware write cycle time delay
}

// EEPROM Byte Read Logic
unsigned char Eeprom_ReadByte(unsigned short address) {
    unsigned char data = 0;
    I2c_Start();
    I2c_Write(0xA0); // Control byte + Write bit (Dummy write)
    I2c_Write((address >> 8) & 0xFF); // High address byte
    I2c_Write(address & 0xFF);        // Low address byte
    
    I2c_Start(); // Repeated Start condition
    I2c_Write(0xA1); // Control byte + Read bit
    data = I2c_ReadNoAck();
    I2c_Stop();
    return data;
}
