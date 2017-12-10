////////////////////////////////////////////
// Author: Nicolo Frisiani /////////////////       
// Data Networking Lab 2 Task 2 Part A /////
////////////////////////////////////////////

//inclusions
#include "i2c.h"
#include <stdio.h>
#include <stdlib.h>
#include <delays.h>
#include "p18f8722.h"
#include "adc.h"

//function declarations
void updateValue(char addr);
int readPot(void);
void config(void);
void displayOnSSD(int value);
void readValue(char addr);

//useful definitions
#define PB1 PORTJbits.RJ5
#define PB2 PORTBbits.RB0
#define SWITCHES (PORTH>>4 & 0x03)
#define EDIT PORTCbits.RC2

// global variables
const char displayArray[] = {0x84, 0xF5, 0x4C, 0x64, 0x35, 0x26, 0x06, 0xB4, 0x04, 0x24, 0x14, 0x07, 0x8E, 0x45, 0x0E, 0x1E};
unsigned char buffer[4] = {0, 0, 0, 0};
char address = 0x0D4; 
int val = 0;
int index = 0;


//main method
void main(void)
{
    config(); 
    while(1)
    {
        if(EDIT) //check for EDIT switch
        {
            val = readPot(); //Read value from potentiometer
            if(!PB1) //if PB1 is pressed
            {
                buffer[index] = val; //fill the buffer with the value read

                index++; //index is incremented on every click on PB1
                if(index == 4) //if index is over array size bring it back to 0
                    index = 0;

                Delay10KTCYx(250); //debounce PB1
                Delay10KTCYx(100);
            }

            displayOnSSD(val); //display value on the SSD
        }
        else  //if not EDIT (READ)
        {
            if(!PB1) //if PB1 is pressed
            {
                updateValue(address); //Write buffer values in EEPROM starting from location 'address'

                //empty buffer (for loops are giving problems with C18 compiler)
                buffer[0] = 0;
                buffer[1] = 0;
                buffer[2] = 0;
                buffer[3] = 0;

                Delay10KTCYx(250); //debounce PB1
                Delay10KTCYx(100);
            }

            if(!PB2) //if PB2 is pressed
            {
                readValue(address); //read values from EEPROM starting from 'address' and store them in buffer
                
                Delay10KTCYx(250); //debounce PB2
                Delay10KTCYx(100);
            }

            displayOnSSD(buffer[SWITCHES]); //display content of buffer on SSD
        }
    }
}

//Reads values from EEPROM and stores them into buffer
void readValue(char addr)
{
    //setting the address pointer in the EEPROM IC
    StartI2C1(); //start I2C connection
    WriteI2C1(0xA0); //send control bit
    WriteI2C1(0x00); //MSB of address
    WriteI2C1(addr); //LSB of address
    StopI2C1(); //stop I2C connection

    
    StartI2C1(); //start I2C connection
    WriteI2C1(0xA1); //send control bit (LSB set for read operation)
    
    //sequentially read values from EEPROM and store them in buffer
    buffer[0] = ReadI2C1();
    AckI2C1(); //Wait for ack after every read operation
    buffer[1] = ReadI2C1();
    AckI2C1();
    buffer[2] = ReadI2C1();
    AckI2C1();
    buffer[3] = ReadI2C1();
    StopI2C1(); //stop I2C connection
}

//Writes values from buffer to EEPROM
void updateValue(char addr)
{
    StartI2C1(); //start I2C connection
    WriteI2C1(0xA0); //send control bit
    WriteI2C1(0x00); //MSB of address
    WriteI2C1(addr); //LSB of address
    
    //page write all buffer values to subsequent addresses
    WriteI2C1(buffer[0]); 
    WriteI2C1(buffer[1]);
    WriteI2C1(buffer[2]);
    WriteI2C1(buffer[3]);
    StopI2C1(); //stop I2C connection
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

//Displaying the integer "value" from the potentiometer on the right SSD
void displayOnSSD(int value)
{
    TRISH = 0xFE;
    LATH = 0xFE;
    LATF = displayArray[(value & 0xF0) >> 4];
    Delay100TCYx(20);
}
