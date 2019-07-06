#define F_CPU	16000000UL

// read and write addresses; set ADD0 = GND
#define TMP102_READ	 	0x91
#define TMP102_WRITE 	0x90

#include <avr/io.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../I2C/I2C_implement_me.h"
#include "TMP102.h"

int16_t tmp102Read(void)
{
	
	uint8_t msb, lsb;
	int16_t temp;
	
		
	i2cSendStart();							// send start condition
	i2cWaitForComplete();
	
	i2cSendByte(TMP102_WRITE); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cSendByte(0x00); 						// set TMP102 pointer register to 0 (read temperature)
	i2cWaitForComplete();
	
	i2cSendStart();							// send repeated start condition
	i2cWaitForComplete();
	
	i2cSendByte(TMP102_READ);				// send READ address of TMP102
	i2cWaitForComplete();
	i2cReceiveByte(true);					// receives one byte from the bus and ACKs it
	i2cWaitForComplete();
	msb = i2cGetReceivedByte(); 			// reads the MSB (it is a 12 bit value!)
	i2cWaitForComplete();
	i2cReceiveByte(false);					// receives one byte from the bus and NAKs it (last one)
	i2cWaitForComplete();
	lsb = i2cGetReceivedByte();				// reads the LSB
	i2cWaitForComplete();
	
	i2cSendStop();							// send stop condition
	TWCR = 0;								// stop everything
	
	
	// Convert the number to an 8-bit degree Celsius value
	temp = (msb<<8) | lsb;					// combine those two values into one 16 bit value
	temp >>= 4; 							// the values are left justified; fix that by shifting 4 right
	// negative numbers are represented in two's complement, but we just shifted the value and thereby potentially messed it up
	if(temp & (1<<11))						// Hence: if it is a negative number
		temp |= 0xF800;						// restore the uppermost bits
		
	// The 12 bit value has 0.0625°C precision, which is too much for what we want (and the sensor is anyways only about 0.5°C precise)
	// 0.0625 is 1/16 -> Dividing by 16 leaves 1°C precision for the output. Note that shifting >> 4 might go wrong here for negative numbers.
	temp /= 16;
	
	return(temp);
}
