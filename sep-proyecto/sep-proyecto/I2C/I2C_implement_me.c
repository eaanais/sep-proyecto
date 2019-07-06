
#include <avr/io.h>

#include "I2C_implement_me.h"


// Send a start condition
void i2cInit(void){
	TWBR = 12;									// 400 kHz at 16MHz crystal and no prescaler
	TWSR &= ~((1 << TWPS1) | (1 << TWPS0));		// prescaler 0
	TWDR = 0xFF;								// sending only ones equals an idle high SDA line
	TWCR = (1<<TWEN)|							// Enable TWI-interface and release TWI pins.
	(0<<TWIE)|(0<<TWINT)|				// Disable Interupt.
	(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|	// No Signal requests.
	(0<<TWWC);
}

void i2cSendStart()
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);//bandera de interrupción a 1, start, habilita I2C AVR
	i2cWaitForComplete();       //espera mientras el bit TWINT sea 0
}




// Send a stop condition
void i2cSendStop()
{
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);//bandera de interrupción a 1, detener, habilita I2C AVR
}




// Waits for the previous operation to finish
void i2cWaitForComplete()
{
	while ((TWCR & (1<<TWINT)) == 0);//espera mientras el  bit de interrupcion sea 0
}




// Transmits one byte over the bus to a slave
// (Can be either an address or a value.)
void i2cSendByte(unsigned char data)
{
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);//para empezar a enviar el dato
	i2cWaitForComplete(); //cuando TWINT se ponga a 1 se habrá terminado de enviar el dato
}




// Receives one byte from a slave
// (Always a value; master has no address.)
// Send an ACK if you expect more data, NAK if not.
void i2cReceiveByte(bool sendAnACK)
{
	if (sendAnACK) TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	else TWCR = (1<<TWINT)|(1<<TWEN);
	i2cWaitForComplete();
}




// After receiving a byte, fetch this byte from the TWDR register
// (if you are interested in it).
// It is possible to *not* call this function after a call to
// i2cReceiveByte() if you are not interested in the actual value.
unsigned char i2cGetReceivedByte(void)
{
	return TWDR;
}