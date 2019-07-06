/*
 * display_contador.c
 *
 * Created: 12-05-2019 22:26:54
 * Author : Lenovo
 */ 


#define F_CPU 16000000UL //16Mhz






#define rojo 3
#define verde 2
#define azul 1

//boton placa
#define BOTON_PINZAS 5
#define BOTON_PINZAS_2 1


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
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





void SIETE_SEG(int digito, int pos);


ISR(PCINT0_vect) //rutina de la interrupcion
{
	if (button_down_2)
	{
		
		Contador++;
		if (Contador==5)
		{
			Contador=0;
		}
		else
		{
			Contador=Contador;
		}
		
		button_down_2 = 0;
		
		if (Contador == 0)
		{
			clearScreen();
			setCursor(0,0);
			Send_A_String("Temp. Externa");
		}
		else if (Contador == 1)
		{
			clearScreen();
			setCursor(0,0);
			Send_A_String("Temp. Interna");
		}
		else if (Contador == 2)
		{
			clearScreen();
			Send_A_String("Humedad");
		}
		else if (Contador == 3)
		{
			clearScreen();
			Send_A_String("Pto. Rocio");
		}
		else
		{
			clearScreen();
			Send_A_String("Max. y Min.");
		}
	}
	
	else
	{
		Contador=Contador;
	}
}

ISR(PCINT1_vect)
{
	if (button_down)
	{
		
		
		Contador--;
		if (Contador==-1)
		{
			Contador=4;
		}
		else
		{
			Contador=Contador;
		}
		
	button_down = 0;	
	
	
	if (Contador == 0)
	{
		clearScreen();
		Send_A_String("Temp. Externa");
	}
	else if (Contador == 1)
	{
		clearScreen();
		Send_A_String("Temp. Interna");
	}
	else if (Contador == 2)
	{
		clearScreen();
		Send_A_String("Humedad");
	}
	else if (Contador == 3)
	{
		clearScreen();
		Send_A_String("Pto. Rocio");
	}
	else
	{
		clearScreen();
		Send_A_String("Max. y Min.");
	}
	}

	else
	{
		Contador=Contador;
	}
	
}

int main(void)
{
	DDRC |= (1 << rojo);
	DDRC |= (1 << verde);
	DDRC |= (1 << azul);
	
	
	PORTC |= (1 << rojo);
	PORTC |= (1 << verde);
	PORTC |= (1 << azul);
	
	DDRB &= ~(1 << BOTON_PINZAS_2);
	DDRC &= ~(1 << BOTON_PINZAS);
	
	//activo el pull-up combinando el pin en SALIDA y con PORTB high
	PORTC |= (1 << BOTON_PINZAS);
	PORTB |= (1 << BOTON_PINZAS_2);
	
	
	PCMSK0 |= (1<<PCINT1); //funcion adicional del puerto b pin 7
	PCMSK1 |= (1 << PCINT13);   // set PCINT13 to trigger an interrupt on state change (pin pc5 (SW5 button))
	
	
	PCICR |= (1<<PCIE0); //habilitar la interrupcion
	PCICR |= (1 << PCIE1);     // set PCIE0 to enable PCMSK1 scan (PORTC)
	
	OCR1A = 62500; //1000HZ62500
	TCCR1B |= (1 << WGM12);
	// Mode 4, CTC on OCR1A
	TIMSK1 |= (1 << OCIE1A);
	//Set interrupt on compare match
	TCCR1B |= (1 << CS12);
	// set prescaler to 1024 and start the timer

	
	OCR2A = 156; // 156
	TCCR2A |= (1 << WGM21);
	// Set to CTC Mode
	TIMSK2 |= (1 << OCIE2A);
	//Set interrupt on compare match
	TCCR2B |= (1 << CS12) | (1 << CS10);
	// set prescaler to 64 and starts PWM
	
	// Set the Timer Mode to CTC
	TCCR0A |= (1 << WGM01);
	// Set the value that you want to count to
	OCR0A = 62;
	TIMSK0 |= (1 << OCIE0A);    //Set the ISR COMPA vect
	TCCR0B |= (1 << CS02);
	// set prescaler to 256 and start the timer

    start();
	
	sei(); //habilita las interrupciones globales
	


    /* Replace with your application code */
    while (1) 
    {
		//debounce();
		if (Contador == 0)
		{
			PORTC &= ~(1 << rojo);
			PORTC |=(1 << verde);
			PORTC |=(1 << azul);
			cli();
			setCursor(1,0);
			Send_An_Integer(segundos);
			sei();

		}
		else if (Contador == 1)
		{
			PORTC |= (1 << rojo);
			PORTC &= ~(1 << verde);
			PORTC |= (1 << azul);
			cli();
			setCursor(1,0);
			Send_An_Integer(segundos);
			sei();
		}
		else if (Contador == 2)
		{
			PORTC |= (1 << rojo);
			PORTC &= ~(1 << verde);
			PORTC |= (1 << azul);
			cli();
			setCursor(1,0);
			Send_An_Integer(segundos);
			sei();
		}
		else if (Contador == 3)
		{
			PORTC |= (1 << rojo);
			PORTC &= ~(1 << verde);
			PORTC |= (1 << azul);
			cli();
			setCursor(1,0);
			Send_An_Integer(segundos);
			sei();
		}
		else
		{
			PORTC |= (1 << rojo);
			PORTC |= (1 << verde);
			PORTC &= ~(1 << azul);
			cli();
			setCursor(1,0);
			Send_An_Integer(segundos);
			sei();
		}
		
		_delay_ms(100);
    }
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