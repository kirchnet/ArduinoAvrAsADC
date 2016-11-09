/***********************************************************
 * i2cADC turns your Arduino into an Analog to Digital Converter 
 * (ADC) in I2C slave mode for a Raspberry Pi or other master.
 * 
 * The program on the Raspberry Pi side is i2cADC.cpp
 * Please also review the comments in the Raspi file i2cADC.cpp
 * 
 * The Arduino Uno and any standalone ATMEGA 328 chip have six
 * ADC channels, of which four can be used with the I2C interface.
 * Two analog lines are used for i2c communication (A4, A5 on Uno).
 * Leonardo, Mini and other Arduinos have additional channels. 
 * To port this to the Leonardo the timer will have to be modified. 
 * For all others this code may or may not work. It occupies under
 * 2,400 bytes and hence might even run on an ATTINY44, where
 * 6 channels would be available when the I2C is used. For an even 
 * smaller code footprint try using the SPI ADC.
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!                         !!WARNING!!                        !!   
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!                                                            !!                                                        !!
!! WARNING: DO NOT CONNECT a 5V Arduino directly to the       !!
!! Raspberry Pi. This will destroy your Raspberry Pi.         !!
!! YOU MUST USE A 5V-to-3.3V LEVEL SHIFTER.                   !!
!! I shall not be liable for any damage to your equipment     !!
!!                                                            !!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 ***********************************************************/
//Use channels 0 through MAXCHANNEL
#define MAXCHANNEL 3
#define I2C_ADDRESS 0x02  // i2c bus address #2. Use the same address in the Raspberry Pi code

#include <Wire.h>


//int values[MAXCHANNEL]={0};
byte upperBytes[4]={0,0,0,0};
byte lowerBytes[4]={0,0,0,0};
byte Command = 255;
int ISRchannelADC=0;

void setup()
{
  Wire.begin(I2C_ADDRESS);         // join i2c bus with address I2C_ADDRESS
  Wire.onReceive(requestCommand);  // register event
  Wire.onRequest(Respond);         // Perform on master's request
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);  //Adjust as a function of MAXCHANNEL
  pinMode(A2, INPUT);  //Adjust as a function of MAXCHANNEL
  pinMode(A3, INPUT);  //Adjust as a function of MAXCHANNEL
  //Note that A4 and A5 are used by i2c
  
  /***********************************************************
   * Set up ADC
   ***********************************************************/
   /* Disable digital buffers on analog pins to reduce noise */
   bitSet (DIDR0, ADC0D);  // disable digital buffer on A0
   bitSet (DIDR0, ADC1D);  // disable digital buffer on A1. Adjust as a function of MAXCHANNEL
   bitSet (DIDR0, ADC2D);  // disable digital buffer on A2. Adjust as a function of MAXCHANNEL
   bitSet (DIDR0, ADC3D);  // disable digital buffer on A3. Adjust as a function of MAXCHANNEL
   //Note that A4 and A5 are used by i2c

  /***********************************************************
   * Set prescaler to sample at 125 kHz (if chip runs at 16MHz)
   * Reference voltage to AVCC
   * Single conversion mode
   ***********************************************************/
   ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);  //Prescaler=128
   ADMUX |= (1 << REFS0);  // AVCC is  reference voltage 
   
   ADCSRA |= (1 << ADEN);  // Enable ADC
   ADMUX = (B01000000 | ISRchannelADC);   // Set channel for initial conversion, Vcc reference

   // Set up a timer 2 interrupt every 1ms to kick off ADC conversions etc.
   // Do this last after we have initialized everything else
   ASSR = 0;
   TCCR2A = (1 << WGM21);    // CTC mode
   TCCR2B = (1 << CS22);     // prescaler = 64
   //TCCR2B |=  (1 << CS22) | (1 << CS21) | (1 << CS20);
   TCNT2 = 0;                // restart counter
   OCR2A = 249;              // compare register = 249, will be reached after 1ms
   TIMSK2 = (1 << OCIE2A);
}  

// Interrupt service routine for the 1ms tick
ISR(TIMER2_COMPA_vect){
 ADCSRA = B11001111;   // ADC enable, ADC start, manual trigger mode, ADC interrupt enable, prescaler = 128
}

/******************************************************
* The ADC ISR is called when the ADC is done digitizing
*******************************************************/
ISR(ADC_vect) {
  cli();
  lowerBytes[ISRchannelADC] =  ADCL;//get value from analog pin. Order: Low byte first
  upperBytes[ISRchannelADC] = ADCH;//High byte must be read second to be accurate 
  if(ISRchannelADC>=MAXCHANNEL) ISRchannelADC=0; else ISRchannelADC++;
  ADMUX = (B01000000 | ISRchannelADC);   // Select channel for next conversion, Vcc reference
  sei();
}

void loop()
{
/***********************************************************
* Your code goes here (if any)
***********************************************************/
 }

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void requestCommand(){ 
  Command = Wire.read();
}
void Respond(){
  int returnValue = 0;
  uint8_t buffer[2];              // split integer return value into two bytes buffer
 
  if(Command==255){
     // No new command was received
      Wire.write("NA");
      return;
  } ;
  buffer[0] = upperBytes[Command];//returnValue >> 8;
  buffer[1] = lowerBytes[Command];//returnValue & 0xff;
  Wire.write(buffer, 2);          // return slave's response to last command
  Command = 255;          // null last Master's command and wait for next
 
}
