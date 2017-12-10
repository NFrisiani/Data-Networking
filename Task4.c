////////////////////////////////////////////
// Author: Nicolo Frisiani /////////////////       
// Data Networking Lab 2 Task 2 Part B /////
////////////////////////////////////////////

//inclusions
#include "i2c.h"
#include <stdio.h>
#include <stdlib.h>
#include <delays.h>
#include "p18f8722.h"
#include "adc.h"

//function declaration
void writeValue();
int readPot(void);
void config(void);
void displayOnSSD(int value);
void readValue();

//useful definitions
#define PB1 PORTJbits.RJ5
#define PB2 PORTBbits.RB0
#define SWITCHES (PORTH>>4 & 0x07)
#define EDIT PORTCbits.RC2

//global variables
const char displayArray[] = {0x84, 0xF5, 0x4C, 0x64, 0x35, 0x26, 0x06, 0xB4, 0x04, 0x24, 0x14, 0x07, 0x8E, 0x45, 0x0E, 0x1E};
unsigned char buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
char address = 0x0DA; 
int val = 0;
int index = 0;


void main(void)
{
    config();
    while(1)
    {
        if(EDIT)
        {
            val = readPot(); //Read value from potentiometer
            if(!PB1) //if PB1 is pressed
            {
                buffer[index] = val; //fill the buffer with the value read

                index++; //index is incremented on every click on PB1
                if(index == 8) //if index is over array size bring it back to 0
                    index = 0;

                Delay10KTCYx(250); //debounce PB1
                Delay10KTCYx(100);
            }

            displayOnSSD(val); //display value on the SSD
        }
        else
        {
            if(!PB1) //PB1 is pressed
            {
                writeValue(); //Write values from buffer to EEPROM
                
                //clear buffer (for loops giving problems with C18)
                buffer[0] = 0;
                buffer[1] = 0;
                buffer[2] = 0;
                buffer[3] = 0;
                buffer[4] = 0;
                buffer[5] = 0;
                buffer[6] = 0;
                buffer[7] = 0;
                

                Delay10KTCYx(250); //debounce PB1
                Delay10KTCYx(100);
            }
             
            if(!PB2) //PB2 is pressed
            {
                readValue(); //read values from EEPROM and store them in buffer
                
                Delay10KTCYx(250); //debounce PB2
                Delay10KTCYx(100);
            }

            displayOnSSD(buffer[SWITCHES]);//display buffer content of SSD
        }
    }
}

//Reads values from EEPROM and stores them into buffer
void readValue()
{
    //setting the address pointer in the EEPROM IC
    StartI2C1(); //start I2C connection
    WriteI2C1(0xA0); //send control bit
    WriteI2C1(0x00); //MSB of address
    WriteI2C1(address); //LSB of address
    StopI2C1(); //stop I2C connection

    
    StartI2C1(); //start I2C connection
    WriteI2C1(0xA1); //send control bit (LSB set for read operation)
    
    //reading sequentially the first six value starting from 0x0DA. 
    //The last two values can't be read sequentially as they are stored from 0x00 onwards
    buffer[0] = ReadI2C1();
    AckI2C1();
    buffer[1] = ReadI2C1();
    AckI2C1();
    buffer[2] = ReadI2C1();
    AckI2C1();
    buffer[3] = ReadI2C1();
    AckI2C1();
    buffer[4] = ReadI2C1();
    AckI2C1();
    buffer[5] = ReadI2C1();
    StopI2C1(); //stop I2C connection
    
    //setting the address pointer in the EEPROM IC to 0x00 for the last two values
    StartI2C1(); //start I2C connection
    WriteI2C1(0xA0); //send control bit
    WriteI2C1(0x00); //MSB of address
    WriteI2C1(0x00); //LSB of address
    StopI2C1(); //stop I2C connection
    
    StartI2C1(); //start I2C connection
    WriteI2C1(0xA1); //send control bit (LSB set for read operation)
    
    //reading sequentially the last two value starting from 0x00.
    buffer[6] = ReadI2C1();
    AckI2C1();
    buffer[7] = ReadI2C1();
    StopI2C1();
}

//Writes values from buffer to EEPROM
void writeValue()
{
    //setting the address pointer in the EEPROM IC
    StartI2C1(); //start I2C connection
    WriteI2C1(0xA0); //send control bit
    WriteI2C1(0x00); //MSB of address
    WriteI2C1(address); //LSB of address
    
    //page writing only the first 6 values as the other two don't 'fit' sequentially
    WriteI2C1(buffer[0]);
    WriteI2C1(buffer[1]);
    WriteI2C1(buffer[2]);
    WriteI2C1(buffer[3]);
    WriteI2C1(buffer[4]);
    WriteI2C1(buffer[5]);
    StopI2C1();
    
    //waits for acknolwedge to arrive
    do
    {
        //keeps initiating connection to call for ack until it has arrived
        StartI2C1();
        WriteI2C1(0xA0);
    }while(SSP1CON2bits.ACKSTAT);
    
    StopI2C1(); //stop I2C connection
    
    StartI2C1(); //start I2C connection
    WriteI2C1(0xA0); //send control bit
    WriteI2C1(0x00); //MSB of address
    WriteI2C1(0x00); //LSB of address
    
    //page writing the last two values starting from address 0x00
    WriteI2C1(buffer[6]);
    WriteI2C1(buffer[7]);
    StopI2C1();
}


//reading values from the potentiometer
int readPot(void)
{
    ConvertADC();
    while(BusyADC());
    return ReadADC()>> 2;
}

//configure ADC and I2C settings
void config(void)
{
    OpenI2C1(MASTER, SLEW_OFF);
    SSP1ADD = 0x18; //setting baud rate of the serial link

    OpenADC(ADC_FOSC_16 & ADC_RIGHT_JUST & ADC_0_TAD, ADC_CH0 & ADC_INT_OFF & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, 0x0E);
    ADCON1 = 0x0F;
    TRISF = 0x00; 
}

//Displaying the int "value" from the potentiometer on the right SSD
void displayOnSSD(int value)
{
    TRISH = 0xFE;
    LATH = 0xFE;
    LATF = displayArray[(value & 0xF0) >> 4];
    Delay100TCYx(20);
}
