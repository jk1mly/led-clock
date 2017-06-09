//  ７セグＬＥＤ時計
//  東芝アマチュア無線クラブ
//      JA1YTS:Toshiba Amature Radio Station
//      JK1MLY:Hidekazu Inaba
//  

// 7Segment Digital Clock for PIC16F1827
//  (C)2017 JA1YTS,JK1MLY All rights reserved.
// Redistribution and use in source and binary forms, with or without modification, 
// are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, 
// this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation and/or 
// other materials provided with the distribution.

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <pic16f1827.h>         //Include header file

//  chip define
#define _XTAL_FREQ 8000000
#pragma config FOSC = INTOSC
#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select bit (MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
//#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
//#pragma config IOSCFS = 4MHZ    // Internal Oscillator Frequency Select (4 MHz)
#pragma config BOREN = OFF       // Brown-out Reset Selection bits (BOR enabled)
//#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
//#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
//#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)
//#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
//#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
//#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
//#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
//#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)
#pragma config LVP = OFF

//  local define
#define COL1ON PORTB=0b11111011
#define COL2ON PORTB=0b11110111
#define COL3ON PORTB=0b11011111
#define COL4ON PORTB=0b11101111
#define COLOFF PORTB=0b11111111
#define SW1BIT (PORTB & (1<<0))
#define SW2BIT (PORTB & (1<<1))
#define DIM3M 180               //  Turn off LED
#define WAIT_ON __delay_ms(5)    //  Dynamic Drive
#define WAIT_SW __delay_ms(300)    //  Safty

//  Global Val
long    sec = 0;        //  Stopwatch
short   hold = 0;       //  Hold flag
short   ss = 0;         //  Seconds
short   mm = 0;         //  Minutes
short   hh = 0;         //  Hours
short   sav = 0;        //  Power Save
short   mode = 0;       //  Operation Mode

//  LED pattern
short   row_on[11] = {
//        de fbacg
        0b11011110,        //0
        0b00010010,        //1
        0b11010101,        //2
        0b10010111,        //3
        0b00011011,        //4
        0b10001111,        //5
        0b11001111,        //6
        0b00011110,        //7
        0b11011111,        //8
        0b10011111,        //9
        0b00000000         //OFF
};

//  Interruput for 1sec
void interrupt tim1(){
    if(TMR1IF == 1){
        TMR1H = 128;    //  ONLY TMR1H

        if(hold == 0){
            sec++;          //  for Stopwatch
            if(sec >= 10000){
                sec = 0;
                mode = 4;   // return normal
            }
        }

        if(mode != 5){
            sav++;          //  dimming LED
            if(sav >= DIM3M){
                sav = DIM3M ;
                if(mode != 0){
                    mode = 4;   // return normal
                } else {
                    if(hold != 0){
                        mode = 4;
                        hold = 0;
                    }
                }
            }
        }
        
        ss++;           //  seconds
        if(ss >= 60){
            ss = 0;
            mm++;       //  minutes
            if(mm >= 60){
                mm = 0;
                hh++;   //  hours
                if(hh >= 24){
                    hh = 0;
                }
            }
        }
        TMR1IF = 0;     //  Clear Flag
    }
    return;
}

//  Initialize Reg.
void init(){
    T1CON = 0b10001001;         //
    ANSELA = 0b00000000;        //
    ANSELB = 0b00000000;        //
    TRISA = 0b00000000;         //
    TRISB = 0b11000011;         //
    PORTB = 0b00000000;         //Turn ON LED
    PORTA = 0b11111111;         //
    CPSCON0 = 0b00000000;       //
    MDSRC = 0b10000000;         //
    MDMSODIS = 1;               //
    MDCON = 0b00000000;         //
    __delay_ms(100);
    OSCCON = 0b01110010;        //INT8M
    PORTB = 0b11111111;         //Turn OFF LED
    WPUB = 0b00000011;          //Pullup RB0,1
    nWPUEN = 0;                 //Pullup Enable

//  Clear Counters
    sec = 0;
    hh = 0;
    mm = 0;
    ss = 0;
    sav = 0;
    mode = 0;

    return;
}

//  Display LED
void disp(short col1, short col2, short col3, short col4){
    for(short lp=0; lp<3; lp++){
        PORTA = row_on[ col1 ] ;
//        COLOFF ;
        COL1ON ;
        WAIT_ON ;
//        COLOFF ;

        PORTA = row_on[ col2 ];
        COLOFF ;
        COL2ON ;
        WAIT_ON ;
//        COLOFF ;

        PORTA = row_on[ col3 ];
        COLOFF ;
        COL3ON ;
        WAIT_ON ;
//        COLOFF ;

        PORTA = row_on[ col4 ];
        COLOFF ;
        COL4ON ;
        WAIT_ON ;
        COLOFF ;
    }
    return;
}


//  Check LED pattern
void chk(){
    for(short lp=0; lp<10; lp++){
        for(short cnt=0; cnt<3; cnt++){
            disp(lp, 10, 10, 10);
            WAIT_ON ;
        }

        for(short cnt=0; cnt<3; cnt++){
            disp(10, lp, 10, 10);
            WAIT_ON ;
        }

        for(short cnt=0; cnt<3; cnt++){
            disp(10, 10, lp, 10);
            WAIT_ON ;
        }

        for(short cnt=0; cnt<3; cnt++){
           disp(10, 10, 10, lp);
            WAIT_ON ;
        }
            __delay_ms(200);
    }
    return;
}


int main() {
    short col1, col2, col3, col4;   //  Column
                
    init();         //  Initialize
    chk();          //  LED check

//  Interrupt setup
    TMR1 = 32768;   //  1sec
    PEIE = 1;
    TMR1IF = 0;
    GIE = 1;
    TMR1IE = 1;   

//  Infinite loop
    while (1) {
//  Push MODE SW
        if( SW2BIT == 0 ){
            if((sav < DIM3M) || (mode != 4)){    //  Change MODE
                mode++;
                if(mode > 5){
                    mode = 0;
                }
                for(short lp=0; lp<10; lp++){
                    disp(mode, 10, 10, 10);
                }
            }
            sav = 0;            //  DIM counter
            WAIT_SW ;
        }

//  Push SET SW
        if( SW1BIT == 0 ){
            if((sav < DIM3M) || (mode != 4)){    //  Change Counter
                switch(mode){
                    case 0:     //  Stopwatch
                        if(hold != 0){
                            hold = 0;
                            sec = 0;
                            for(int lp = 0 ; lp < 10 ; lp++){
                                disp(0, 0, 0, 0);
                            }
                            sec = 0;
                        } else {
                            hold = 1;
                        }
                        break;
                    case 1:     //  Clear sconds
                        if(ss >= 30){   //  Advance
                            ss = 0 ;
                            mm++ ;
                            if(mm >= 60){
                                mm = 0 ;
                                hh++ ;
                                if(hh >= 24){
                                    hh = 0;
                                }
                            }
                        } else {        //  Back
                            ss = 0;
                        }
                        TMR1H = 128+32; //  Set timer
                        break;
                    case 2:     //  Set minutes
                        mm++ ;
                        if(mm >= 60){
                            mm = 0 ;
                            hh++ ;
                            if(hh >= 24){
                                hh = 0;
                            }
                        }
                        break;
                    case 3:     //  Set Hours
                        hh++ ;
                        if(hh >= 24){
                            hh = 0;
                        }
                        break;
    //                default:
                }
            }
            sav = 0;            //  DIM counter
            WAIT_SW ;
        }

//  Display
        switch(mode){
            case 0:     //  Stopwatch
                col4 = ((sec / 1) % 10);
                col3 = ((sec / 10) % 10);
                col2 = ((sec / 100) % 10);
                col1 = ((sec / 1000) % 10);
                disp(col1, col2, col3, col4);
                break;
            case 1:     //  Seconds
                col4 = ss  % 10 ;
                col3 = ss / 10 ;
                col2 = mm % 10 ;
                col1 = mm / 10 ;
                disp(col1, col2, col3, col4);
                break;
            case 2:     //  Minutes
            case 3:     //  Hours
                col4 = mm % 10 ;
                col3 = mm / 10 ;
                col2 = hh % 10 ;
                col1 = hh / 10 ;
                disp(col1, col2, col3, col4);
                break;
            case 4:     //  Normal
                if(sav < DIM3M){        //  Dimmer
                    col4 = mm % 10 ;
                    col3 = mm / 10 ;
                    col2 = hh % 10 ;
                    col1 = hh / 10 ;
                    disp(col1, col2, col3, col4);
                }
                break;
            case 5:     //  Always
                    col4 = mm % 10 ;
                    col3 = mm / 10 ;
                    col2 = hh % 10 ;
                    col1 = hh / 10 ;
                    disp(col1, col2, col3, col4);
        }
    }
    return 0;
}
