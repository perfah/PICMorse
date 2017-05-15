/*
	PICMorse - a PIC16 morse code translator that reads input from UART
	Developed by Per Fahlander and Aron Hansen Berggren
	PIC16F690 compiled with B. Knudsen Cc5x Free, not ANSI-C 
	
				******************************
							HARDWARE                 
				******************************
				 _____________  _____________ 
				|             \/             |
		+5V>	|Vdd        16F690        Vss|	<GND
				|RA5            RA0/AN0/(PGD)|	<UART TOOL (bbTx)
				|RA4/AN3            RA1/(PGC)|	<UART TOOL (bbRx)
				|RA3/!MCLR/(Vpp)  RA2/AN2/INT|
    SPEAKER>	|RC5/CCP1                 RC0|
				|RC4                      RC1|
				|RC3/AN7                  RC2|
				|RC6/AN8             AN10/RB4|
				|RC7/AN9               RB5/Rx|
				|RB7/Tx                   RB6|
				|____________________________|                                      

*/

#include "16F690.h"
#include "int16Cxx.h"
#pragma config |= 0x00D4
#pragma char X @ 0x115

/*
	The interrupt routine works as a neccessary backend for this project:
*/

bit receiver_flag; // A bit-sized signal-flag determing whether a byte has been received from UART
char receiver_byte; // The (latest) transfered byte

// Originally copied from labs
#pragma origin 4
interrupt int_server( void ) {
	int_save_registers
	/* 
		New interrupts are automaticaly disabled            
		"Interrupt on change" at pin RA1 from PK2 UART-tool
	*/
  
	/* Interpret this as the startbit  */
	if( PORTA.1 == 0 ) {  
		/* Receive one full character   */
		char bitCount, ti;
		/* delay 1,5 bit 156 usec at 4 MHz         */
		/* 5+28*5-1+1+2+9=156 without optimization */
		ti = 28; do ; while( --ti > 0); nop(); nop2();
		for( bitCount = 8; bitCount > 0 ; bitCount--) {
			Carry = PORTA.1;
			receiver_byte = rr( receiver_byte);  /* rotate carry */
			/* delay one bit 104 usec at 4 MHz       */
			/* 5+18*5-1+1+9=104 without optimization */ 
			ti = 18; do ; while( --ti > 0); nop(); 
        }
		
		receiver_flag = 1; /* A full character is now received */
    }
	RABIF = 0; //Reset the RABIF-flag before leaving
	int_restore_registers
	/* New interrupts are now enabled */
}

/*
	End of interrupt routine.
*/

#include "math16.h"

#define SPACING_LENGTH 100 // The time (in ms) put in-between each letter
#define TIME_CONSTANT 70 // A percentage amount of tempo to actually give the playback (Speed up/slow down)
#define MAX_MESSAGE_LENGTH 48 // The longest allowed message length for the input

void delay( char);
void init_io_ports( void );
void init_serial( void );
void init_interrupt( void );
void putchar( char);
void printf(const char *string, char variable);

typedef enum {
	DOT = 50,
	DASH = 150,
	_ = 0
} BeepLength;

void beep_for_time( char ms ){
	TRISC.5 = 0; //CCP1 output on
	delay(ms); // Delay of the same length as the beep
	TRISC.5 = 1; //CCP1 output off
}

void beep_letter( BeepLength slot1, BeepLength slot2, BeepLength slot3, BeepLength slot4 ){	
	if (slot1 == _) 
		return;
	beep_for_time(slot1);
	delay(SPACING_LENGTH); 
	
	if (slot2 == _) 
		return;
	beep_for_time(slot2);
	delay(SPACING_LENGTH); 
	
	if (slot3 == _) 
		return;
	beep_for_time(slot3);
	delay(SPACING_LENGTH); 
	
	if (slot4 == _)
		return;
	beep_for_time(slot4);
	delay(SPACING_LENGTH); 
}

void main( void) { 
	init_io_ports();
	init_serial();
	init_interrupt();

  	TRISC.5 = 1;              // CCP1 output            
	CCP1CON = 0b00.00.1100;   // PWM-mode                
	T2CON   = 0b00000.1.00;   // prescale 1:1            
	PR2     = 255;            // max value               
	CCPR1L = 100; 
	
	while(1){	
		char message[MAX_MESSAGE_LENGTH];
		char message_length = 0;

		printf("\r\nEnter text for translation:  ", 0);
		
		while (receiver_byte != '\n'){	
			// Wait for a character to be received from UART
			while (!receiver_flag);

			// Save received byte and increment length of message
			message[message_length] = receiver_byte;
			message_length++;
			
			// Reset interrupt flag
			receiver_flag = 0; 
		}
		
		if (message_length){
			// A message is now available for decoding
			printf("\r\nCaught new message of length:  %u", message_length - 1);
			printf("\r\nTranslating message:  ", 0);
			
			int pos;
			for (pos = 0; pos < message_length; pos++){
				printf("\r %c ", message[pos]);
				
				switch (message[pos]){
					case 'a':	
						beep_letter(DOT, DASH, _, _);
						break;
						
					case 'b':	
						beep_letter(DASH, DOT, DOT, DOT);
						break;
						
					case 'c':
						beep_letter(DASH, DOT, DASH, DOT);
						break;
						
					case 'd':
						beep_letter(DASH, DOT, DOT, _);
						break;
						
					case 'e':
						beep_letter(DOT, _, _, _);
						break;
					
					case 'f':
						beep_letter(DOT, DOT, DASH, DOT);
						break;
					
					case 'g':
						beep_letter(DASH, DASH, DOT, _);
						break;
						
					case 'h':
						beep_letter(DOT, DOT, DOT, DOT);
						break;
						
					case 'i':
						beep_letter(DOT, DOT, _, _);
						break;
						
					case 'j':
						beep_letter(DOT, DASH, DASH, DASH);
						break;
						
					case 'k':
						beep_letter(DASH, DOT, DASH, _);
						break;
						
					case 'l':
						beep_letter(DOT, DASH, DOT, DOT);
						break;
						
					case 'm':
						beep_letter(DASH, DASH, _, _);
						break;
						
					case 'n':
						beep_letter(DASH, DOT, _, _);
						break;
						
					case 'o':
						beep_letter(DASH, DASH, DASH, _);
						break;
						
					case 'p':
						beep_letter(DOT, DASH, DASH, DOT);
						break;
						
					case 'q':
						beep_letter(DASH, DASH, DOT, DASH);
						break;
						
					case 'r':
						beep_letter(DOT, DASH, DOT, _);
						break;
						
					case 's':
						beep_letter(DOT, DOT, DOT, _);
						break;
						
					case 't':
						beep_letter(DASH, _,_,_);
						break;
						
					case 'u':
						beep_letter(DOT, DOT, DASH, _);
						break;
						
					case 'v':
						beep_letter(DOT, DOT, DOT, DASH);
						break;
						
					case 'w':
						beep_letter(DOT, DASH, DASH, _);
						break;
						
					case 'x':
						beep_letter(DASH, DOT, DOT, DASH);
						break;
						
					case 'y':
						beep_letter(DASH, DOT, DASH, DASH);
						break;
						
					case 'z': 
						beep_letter(DASH, DASH, DOT, DOT);
						break;
			
					default:
						break;
				}
			}
			
			printf("\r\n", 0);
		}
	}
}

// Originally copied from labs
void delay( char millisec) {
	millisec *= 100 / TIME_CONSTANT;
	
    OPTION = 2;  /* prescaler divide by 8        */
    do {
		TMR0 = 0;
		while ( TMR0 < 125)   /* 125 * 8 = 1000  */ ;
    } while ( -- millisec > 0);
}

// Originally copied from labs
void init_io_ports( void ) {
  TRISC = 0xF8; // 11111000 0 is for outputbit  */
  PORTC = 0b000;    /* initial value */

  ANSEL =0;     // not AD-input      
  TRISA.5 = 1;  // input rpgA        
  TRISA.4 = 1;  // input rpgB        
  
  // Enable week pullup's          
  OPTION.7 = 0; // !RABPU bit        
  WPUA.5   = 1; // rpgA pullup      
  WPUA.4   = 1; // rpgB pullup      
  X.6 = 1;
  TRISB.6 = 1;  // PORTB pin 6 input
  
  return;
}

// Originally copied from labs
// Initialise PIC16F690 bitbang serialcom
void init_serial( void ) {
   ANSEL.0 = 0; // No AD on RA0             
   ANSEL.1 = 0; // No AD on RA1             
   PORTA.0 = 1; // marking line            
   TRISA.0 = 0; // output to PK2 UART-tool 
   TRISA.1 = 1; // input from PK2 UART-tool
   receiver_flag = 0 ;
   return;      
}

// Originally copied from labs
void init_interrupt( void ) {
  IOCA.1 = 1; /* PORTA.1 interrupt on change */
  RABIE =1;   /* interrupt on change         */
  GIE = 1;    /* interrupt enable            */
  return;
}

// Originally copied from labs
void putchar( char ch ) {
  char bitCount, ti;
  PORTA.0 = 0; // set startbit
  
  for ( bitCount = 10; bitCount > 0 ; bitCount-- ) {
     /* delay one bit 104 usec at 4 MHz       */
     /* 5+18*5-1+1+9=104 without optimization */ 
     ti = 18; do ; while( --ti > 0); nop(); 
     Carry = 1;     /* stopbit                    */
     ch = rr( ch ); /* Rotate Right through Carry */
     PORTA.0 = Carry;
  }
  
  return;
}

// Originally copied from labs
void printf(const char *string, char variable){
  char i, k, m, a, b;
  for(i = 0 ; ; i++)
   {
     k = string[i];
     if( k == '\0') break;   // at end of string
     if( k == '%')           // insert variable in string
      {
        i++;
        k = string[i];
        switch(k)
         {
           case 'd':         // %d  signed 8bit
             if( variable.7 ==1) putchar('-');
             else putchar(' ');
             if( variable > 127) variable = -variable;  // no break!
           case 'u':         // %u unsigned 8bit
             a = variable/100;
             putchar('0'+a); // print 100's
             b = variable%100;
             a = b/10;
             putchar('0'+a); // print 10's
             a = b%10;
             putchar('0'+a); // print 1's
             break;
           case 'b':         // %b BINARY 8bit
             for( m = 0 ; m < 8 ; m++ )
              {
                if (variable.7 == 1) putchar('1');
                else putchar('0');
                variable = rl(variable);
               }
              break;
           case 'c':         // %c  'char'
             putchar(variable);
             break;
           case '%':
             putchar('%');
             break;
           default:          // not implemented
             putchar('!');
         }
      }
      else putchar(k);
   }
}
