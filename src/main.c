/*
 *REVO Sensing - Motor Controller 
 *written by: Chris Lee (@sihrc)
 *9/27/2013

 */
#include "api.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
	//Setting Pins
	//Totem Control Pins R-PD3 (pin 1/OC1A), L-PD2 (pin 2/OC1B)
	DDRD |= _BV(PD2)|_BV(PD3); //To Output
	//LED Pin - PB5
	DDRB |= _BV(PB5); //To Output
	//Input Pin - PB6
	DDRB &= ~_BV(PB6); //To Input

	//Setting up all the other things before looping
	//Enable ADC, set prescalar to 128 (slow down ADC clock)
	ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
	//Enable internal reference voltage
	ADCSRB &= _BV(AREFEN);
	//Set internal reference voltage as AVcc
	ADMUX |= _BV(REFS0);
	// Left adjust ADC result to allow easy 8 bit reading 
	ADMUX |= (1 << ADLAR);

	// Set ADC to Free-Running Mode 
	ADCSRA &= ~(1 << ADTS0)&~(1 << ADTS1)&~(1 << ADTS2)&~(1 << ADTS3);  
	// Enable ADC 
	ADCSRA |= (1 << ADEN);  
	// Start A2D Conversions 
	ADCSRA |= (1 << ADSC);  

	//Enter Loop
	uint8_t ch; //Selects ADC Channel
	double percentage; //Percentage of voltage for speed

	//For PWM
	// enable timer0 for fast pwm with prescaler of one. 
   TCCR0A|=(1<<WGM00)|(1<<WGM01)|(1<<COM0A1)|(1<<CS00); 
   TCCR1A|=(1<<WGM00)|(1<<WGM01)|(1<<COM0A1)|(1<<CS00); 

	for (;;) {
		//Select channel 0 for analog input for reading the pot
		ch = 0;
		 //Have the lowest 4 bits of ADMUX select the channel we want to read
		ADMUX |= ch;
		//Wait for ADC reading
		ADCSRA |=  _BV(ADSC);
		while(bit_is_set(ADCSRA, ADSC));

		//Get the value at PB6 (the button input)
		long val = PINB & _BV(PB6);

		//Getting the percentage voltage of potentiometer
		percentage = ADC/5.0;
		
		//Setting the value at PB6 to the LED at PB5
		//Based on value, forward or backward
		if (val == 0){ //backward
			PORTB &= ~_BV(PB5); //Setting LED to Low
			PORTB &= ~_BV(PB4);  //Setting Left to Low
			TCNT1 = 0; //Reset the other timer not for the right motor
			OCR0A = percentage * 256; //PWM input to Right
		}
		else{ //forward
			PORTB |= _BV(PB5); //Setting LED to High
			PORTB &= ~_BV(PB3); //Setting Right to Low
			TCNT0 = 0; //Reset the other timer not for the left motor
			OCR1A = percentage * 256; //PWM input to Left
		}
	}
}
//Can Message Handling
void handleCANmsg(uint8_t destID, uint8_t msgID, char msg[], uint8_t msgLen){
	 //What messages can this node receive?
	
}
ISR(TIMER0_COMPA_vect) //Timer Interrupt for OC0A
{ 
   PORTD ^= _BV(PD3); // Toggle the right motor
}

ISR(TIMER1_COMPA_vect) //Timer Interrupt for OC1A
{ 
   PORTD ^= _BV(PD2); // Toggle the left motor
}
