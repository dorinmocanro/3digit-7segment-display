/* 
 * File:   main.c
 * Author: Mocan Marin Dorin
 *
 * Created on 04 aprilie 2019, 15:44
 * 
 * NOTE:
 * check my youtube channel on how I've connected the 3x 7LED SEGMENT display
 * on my microcontroller. I am using a single 8bit SHIFT Register
 *
 * my YOUTUBE CHANNEL: 
 * https://www.youtube.com/c/DorinMocan
 * 
 * NOTE 2:
 * I hope that the only thing you need to change is PORTs, LATCHes and TRISes
 * look below for "CHANGE THIS"
 * also, deppending on what microcontroller are you using, you might be needing
 * to generate your OWN "config.h" configuration file
 * 
 * You can ask more questions on my email
 * dorin.mocan.ro@gmail.com
 */

#include <xc.h>
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*////////////////////////////////////////////////////////////
//CHANGE_THIS if needed:
#define PORT_DISP PORTX
#define LAT_DISP  LATX
#define TRIS_DISP TRISX
 * where X is the Letter of your PORT */
//THIS IS WHAT YOU NEED TO CHANGE IF USING A DIFFERENT PORT
#define PORT_DISP PORTB
#define LAT_DISP  LATB
#define TRIS_DISP TRISB
/////////////////////////////////////////////////////////////

////////////////////////////////////
//define  PIN position used for SHIFT REGISTER
typedef const unsigned char POZ;
POZ SER=0x01;
POZ SRCLK=0x01<<1;
POZ SRCLR=0x01<<2;
POZ RCLK=0x01<<3;
POZ OE=0x01<<4;
//pins used by DISPLAY
POZ DIG1=0x20;
POZ DIG2=0x40;
POZ DIG3=0x80;
///////////////////////////////////


////////////////////////////////////
//Some bit masks
typedef const unsigned char BIT_MASK;
BIT_MASK BIT_0=0x01;
BIT_MASK BIT_1=0x01<<1;
BIT_MASK BIT_2=0x01<<2;
BIT_MASK BIT_3=0x01<<3;
BIT_MASK BIT_4=0x01<<4;
BIT_MASK BIT_5=0x01<<5;
BIT_MASK BIT_6=0x01<<6;
BIT_MASK BIT_7=0x01<<7;
////////////////////////////////////

////////////////////////////////
// Glabal VARS
char buffer[4];         //used by numarator() and *ISR()
unsigned int NR=1;      //used by numarator() and *ISR()
unsigned char count=1;  //used by numarator() and *ISR()
/////////////////////////////////


#define _XTAL_FREQ 8000000      //used by delays()
#define MAX_DIGIT 3     //maximum digits on display. used in display3() only

/////////////////////////////////////////////////////////////////////////////////
//helper functions 
//
//data ready
#define data_ready()  LAT_DISP=(LAT_DISP & ~(SER|SRCLK|SRCLR|RCLK)) | RCLK;\
                       LAT_DISP=(LAT_DISP & ~(SER|SRCLK|SRCLR|RCLK));
//these 2 functions are not realy necesary, 
//I just call output_enable(). the (~OE) pin can be connected directly to GND
#define output_enable()   LATB &=~OE // CLEAR only the OE bit
#define output_disable()  LATB |=OE  // SET   only the OE bit
/////////////////////////////////////////////////////////////////



//setup porta
void setupPA(void){
    //initializare PORTA pentru afisaj-7segment 
    PORTA=0x00; //sterge tot 
    ADCON1=0x0f;
    CMCON=0x07;
 //   TRISA=0xff; //  Start cu toti pinii in stare high-impedance 
    
}

//this is a generic . it may be  necessary to do some changes
//depending on your port config
void setupPortDisplay(){
    PORT_DISP=0x00;
    CMCON=0x07;     //comparators off
    ADCON1=0x0F;    // everything is digital, 
                    // may be needed to modify
                    // to modify only this port
    TRIS_DISP=0x00;      //everything is an output
    OSCCON=0b01110000; //8MHz. internal 
}

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
        case 'a': return(unsigned char) ~(d|h);  // the  ~  returns an "int" 
        case 'b': return(unsigned char)  ~(a|b|h);
        case 'c': return(unsigned char) ~(b|c|g|h) ;
        case 'd': return(unsigned char) ~(a|f |h);
        case 'e': return(unsigned char) ~(b|c |h) ;
        case 'f': return(unsigned char) ~(b|c|d |h);
        case '0': return(unsigned char) ~(g |h);
        case '1': return(unsigned char) b|c;
        case '2': return(unsigned char) ~(c|f |h);
        case '3': return(unsigned char) ~(f|e |h);
        case '4': return(unsigned char) ~(a|d|e |h);
        case '5': return(unsigned char) ~(b|e |h) ;
        case '6': return(unsigned char) ~(b |h);
        case '7': return(unsigned char) a|b|c;
        case '8': return(unsigned char) ~(o |h);
        case '9': return(unsigned char) ~(e |h);
        case '.': return(unsigned char) h;
        case 0  : return(unsigned char) o; //digit# off
        case 'o': return(unsigned char) o; //Digit# off
        default: return (unsigned char) ~(c|d); // "P"
    } 
}

void send_digit(unsigned char data){
    unsigned  char a;
    POZ com_mask=SER|SRCLK|SRCLR|RCLK ;  //a sum of all interface pins. for masking
    LAT_DISP&=~com_mask; // 1110.000 bitwise AND. clear the comunication pins
                // ( output (/OE) is set to '0' == ENABLED (default))
    LAT_DISP|=SRCLR; // exit the clear state (/SRCLR =/1)

    for(int i=0;i<8; i++ )// shift right/left 8x times
    {
        //rotate data first , so in RB0 you have the data's MSBit
        data= (data<<1) | (data>>7);    //rotate LEFT 8bit
        a=LAT_DISP & ~(SER|SRCLK); //read portB to a, and clear the last 2 bits (==0))
                        //clears CLOCK and Data bit (SER) 
        a =a| (data & SER); // copy the data's last bit  (data:0) into a
        LAT_DISP=a;
        LAT_DISP|=SRCLK;
    }
    //send data, but do not activate the OUTPUT
    //...missing. it is added on the parent/caller function
}

void display3(const char* data3){
     const unsigned char 
             dig_mask= (DIG1 | DIG2 | DIG3); //  1110 0000  by default 
    
    // this _SHIFT_LEFT_will right-justify the characters
    unsigned char csize=strlen(data3);   

    volatile unsigned char bit=DIG1;
    //RIGHT JUSTIFY the numbers on DISPLAY
    bit= (bit<<(MAX_DIGIT-csize)) | (bit>>(8-MAX_DIGIT+csize)); 

    //Ensure bit is in the range of the defined DIG#
    // *no need to check for NULL character; `csize' (above) takes care of that.
    while( bit & dig_mask )    { 
        output_enable();    //just in case this is disabled. NB: /OE may create ghosting
        send_digit(  character_table(*(data3++))  ); //ideea: |character_table('.')
        LAT_DISP |=  dig_mask;   // TUNR OFF. => transistor pnp; 1= off; SET the 3MSB; aka OUTPUT off
        data_ready();           //load the new values in SHift REG
        __delay_us(30);     //anti ghosting on display
        LAT_DISP &= ~bit;   //TURN ON DIG# pnp tranzistor, active on LOW.

        //set bit 1,2 or 3 and increment (shift) to next bit)
        bit = (bit<<1)|(bit>>7);    //shift left 
       //  bit= (bit>>1)| (bit<<7); //shift RIGHT
        
    }
    //this function ensures the same duty cycle for all 3 DIGits
    //an oscilloscope may be used here to see (for my case is approx 19,3%)
    send_digit('o');__delay_us(20); LAT_DISP|=dig_mask;
    //output_disable();
}

//an ISR for counting (seconds). mostly demo for the display
void __interrupt(high_priority) my_ISR(){
#define oneSec 30
    //check if timer TMR0 overflowed
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
//_this function is for development stage
//_this function can help you to write the:
//_character_table(unsigned char)
    unsigned char test_bit=0x01<<(poz-1);
    LAT_DISP=(unsigned char)~(DIG1 | DIG2 | DIG3); //quick and dirty, all leds on
    while (1){
        send_digit(test_bit); data_ready(); output_enable();
        __delay_ms(2000);
     /*   //uncomment next instruction if you want all the LEDs to blink */
        test_bit=test_bit<<1 | test_bit>>7; 
    }
}
        
int main(int argc, char** argv) {
    //setupPA();
    setupPortDisplay();
    setup_TMR0();
    
    //test_segments( 1 );
    
    NR=123;
    numarator();  // this will never return; 
    
    return (EXIT_SUCCESS);
}

