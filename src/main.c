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
	//Totem Control Pins R-PB3, L-PB4
	DDRB |= _BV(PB3)|_BV(PB4) //To Output
	//LED Pin - PB5
	DDRB |= _BV(PB5) //To Output
	//Input Pin - PB6
	DDRB &= ~_BV(PB6) //To Input

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
	ADCSRA |= (1 << ADFR);  
	// Enable ADC 
	ADCSRA |= (1 << ADEN);  
	// Start A2D Conversions 
	ADCSRA |= (1 << ADSC);  

	//Enter Loop
	uint8_t ch; //Selects ADC Channel
	for (;;) {

		 //Have the lowest 4 bits of ADMUX select the channel we want to read
		ADMUX |= ch;
		//Wait for ADC reading
		ADCSRA |=  _BV(ADSC);
		while(bit_is_set(ADCSRA, ADSC));

		//Get the value at PB6 (the button input)
		val = PINB & _BV(PB6);

		//Setting the value at PB6 to the LED at PB5
		//Based on value, forward or backward
		if (val == 0) //backward
			PORTB &= ~_BV(PB5); //Setting LED to Low
			PORTB |= _BV(PB4);  //Setting Left to Low
			//backward(ADC); Need to implement analog right
		else //forward
			PORTB |= _BV(PB5); //Setting LED to High
			PORTB |= _BV(PB3); //Setting Right to Low
			//forward(ADC); Need to implement analog right
	}
}

//Timer for analog write
/*
Use a timer, set minimum time and maximum time for Setting
low and high to get the right % of time. for percentage of 
voltage.
*/
