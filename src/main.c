/*
 *REVO Sensing - Motor Controller 
 *written by: Chris Lee (@sihrc)
 *9/27/2013

 */
#include "api.h"
#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#define F_CPU (1000000L)
#define TIMESCALAR 1024.0
#define F_IO 1000000
#define DT (TIMESCALAR / F_IO) //real time to atmega time conversion
#define PWM_FREQ 20000
#define CYCLE (1 / PWM_FREQ) //switching time
#define INTERNAL (CYCLE / DT)

uint8_t SHUTDOWN = 0;

int main(){
    //Initializing Uart
    initUART();
    //Setting Pins
    //Totem Control Pins R-PD4 (pin 1/OC1A), L-PC1 (pin 2/OC1B)
    DDRD |= _BV(PD4)|_BV(PC1); //To Output
    //LED Pin - PB3
    DDRB |= _BV(PB3); //To Output  LED (was PB5)
    //Input Pin - PB4 (Button)
    DDRB &= ~_BV(PB4); //To Input direction (was PB6)

    DDRB &= ~_BV(PB2); // Input Throttle

    // //Setting up all the other things before looping
    // //Enable ADC, set prescalar to 128 (slow down ADC clock)
    // ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
    // //Enable internal reference voltage
    // ADCSRB &= _BV(AREFEN);
    // //Set internal reference voltage as AVcc
    // ADMUX |= _BV(REFS0);
    // // Left adjust ADC result to allow easy 8 bit reading 
    // ADMUX |= (1 << ADLAR);

    // // Set ADC to Free-Running Mode 
    // ADCSRA &= ~(1 << ADTS0)&~(1 << ADTS1)&~(1 << ADTS2)&~(1 << ADTS3);  
    // // Enable ADC 
    // ADCSRA |= (1 << ADEN);
    // // Start A2D Conversions 
    // ADCSRA |= (1 << ADSC);

    //Enable ADC, set prescalar to 128 (slow down ADC clock)
    ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
    //Enable internal reference voltage
    ADCSRB &= _BV(AREFEN);
    //Set internal reference voltage as AVcc
    ADMUX |= _BV(REFS0);
    //Set ADMUX to ADC5 (0x06)
    ADMUX |= 0x06;

    //Enter Loop
    uint8_t ch; //Selects ADC Channel
    double percentage; //Percentage of voltage for speed

    //For PWM
    // enable timer0 for fast pwm with prescaler of one. 
   TCCR0A|=(1<<WGM00)|(1<<WGM01)|(1<<COM0A1)|(1<<CS00); 
   TCCR1A|=(1<<WGM00)|(1<<WGM01)|(1<<COM0A1)|(1<<CS00);

    for (;;) {
        //Wait for ADC reading
        ADCSRA |=  _BV(ADSC);
        while(bit_is_set(ADCSRA, ADSC));

        //Get the value at PB4 (button input)
        int val = PINB & _BV(PB4);

        //Percentage is fraction equal to duty cycle
        //If Vin = 2.5V, percentage = 0.5
        percentage = ADC/1023;
        //ADC = (Vin / 5.0) * 1023

        // _delay_ms(500);
        int size = sizeof(double)/sizeof(char);
        char str[size];
        sprintf(str, "%f", percentage);
        UART_putString(str);
        UART_putString("\nHello World\n");
        
        //Setting the value at PB6 to the LED at PB5
        //Based on value, forward or backward
        if (SHUTDOWN == 0) { //emergency shut off
        if (val == 0){ //backward
            PORTB &= ~_BV(PB3); //Setting LED to Low
            PORTB &= ~_BV(PD4);  //Setting Left to Low
            if (TCNT1 <= percentage*INTERNAL) {
                PORTB |= _BV(PC1);
            } else if (TCNT1 < INTERNAL) {
                PORTB &= ~_BV(PC1);
            } else {
                TCNT1 = 0;
            }
        }
        else{ //forward
            PORTB |= _BV(PB3); //Setting LED to High
            PORTB &= ~_BV(PC1); //Setting Right to Low
            if (TCNT1 <= percentage*INTERNAL) {
                PORTB |= _BV(PD4);
            } else if (TCNT1 < INTERNAL) {
                PORTB &= ~_BV(PD4);
            } else {
                TCNT1 = 0;
            }
        }
    }
    }
}



/*#define NODE_watchdog       0
#define NODE_bms            1
#define NODE_speedometer    2
#define NODE_halleffect     3
#define NODE_sdlogger       4

#define MSG_critical        0
#define MSG_warning         1
#define MSG_speed           2
#define MSG_voltagelow      3
#define MSG_shunting        4
#define MSG_shutdown        5
#define MSG_data_other      6*/

//Can Message Handling
void handleCANmsg(uint8_t destID, uint8_t msgID, uint8_t* msg, uint8_t msgLen){
    uint8_t cmd = msg[0];
     //if a critical message or shutdown message are sent, shut down the system and stop the clock
    if (msgID == MSG_critical || msgID == MSG_shutdown || msgID == MSG_voltagelow) {
        OCR0A = 0;
        OCR1A = 0;
        SHUTDOWN = 1;
        TCCR0B&=~((1<<CS00)|(1<<CS01)|(1<<CS02)); 
        TCCR1B&=~((1<<CS00)|(1<<CS01)|(1<<CS02)); 
    }   

    //TODO: handle MSG_warning
}


// This is slow PWM method but we are using fast PWM
ISR(TIMER0_COMPA_vect) //Timer Interrupt for OC0A
{ 
/*   PORTD ^= _BV(PD3); // Toggle the right motor
*/}

ISR(TIMER1_COMPA_vect) //Timer Interrupt for OC1A
{ 
/*   PORTD ^= _BV(PD2); // Toggle the left motor
*/}
