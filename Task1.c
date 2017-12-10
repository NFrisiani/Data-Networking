////////////////////////////////////////////
// Author: Nicolo Frisiani /////////////////       
// Data Networking Lab 2 Task 1 ////////////
///////////////////////////////////////////

//inclusions
#include "i2c.h"
#include <stdio.h>
#include <stdlib.h>
#include <delays.h>
#include "p18f8722.h"
#include "adc.h"

//function declarations
void updateValue(int value, char addr);
int readPot(void);
void config(void);
void displayOnSSD(int value);
int readAddress(char addr);

//useful definitions
#define PB1 PORTJbits.RJ5
#define PB2 PORTBbits.RB0
#define SWITCHES (PORTH>>4 & 0x0F)
#define EDIT PORTCbits.RC2

//global variables
const char displayArray[] = {0x84, 0xF5, 0x4C, 0x64, 0x35, 0x26, 0x06, 0xB4, 0x04, 0x24, 0x14, 0x07, 0x8E, 0x45, 0x0E, 0x1E};
char buffer = 0;
char address = 0xC0;
int val = 0;

char pressed = 0;

void main(void)
{
    config();
    while(1)
    {
        if(EDIT)
        {
            val = readPot();
            buffer = val;

            if(!PB1)
            {
                updateValue(val, address);
                Delay10KTCYx(250);
                Delay10KTCYx(100);
            }

            displayOnSSD(val);
        }
        else
        {
            if(!PB2)
            {
                if(pressed)
                    pressed = 0;
                else
                    pressed = 1;
                Delay10KTCYx(250);
                Delay10KTCYx(100);
            }
            
            if(pressed)
            {
                val = readAddress(address);
            }
            else
            {
                val = buffer;
            }

            displayOnSSD(val);
        }
    }
}


int readAddress(char addr)
{
    int readVal = 0;

    StartI2C1();
    WriteI2C1(0xA0);
    WriteI2C1(0x00);
    WriteI2C1(addr);
    StopI2C1();

    StartI2C1();
    WriteI2C1(0xA1);
    readVal = ReadI2C1();
    StopI2C1();

    return readVal;
}


void updateValue(int value, char addr)
{
    StartI2C1();
    WriteI2C1(0xA0);
    WriteI2C1(0x00);
    WriteI2C1(addr);
    WriteI2C1(value);
    StopI2C1();
}


int readPot(void)
{
    ConvertADC();
    while(BusyADC());
    return ReadADC()>> 2;
}

void config(void)
{
    OpenI2C1(MASTER, SLEW_OFF);
    SSP1ADD = 0x18;

    OpenADC(ADC_FOSC_16 & ADC_RIGHT_JUST & ADC_0_TAD, ADC_CH0 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, 0x0E);
    ADCON1 = 0x0F;
    TRISF = 0x00; 
}

void displayOnSSD(int value)
{
    TRISH = 0xFE;
    LATH = 0xFE;
    LATF = displayArray[(value & 0xF0) >> 4];
    Delay100TCYx(20);
}