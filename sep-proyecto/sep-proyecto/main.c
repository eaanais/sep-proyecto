#define F_CPU 16000000UL //16Mhz

//boton placa
#define BOTON_PINZAS 0
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

volatile uint8_t temp_int = 0;
volatile uint8_t item = 0;
volatile uint8_t dtem = 0;
volatile uint8_t ihum = 0;
volatile uint8_t dhum = 0;
volatile uint8_t ipr = 0;
volatile uint8_t dpr = 0;
volatile uint8_t imin;
volatile uint8_t dmin;
volatile uint8_t imax;
volatile uint8_t dmax;
	
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
	
	temp_int = tmp102Read();
	
	dhtxxconvert( DHTXX_DHT22, &PORTC, &DDRC, &PINC, ( 1 << 3 ) );
	_delay_ms( 1000 );
	dhtxxread( DHTXX_DHT22, &PORTC, &DDRC, &PINC, ( 1 << 3 ), &temp, &humid );


	item = temp / 10;
	dtem = temp % 10;
	float T = item + dtem/10;
		
	if (temp > (imax*10 + dmax)) {
		imax = item;
		dmax = dtem;
		}
	else if (temp < (imin*10 + dmin)){
		imin = item;
		dmin = dtem;
		}
	

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
			Send_A_String("T interior: ");
			setCursor(1,0);
			Send_An_Integer(temp_int);
			Send_A_String("C");
			break;
		}
		case 1:{
			clearScreen();
			setCursor(0,0);
			Send_A_String("T exterior: ");
			setCursor(1,0);
			Send_An_Integer(item);
			Send_A_String(".");
			Send_An_Integer(dtem);
			Send_A_String("C");
			break;
		}
		case 2:{
			clearScreen();
			setCursor(0,0);
			Send_A_String("Humedad: ");
			setCursor(1,0);
			Send_An_Integer(ihum);
			Send_A_String(".");
			Send_An_Integer(dhum);
			Send_A_String("%");
			break;
		}
		case 3:{
			clearScreen();
			setCursor(0,0);
			Send_A_String("Pto. Rocio: ");
			setCursor(1,0);
			Send_An_Integer(ipr);
			Send_A_String(".");
			Send_An_Integer(dpr);
			Send_A_String("C");
			break;
		}
		case 4:{
			clearScreen();
			setCursor(0,0);
			Send_A_String("T max/min: ");
			setCursor(1,0);
			Send_An_Integer(imax);
			Send_A_String(".");
			Send_An_Integer(dmax);
			Send_A_String("C/");
			Send_An_Integer(imin);
			Send_A_String(".");
			Send_An_Integer(dmin);
			Send_A_String("C");
			break;
		}
	}
}

//////////

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
	///////////////////////////////////////
	DDRC &= ~((1<<4)|(1<<5));	// all input; the I2C special pin functions overrule this anyways
	PORTC &= ~((1<<4)|(1<<5));	// all pullups off; the breakout board has its own 1k pullups attached
	///////////////////////////////////////
	
	DDRB &= ~(1 << BOTON_PINZAS_2);
	DDRC &= ~(1 << BOTON_PINZAS);
	
	//activo el pull-up combinando el pin en SALIDA y con PORTB high
	PORTC |= (1 << BOTON_PINZAS);
	PORTB |= (1 << BOTON_PINZAS_2);
	
	
	PCMSK0 |= (1<<PCINT1); //funcion adicional del puerto b pin 7
	PCMSK1 |= (1 << PCINT8);   // set PCINT13 to trigger an interrupt on state change (pin pc5 (SW5 button))
	
	
	PCICR |= (1<<PCIE0); //habilitar la interrupcion
	PCICR |= (1 << PCIE1);     // set PCIE0 to enable PCMSK1 scan (PORTC)

	_delay_ms(200);

	i2cInit();
	TIMER_Init();
    start();
	
	get_data();
	imin = item;
	dmin = dtem;
	imax = item;
	dmax = dtem;
	
	_delay_ms(200);
	sei();

    while (1) 
    {
		get_data();
    }
}

ISR(PCINT0_vect){
	if (button_down_2){ 
		Contador++;
		if (Contador > 4)
		{
			Contador = 0;
		}
		button_down_2 = 0;
	}
	
}

ISR(PCINT1_vect){
	if (button_down){
		 Contador--;
		 if (Contador < 0)
		 {
			 Contador = 4;
		 }
		 button_down = 0;
	}
}

ISR (TIMER2_COMPA_vect)
{
	debounce();
	debounce_2();
}

ISR (TIMER1_COMPA_vect)  // timer0 overflow interrupt
{
	segundos++;
	print_data();
	if (segundos >= 60)
	{
		imin = item;
		dmin = dtem;
		imax = item;
		dmax = dtem;
		minutos++;
		if (minutos >= 60)
		{
			horas++;
			if (horas >= 24)
			{
				/*agregar funciona para resetear el dia*/
				imin = item;
				dmin = dtem;
				imax = item;
				dmax = dtem;
				horas = 0;
			}
			
			minutos = 0;
		}
		
		segundos = 0;
	}
	
}