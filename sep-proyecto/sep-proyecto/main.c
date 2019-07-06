#define F_CPU 16000000UL //16Mhz

//boton placa
#define BOTON_PINZAS 5
#define BOTON_PINZAS_2 1

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "TIMER/TIMER.h"
#include "lcd.h"

volatile int Contador = 0;
volatile int Habilitador = 0;

volatile uint8_t button_down;
volatile uint8_t button_down_2;

uint8_t segundos = 0;
uint8_t minutos = 0;
uint8_t horas = 0;

uint16_t sumador = 0;
uint16_t sumador_display = 0;
int refresco = 1;
int reinicio = 0;

////////////
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "I2C/I2C_implement_me.h"
#include "TMP102/TMP102.h"
#include "DHT22/dhtxx.h"

volatile uint8_t temp_ext = 0;
volatile uint8_t item = 0;
volatile uint8_t dtem = 0;
volatile uint8_t ihum = 0;
volatile uint8_t dhum = 0;
volatile uint8_t ipr = 0;
volatile uint8_t dpr = 0;
	
char buffer[10];

void f1(void);
void f2(void);

float dew_point(float T, float H){
	float b = 18.678;
	float c = 257.14;
	float gamma = log(H/100) + b*T/(c + T);
	float pr = c*gamma/(b - gamma);
	return pr;
}

void get_data(void){
	int temp, humid;
	
	temp_ext = tmp102Read();
	
	dhtxxconvert( DHTXX_DHT22, &PORTC, &DDRC, &PINC, ( 1 << 3 ) );

	_delay_ms( 1000 );

	//Read data from sensor to variables `temp` and `humid` (`ec` is exit code)
	dhtxxread( DHTXX_DHT22, &PORTC, &DDRC, &PINC, ( 1 << 3 ), &temp, &humid );

	item = temp / 10;
	dtem = temp % 10;
	float T = item + dtem/10;
	
	ihum = humid / 10;
	dhum = humid % 10;
	float H = ihum + dhum/10;
	
		
	float pr = dew_point(T, H);
	ipr = pr;
	dpr = pr * 10 - ipr * 10;
}

void print_data(void){
	switch (Contador){
		case 0:{
			clearScreen();
			setCursor(0,0);
			Send_A_String("Temperatura interior: ");
			itoa(temp_ext, buffer, 10);
			Send_A_String(buffer);
			Send_A_String("C");
			break;
		}
		case 1:{
			clearScreen();
			setCursor(0,0);
			Send_A_String("Temperatura exterior: ");
			itoa(item, buffer, 10);
			Send_A_String(buffer);
			Send_A_String(".");
			itoa(dtem, buffer, 10);
			Send_A_String(buffer);
			Send_A_String("C");
			break;
		}
		case 2:{
			clearScreen();
			setCursor(0,0);
			Send_A_String("Temperatura exterior: ");
			itoa(ihum, buffer, 10);
			Send_A_String(buffer);
			Send_A_String(".");
			itoa(dhum, buffer, 10);
			Send_A_String(buffer);
			Send_A_String("C");
			break;
		}
		case 3:{
			clearScreen();
			setCursor(0,0);
			Send_A_String("Temperatura exterior: ");
			itoa(ipr, buffer, 10);
			Send_A_String(buffer);
			Send_A_String(".");
			itoa(dpr, buffer, 10);
			Send_A_String(buffer);
			Send_A_String("C");
			break;
		}
		case 4:{
			clearScreen();
			setCursor(0,0);
			Send_A_String("Temperatura maxima/minima: ");
			break;
		}
	}
	setCursor(1,0);
	Send_An_Integer(segundos);
}
/////////////

void debounce(void){
	static uint8_t count = 0;
	static uint8_t button_state = 1;
	uint8_t current_state = (~PINC & (1<<BOTON_PINZAS)) != 0;
	if (current_state != button_state) {
		// Button state is about to be changed, increase counter
		count++;
		if (count >= 4) {
			// The button have not bounced for four checks, change state
			//button_state = current_state;
			// If the button was pressed (not released), tell main so
			if (current_state != 1) {
				button_down = 1;
			}
			count = 0;
		}
	} else {
	// Reset counter
		count = 0;
	}
}

void debounce_2(void){
	static uint8_t count_2 = 0;
	static uint8_t button_state_2 = 1;
	uint8_t current_state_2 = (~PINB & (1<<BOTON_PINZAS_2)) != 0;
	if (current_state_2 != button_state_2) {
		// Button state is about to be changed, increase counter
		count_2++;
		if (count_2 >= 4) {
			// The button have not bounced for four checks, change state
			//button_state = current_state;
			// If the button was pressed (not released), tell main so
			if (current_state_2 != 1) {
				button_down_2 = 1;
			}
			count_2 = 0;
		}
		} else {
		// Reset counter
		count_2 = 0;
	}
}


int main(void)
{
	DDRB &= ~(1 << BOTON_PINZAS_2);
	DDRC &= ~(1 << BOTON_PINZAS);
	
	//activo el pull-up combinando el pin en SALIDA y con PORTB high
	PORTC |= (1 << BOTON_PINZAS);
	PORTB |= (1 << BOTON_PINZAS_2);
	
	
	PCMSK0 |= (1<<PCINT1); //funcion adicional del puerto b pin 7
	PCMSK1 |= (1 << PCINT13);   // set PCINT13 to trigger an interrupt on state change (pin pc5 (SW5 button))
	
	
	PCICR |= (1<<PCIE0); //habilitar la interrupcion
	PCICR |= (1 << PCIE1);     // set PCIE0 to enable PCMSK1 scan (PORTC)

	i2cInit();

    start();
	
	sei(); //habilita las interrupciones globales
	

    /* Replace with your application code */
    while (1) 
    {
		//debounce();
		if (Contador == 0)
		{
			cli();
			setCursor(1,0);
			Send_An_Integer(segundos);
			sei();

		}
		else if (Contador == 1)
		{
			cli();
			setCursor(1,0);
			Send_An_Integer(segundos);
			sei();
		}
		else if (Contador == 2)
		{
			cli();
			setCursor(1,0);
			Send_An_Integer(segundos);
			sei();
		}
		else if (Contador == 3)
		{
			cli();
			setCursor(1,0);
			Send_An_Integer(segundos);
			sei();
		}
		else
		{
			cli();
			setCursor(1,0);
			Send_An_Integer(segundos);
			sei();
		}
		
		_delay_ms(100);
    }
}

ISR(PCINT0_vect){
	if (button_down_2) Contador++;
}

ISR(PCINT1_vect){
	if (button_down) Contador--;
}

ISR (TIMER2_COMPA_vect)
{
	debounce();
	debounce_2();
}

ISR (TIMER0_COMPA_vect)
{
	/*timer disponible*/
	/*if (sumador <=999)
	{
		int a = (int)(sumador/100);
		int b = (int)(sumador/10-(int)(sumador/100)*10);
		int c = (int)(sumador-(int)(sumador/100)*100-((int)(sumador/10)-(int)(sumador/100)*10)*10);

		switch (refresco)
		{
			case 1:
			SIETE_SEG(a,refresco);
			refresco++;
			break;
			case 2:
			SIETE_SEG(b,refresco);
			refresco++;
			break;
			case 3:
			SIETE_SEG(c,refresco);
			refresco++;
			break;
			case 4:
			SIETE_SEG(10,3);
			refresco=1;
			break;
		}// action to be done every 250 usec
	}
	else if (sumador<=9999)
	{
		sumador_display = sumador/10;
		int a = (int)(sumador_display/100);
		int b = (int)(sumador_display/10-(int)(sumador_display/100)*10);
		int c = (int)(sumador_display-(int)(sumador_display/100)*100-((int)(sumador_display/10)-(int)(sumador_display/100)*10)*10);
		
		
		switch (refresco)
		{
			case 1:
			SIETE_SEG(a,refresco);
			refresco++;
			break;
			case 2:
			SIETE_SEG(10,1);
			refresco++;
			break;
			case 3:
			SIETE_SEG(b,refresco-1);
			refresco++;
			break;
			case 4:
			SIETE_SEG(c,refresco-1);
			refresco=1;
			break;
		}// action to be do
	}
	else 
	{
		switch (refresco)
		{
			case 1:
			SIETE_SEG(11,refresco);
			refresco++;
			break;
			case 2:
			SIETE_SEG(12,refresco);
			refresco++;
			break;
			case 3:
			SIETE_SEG(12,refresco);
			refresco=1;
			break;
		}// action to
	}
	*/
}

ISR (TIMER1_COMPA_vect)  // timer0 overflow interrupt
{
	segundos++;
	if (segundos >= 60)
	{
		print_data();
		minutos++;
		if (minutos >= 60)
		{
			horas++;
			if (horas >= 24)
			{
				/*agregar funciona para resetear el dia*/
				horas = 0;
			}
			
			minutos = 0;
		}
		
		segundos = 0;
	}
	
}