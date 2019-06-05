/* 
 * File:   main.c
 * Author: dorin
 *
 * Created on 04 aprilie 2019, 15:44
 */

#include <xc.h>
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT PORTB

typedef union  {
    struct PORT;
    struct {
        unsigned p0     :1;
        unsigned p1     :1;
        unsigned p2     :1;
        unsigned p3     :1;
        unsigned p4     :1;
        unsigned p5     :1;
        unsigned p6     :1;
        unsigned p7     :1;
    };
}_PORT_TYPE;

_PORT_TYPE my_test_port;


#define _XTAL_FREQ 8000000
char buffer[4];
unsigned int NR=1;
unsigned char count=1;

typedef const unsigned char POZ;
POZ SER=0x01;
POZ SRCLK=0x01<<1;
POZ SRCLR=0x01<<2;
POZ RCLK=0x01<<4;
POZ OE=0x01<<3;
/*
digits needs to be in sequence: like so:
POZ DIG1=0x20;
POZ DIG2=0x40;
POZ DIG3=0x80;
 */

POZ DIG1=0x20;
POZ DIG2=0x40;
POZ DIG3=0x80;
/*
 * for __delay_ms(unsigned) or __delay_us(unsigned)
 */


//setup porta
void setupPA(void){
    //initializare PORTA pentru afisaj-7segment 
    PORTA=0x00; //sterge tot 
    ADCON1=0x0f;
    CMCON=0x07;
 //   TRISA=0xff; //  Start cu toti pinii in stare high-impedance 
    
}

void setupPB(){
    PORTB=0x00;
    ADCON1=0x0F;    // everything is digital, 
                    // may be needed to modify
                    // to modify only this port
    TRISB=0x00;      //everything is an output
    OSCCON=0b01110000; //8MHz. internal 
}

//only declaration. 
void send_digit(unsigned char );

//this function defines each led and return a symbol char for send_digit
unsigned char character_table(unsigned char symbol){
    typedef const unsigned char c_t;  //character type c_t
    //assign to eatch LED a bit:
    c_t o=0x00; //define 'o' as zero / off
    c_t a=0x01;
    c_t b=0x01<<1;
    c_t c=0x01<<2;
    c_t d=0x01<<3;
    c_t e=0x01<<4;
    c_t f=0x01<<5;
    c_t g=0x01<<6;
    c_t h=0x01<<7;  // the dot LED
    
    //defining characters
    switch (symbol){
        //  |h will deactivate the implicit "dot" on NOT (EXPRESION);
        case 'a': return ~(d|h);
        
        case 'b': return  ~(a|b|h);
        //case 'b':return b|d|e|f|g;
        case 'c': return ~(b|c|g|h) ;
        case 'd': return ~(a|f |h);
        case 'e': return ~(b|c |h) ;
        case 'f': return ~(b|c|d |h);
        case '0': return ~(g |h);
        case '1': return b|c;
        case '2': return ~(c|f |h);
        case '3': return ~(f|e |h);
        case '4': return ~(a|d|e |h);
        case '5': return ~(b|e |h) ;
        case '6': return ~(b |h);
        case '7': return a|b|c;
        case '8': return ~(o |h);
        case '9': return ~(e |h);
        case '.': return h;
        case 0  : return o; //digit# off
        case 'o': return o; //Digit# off
        default: return  ~(c|d); // "P"
    }
   
    
}

//helper functions (needed for better sincronisation with digit shift/select)
//the "inline instruction does not create an inline function... 
 //data ready, but deactivate output
    //REG =  xxx0 000   and set  0001 1000

//send data, but do not activate the OUTPUT
#define data_ready()  LATB=(LATB & ~(SER|SRCLK|SRCLR|RCLK)) | RCLK;\
                       LATB=(LATB & ~(SER|SRCLK|SRCLR|RCLK));
#define output_enable()    LATB&=~OE //LATBbits.LB4=0;
#define output_disable()   LATB |=OE //LATBbits.LB4=1;

void send_digit(unsigned char data){
    unsigned  char a;
    POZ com_mask=SER|SRCLK|SRCLR|RCLK ;  //a sum of all interface pins. for masking
    LATB&=~com_mask; // 1110.000 bitwise AND. clear the comunication pins
                // ( output (/OE) is set to '0' ==? ENABLED (default))
                //0xF* /OE is 1 => output is off
    LATB|=SRCLR; // exit the clear state (/SRCLR =/1)
    //PORTBbits.RB2=1; //need to add this instr in the above instr.. (testing?))
    for(int i=0;i<8; i++ )// shift right/left 8x times
    {
        //rotate data first , so in RB0 you have the data's MSBit
        data= (data<<1) | (data>>7);
        a=LATB & ~(SER|SRCLK); //read portB to a, and clear the last 2 bits (==0))
                        //clears CLOCK and Data bit (SER) 
        a =a| (data & SER); // copy the data's last bit  (data:0) into a
        LATB=a;
        LATB|=SRCLK;
/*   
        //data>>=1;   //deplasare spre dreapta
        //this formula will correctly rotate left (towards LSBit)
        //data= (data<<1) | (data>>7);
*/
    }
    //send data, but do not activate the OUTPUT
    //...missing. it is added on the parent/caller function
}


//this function depends heavily on PORTB
//all the pins are used. so it may be needed to change this function
//in the future
//the 3 Most Significant Bits are used for multiplexing.!!!
void display3(const char* data3){
#define MAX_DIGIT 3 
    const unsigned char 
        dig_mask= (DIG1 | DIG2 | DIG3); //  1110 0000  by default 
    
    // this _SHIFT_LEFT_will right-justify the characters
    unsigned char csize=strlen(data3);   
    volatile unsigned char bit=DIG1;
    
    bit= (bit<<(MAX_DIGIT-csize)) | (bit>>(8-MAX_DIGIT+csize)); 
    // ^skip 1 or 2 digits from MSB, RIGHT_JUSTIFY
  
    //bit it is shifted to the left by 1.
    //Ensure bit is in the range of the defined DIG#
    // no need to check for NULL character; `csize' (above) takes care.
    while( bit & (DIG1|DIG2|DIG3) )    { 
        output_enable();    //just in case this is disabled. NB: /OE may create ghosting
        send_digit(  character_table(*(data3++))  ); //ideea: |character_table('.')
        //send_digit(character_table('.'))
        
        
        LATB |=  dig_mask;   //pnp; 1= off; SET the 3MSB; aka OUTPUT off
        data_ready();
        __delay_us(30);
        LATB &= ~bit;   //pnp tranzistor, active on LOW.
/*       
        LATB=(LATB & dig_mask);//clear the 3 MSB.  aka Output off
        data_ready();
        __delay_us(30);  //wait for inductive R (prevent ghosting))        
        LATB|= bit; // and set the new bit (that was shifted to the left))
 * */
        //set bit 1,2 or 3 and increment (shift) to next bit)
        bit = (bit<<1)|(bit>>7);    //shift left 
       //  bit= (bit>>1)| (bit<<7);
        
    }
    //this function ensures the same duty cycle for all 3 DIGits
    //an oscilloscope may be used here to see (for my case is approx 19,3%)
    send_digit('o');__delay_us(20); LATB|=dig_mask;
    //output_disable();
    
}



typedef const unsigned char BIT_MASK;
BIT_MASK BIT_0=0x01;
BIT_MASK BIT_1=0x01<<1;
BIT_MASK BIT_2=0x01<<2;
BIT_MASK BIT_3=0x01<<3;
BIT_MASK BIT_4=0x01<<4;
BIT_MASK BIT_5=0x01<<5;
BIT_MASK BIT_6=0x01<<6;
BIT_MASK BIT_7=0x01<<7;



//trying to write an ISR (no priority)
void __interrupt(high_priority) my_ISR(){
    //check if timer TMR0 overflowed
    //if(TMR0IE);       //blah.. i prefer using masks
#define oneSec 30
    if( (INTCON & BIT_5) && (INTCON & BIT_2) ) { //if TMR0IE and TMR0IF
        INTCON &= ~BIT_2;                   //clear timer 0 Flag
        
        count++;
        if(count>=oneSec) { //approx 1 sec
            count-=oneSec;
            NR++;
            if(NR==1000) NR=0;
        }        
    }
}

void setup_TMR0(){
    T0CON=0b10001111;   //no prescaler, etc...
    TMR0H=0x00;             //this, aka Timer 0 will wi updated one TMR0l is written to
    TMR0L=0x00;          //writing to TMR0. the increment is inhibited for 2 instr Cycle
    INTCONbits.TMR0IE=1;        //enable interrupt
    INTCONbits.GIE=1;
    RCONbits.IPEN=0;        //disable priority interruption
}

//this function will never return
void numarator(){
    unsigned int nr=0; //comparator with previous value) 
    while (1){
        if(nr!=NR){  //its time-cheaper to test the old values, 
                     //^than to sprintf()
                     //NR is set by ISR setup_TMR0()
            snprintf(buffer,4,"%d",NR);
            nr=NR;
        }
        display3(buffer); //__delay_ms(30);
    }
}



void test_segments(unsigned char poz){
    //this function helps you identify each LEDs position
//_this function if for development stage
//_this function can help you to write the:
//_character_table(unsigned char)
    

    unsigned char test_bit=0x01<<(poz-1);
    LATB=~(DIG1 | DIG2 | DIG3); //quick and dirty, all leds on
    while (1){
        send_digit(test_bit); data_ready(); output_enable();
        __delay_ms(2000);
     /*   //uncomment next instruction if you want all the LEDs to blink */
        //test_bit=test_bit<<1 | test_bit>>7; 
    }
}
        
int main(int argc, char** argv) {
    //setupPA();
    setupPB();
    setup_TMR0();
    
    //test_segments( 8 );
    
    NR=123;
    numarator();  // this will never return; 
    
    return (EXIT_SUCCESS);
}


/*
 display3
 * 
 
//this function depends heavily on PORTB
//all the pins are used. so it may be needed to change this function
//in the future
//the 3 Most Significant Bits are used for multiplexing.!!!
void display3(const char* data3){
#define MAX_DIGIT 3 
    const unsigned char 
        dig_mask= ~(DIG1 | DIG2 | DIG3); //0x1f  by default 
    // this _SHIFT_LEFT_will right-justify the characters
    unsigned char csize=strlen(data3);   
    volatile unsigned char bit=DIG1;
    bit= (bit<<(MAX_DIGIT-csize)) | (bit>>(8-MAX_DIGIT+csize)); 
    // ^skip 1 or 2 digits from MSB, RIGHT_JUSTIFY
  
    //bit it is shifted to the left by 1.
    //Ensure bit is in the range of the defined DIG#
    // no need to check for NULL character; `csize' (above) takes care.
    while( bit & (DIG1|DIG2|DIG3) )    { 
        output_enable();    //just in case this is disabled. NB: /OE may create ghosting
        send_digit(  character_table(*(data3++))  ); //ideea: |character_table('.')
        //send_digit(character_table('.'))
        
        LATB=(LATB & dig_mask);//clear the 3 MSB.  aka Output off
        data_ready();
        __delay_us(30);  //wait for inductive R (prevent ghosting))        
        LATB|= bit; // and set the new bit (that was shifted to the left))
        //set bit 1,2 or 3 and increment (shift) to next bit)
        bit = (bit<<1)|(bit>>7);    //shift left 
       //  bit= (bit>>1)| (bit<<7);
        
    }
    //this function ensures the same duty cycle for all 3 DIGits
    //an oscilloscope may be used here to see (for my case is approx 19,3%)
    send_digit('o');__delay_us(30); LATB=(LATB & dig_mask);
    //output_disable();
    
}


 */