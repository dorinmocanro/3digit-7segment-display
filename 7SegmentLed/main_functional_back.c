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


#define _XTAL_FREQ 8000000
/*
 * for __delay_ms(unsigned) or __delay_us(unsigned)
 */


void my_function(const char a, const char b , const char c){
    return;
}

//setup porta
void setupPA(void){
    //initializare PORTA pentru afisaj-7segment 
    PORTA=0x00; //sterge tot 
    ADCON1=0x0f;
    CMCON=0x07;
    TRISA=0xff; //  Start cu toti pinii in stare high-impedance 
    
}

void setupPB(){
    PORTB=0x00;
    ADCON1=0x0F;    // everything is digital, 
                    // may be needed to modify
                    // to modify only this port
    TRISB=0x00;      //everything is an output
    
}

void display_digit(unsigned char );

void test1(){
    setupPB();
    PORTB=0x10; // clear everything, output (/OE) is disabled

    PORTB=0x15;
    PORTB=0x17;

    PORTB=0x14;
    PORTB=0x16;

    PORTB=0x14;
    PORTB=0x16;

    PORTB=0x15;
    PORTB=0x17;

    PORTB=0x14;
    PORTB=0x16;

    PORTB=0x14;
    PORTB=0x16;

    PORTB=0x15;
    PORTB=0x17;

    PORTB=0x14;
    PORTB=0x16;
 
// x8 times    
    PORTB=0x08;
}






//this function defines eatch led and return a symbol char for display_digit
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
        case 'b': return ~(a|c|h);
        //case 'b':return b|d|e|f|g;
        case 'c': return ~(b|c|e|h) ;
        case 'd': return ~(a|g |h);
        case 'e': return ~(b|c |h) ;
        case 'f': return ~(b|c|d |h);
        case '0': return ~(e |h);
        case '1': return b|c;
        case '2': return ~(b|g |h);
        case '3': return ~(f|g |h);
        case '4': return ~(a|d|f |h);
        case '5': return ~(c|f |h) ;
        case '6': return ~ (c |h);
        case '7': return a|b|c;
        case '8': return ~(o |h);
        case '9': return ~(f |h);
        case '.': return h;
        default: return  ~(b|d); // "P"
    }
   
    
}

disp_test(){ 
    //verifica pozitia la care fiecare led este conectat
    unsigned char c_symbol=0x01;
    while(1) {
    //shift "1" to the left
    display_digit(c_symbol); c_symbol=(c_symbol <<1) | (c_symbol >>7) ;
    __delay_ms(500);

    //a quick flash
    display_digit(0xff); __delay_ms(20);

    }
    
}

void display_digit(unsigned char data){
    unsigned  char a;
    PORTB&=0xE0; // 1110.000 bitwise AND. clear the comunication pins ( output (/OE) is set to '0' ==? ENABLED (default))
    PORTBbits.RB2=1; 
    for(int i=0;i<8; i++ )// shift right/left 8x times
    {
        //rotate data first , so in RB0 you have the data's MSBit
        data= (data<<1) | (data>>7);
        a=LATB & 0xFC; //read portB to a, and clear the last 2 bits (==0))
                        //clears CLOCK and Data bit (SER) 
        a =a| (data & 0x01); // copy the data's last bit  (data:0) into a
        LATB=a;
        PORTBbits.RB1=1;    //set CLOCK high
/*   
        //data>>=1;   //deplasare spre dreapta
        //this formula will correctly rotate left (towards LSBit)
        //data= (data<<1) | (data>>7);
*/
    }
    //send data
    //reedited with an OR, to only change the desired pin(s). 
    a=PORTB & 0xE0;    //apply mask. This wil lkeep the 3 MSBits unchanged.
    a|=0x08;
    PORTB =a;   //  !OutputEnable is low; ReadyClock is high; others: N.A.
}



void test2(unsigned char data){
    setupPB();
    unsigned  char a;
    
     
    PORTB=0x10; // clear everything, output (/OE) is disabled
    PORTBbits.RB2=1;    //reset CLEAR-state
    int i=0;
    for(i=0;i<8; i++ )// shift right 7x times
    {
        //rotate data first , so in RB0 you have the data's MSBit
        data= (data<<1) | (data>>7);
        a=LATB & 0xFC; //read portB to a, and clear the last 2 bits (==0))
                        //also clears CLOCK
        a =a| (data & 0x01); // copy the data's last bit  (data:0) into a
        LATB=a;
        PORTBbits.RB1=1;    //set CLOCK high
   
        //data>>=1;   //deplasare spre dreapta
        //this formula will correctly rotate left (towards LSBit)
        //data= (data<<1) | (data>>7);
    }
  
    
    //send data
    PORTB = 0x08;
    //test1();
    
}


void test3(){
    setupPB();
    PORTB=0x10; // clear everything, output (/OE) is disabled
    PORTBbits.RB2=1;    //reset CLEAR-state
    for (int i=0;i<3;i++){
        PORTB=0x15;
        PORTB=0x17;
    }

 /*   
    PORTB=0x15;
    PORTB=0x17;
    
    PORTB=0x15;
    PORTB=0x17;
    
    PORTB=0x15;
    PORTB=0x17;
    
    PORTB=0x15;
    PORTB=0x17;
    

    PORTB=0x15;
    PORTB=0x17;

    PORTB=0x15;
    PORTB=0x17;

    PORTB=0x15;
    PORTB=0x17;
 */
// x8 times    
    PORTB=0x08;
}

//more of a test function
void enumerate_symbols(){
     //enumerate characters for ever
    unsigned char c='0';
    while (1) {
        if (c=='g') c='0';
        else if (c==58) c='a'; // if c==10 (ascii))
        display_digit(character_table(c++)); __delay_ms(50);
    }
}

void display3(char* data3){
    for(unsigned char bit=0x20 ; bit!=0x01 ;  ) { //do a shift to the left 
        //set bit 1,2 or 3;
        LATB=(LATB & 0x1F)| bit; //chear the 3 MSB and set the new bit (that was shifted to the left))
        
        display_digit(character_table(*(data3++) ) ); __delay_ms(50);
        bit = (bit<<1)|(bit>>7); __delay_ms(50);
    }
}

int main(int argc, char** argv) {
    //setupPA();
    setupPB();
    //LATA=0x00; //reset everything
   // test2();
   // test2(0xff);
    
    LATBbits.LB5=1; //bit7(MSB) == Digit 1
                    //bit 6     == Digit 2
                    //bit 5     == Digit 3
    char buffer[4]="120";
    while (1) display3(buffer);
    //enumerate_symbols(); //repeat forever
    
    while (1);
    
    //work to be done  
    /* this works but is a test*/
    char my_string[5]="ab"; 
    char * my_pointer=my_string;
    display_digit(character_table(*my_pointer));__delay_ms(50);my_pointer++;
    display_digit(character_table((strlen(my_string)+'0'))); __delay_ms(500); // convert to char
    display_digit(character_table(*my_pointer));__delay_ms(50);my_pointer++;
    display_digit(character_table(*my_pointer));__delay_ms(50);my_pointer++;
    
    while(1){

    
        CLRWDT();   //this is disabled by me in config.h
    } // hold in here
    return (EXIT_SUCCESS);
}

