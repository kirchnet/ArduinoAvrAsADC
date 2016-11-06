/***********************************************************
 * SPIadc turns your Arduino into an Analog to Digital Converter 
 * (ADC) in SPI slave mode for a Raspberry Pi or other master.
 * 
 * The program on the Raspberry Pi side is SPIadc.cpp
 * Please also review the comments in the Raspi file SPIadc.cpp
 * 
 * The Arduino Uno and any standalone ATMEGA 328 chip have six
 * ADC channels, all of which can be used with the SPI interface.
 * Leonardo, Mini and other Arduinos have additional channels. 
 * To port this to the Leonardo the timer will have to be modified. 
 * For all others this code may or may not work. It occupies about
 * 1,100 bytes and hence might even run on an ATTINY24, where
 * 6 channels would be available when the SPI is used.
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
!!                                                            !!                                                          !!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 ***********************************************************/
//Use channels 0 through MAXCHANNEL
#define MAXCHANNEL 5

unsigned char handshake;
unsigned char analogChannel;
byte SPIpointer = 0;
byte upperBytes[6]={0,0,0,0,0,0};
byte lowerBytes[6]={0,0,0,0,0,0};
int ISRchannelADC=0;

void setup (void)
{
  /***********************************************************
   * Set SPI to slave mode by defining MISO pin as output and by 
   * enabling the SPI configuration register 
   ***********************************************************/
  pinMode(MISO, OUTPUT);
  SPCR |= _BV(SPE);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);//Adjust as a function of MAXCHANNEL
  pinMode(A2, INPUT);//Adjust as a function of MAXCHANNEL
  pinMode(A3, INPUT);//Adjust as a function of MAXCHANNEL
  pinMode(A4, INPUT);//Adjust as a function of MAXCHANNEL
  pinMode(A5, INPUT);//Adjust as a function of MAXCHANNEL

  /***********************************************************
   * Set up ADC
   ***********************************************************/
   /* Disable digital buffers on analog pins to reduce noise */
   bitSet (DIDR0, ADC0D);  // disable digital buffer on A0
   bitSet (DIDR0, ADC1D);  // disable digital buffer on A1. Adjust as a function of MAXCHANNEL
   bitSet (DIDR0, ADC2D);  // disable digital buffer on A2. Adjust as a function of MAXCHANNEL
   bitSet (DIDR0, ADC3D);  // disable digital buffer on A3. Adjust as a function of MAXCHANNEL
   bitSet (DIDR0, ADC4D);  // disable digital buffer on A4. Adjust as a function of MAXCHANNEL
   bitSet (DIDR0, ADC5D);  // disable digital buffer on A5. Adjust as a function of MAXCHANNEL
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

void loop (void){
  
  /********************************************
   * Continuously loop through the SPI register 
   * to see if request received from master
   ********************************************/
  if((SPSR & (1 << SPIF)) != 0){
    SPItransceiver();
   }
}

/***************************************************************   
 *    SPI transceiver. 
 *    * Start with handshake. First byte received from master 
 *      should be 'r' to request a new transfer. 
 *    * Return 'a' to acknowledge.
 *    * Next byte received is the number of the A/D channel that 
 *      the master requests data for. This is transmitted as a char
 *      so that ASCII '1' means 1. Subtract 48 to get the channel 
 *      number as an int. 
 *    * The upper and lower bytes are transmitted back to the master. 
 *      If you need 8 bits only instead of 10 you can eliminate the 
 *      last step and finish data acquisition faster.
****************************************************************/
void SPItransceiver(){
  
  if(SPIpointer==0){
    //Handshake to initiate transfer
    handshake = SPDR;
    if (handshake == 'r') //Master sends 'r'eqest
    {
      SPDR = 'a'; // Slave 'a'cknowledges
      SPIpointer++;
    } return;
  }
  if( (SPIpointer==1) ) {
    analogChannel = SPDR; //Master sends channel number 
    SPIpointer++;
    return;
  }
  if( SPIpointer==2) {
    //The next two transfers back to master are the two bytes with the ADC value
    //Least significant byte is transmitted first
    SPDR = lowerBytes[analogChannel-48];
    SPIpointer++;
    return;
  }
  if( SPIpointer==3) {
    //Most significant byte is transmitted second
    SPDR = upperBytes[analogChannel-48];
    SPIpointer=0;
    return;
  }
}
