/**********************************************************
* SPI ADC
* Use Arduino as an Analog to Digital Converter 
* 
* Compile with this command:
* 	g++ -o SPIadc SPIadc.cpp
* Execute with
* 	./SPIadc
* 
* On the Arduino side the corresponding sketch that must be
* loaded to work with this program is SPIadc.ino
* Please also review the comments in the Arduino sketch
* 
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!					!!WARNING!!								!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!															!!
!! WARNING: DO NOT CONNECT a 5V Arduino directly to the     !!
!! Raspberry Pi. This will destroy your Raspi. YOU MUST USE !!
!! A 5V-to-3.3V LEVEL SHIFTER. I shall not be liable for    !!
!! any damage.												!!
!!															!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* 
**************************************************************/

#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <unistd.h>


using namespace std;


int results, device;
unsigned int spiSpeed = 1000000;//SPI speed = 1MkHz

int spiTxRx(unsigned char txDat);
int readADC(char i);


int main (void)
{
	int i=0;

/**********************************************************
 * Start SPI spidev0.0 (chip enable 0 on pin 24)
 * SPI speed as defined above
***********************************************************/

   device = open("/dev/spidev0.0", O_RDWR);
   ioctl (device, SPI_IOC_WR_MAX_SPEED_HZ, &spiSpeed);

/**********************************************************
 * Endless loop through channels 0 to 5. Send channel number 
 * to the Arduino and display the returned ADC value
***********************************************************/
   while (1)
   {

      if(i>5)i=0;
      results = readADC('0'+i);

      cout << "ADC channel "<<i<<" has value: " <<   (int)(results) << endl;


      sleep(1);
      i++;

     }

}

/**********************************************************
 * spiTxRx transmits one byte via the SPI device, and 
 * returns one byte as the result.
 * spi_ioc_transfer is defined in the linked header file 
 * spidev.h and is needed for IOCTL
***********************************************************/

int spiTxRx(unsigned char txDat)
{
 
  unsigned char rxDat;

  struct spi_ioc_transfer spi;

  memset (&spi, 0, sizeof (spi));

  spi.tx_buf        = (unsigned long)&txDat;
  spi.rx_buf        = (unsigned long)&rxDat;
  spi.len           = 1;

  ioctl (device, SPI_IOC_MESSAGE(1), &spi);

  return rxDat;
}


/**********************************************************
 * readADC implements a simple handshake. 
 * Send 'r' to Arduino, which returns 'a' to acknowledge.
 * Next send the ADC channel number. It is sent as an ASCII char
 * The following two bytes returned is the ADC value
***********************************************************/


int readADC(char channel)
{

unsigned char resultByte, upperByte, lowerByte;
bool acknowledged;

/*******************************************
 * wait for success of the initial handshake 
 *******************************************/
  do
  {
    acknowledged = false;

    resultByte = spiTxRx('r');
    usleep (100);


    resultByte = spiTxRx(channel);
    if (resultByte == 'a')
    {
      acknowledged = true;
    }
    usleep (10);  

   }
  while (acknowledged == false);


  resultByte=spiTxRx(channel);//Transmit channel number
  usleep (10);

/**********************************************************
 * Now send any two bytes to receive the two byte result back
***********************************************************/
 
  lowerByte = spiTxRx(0);
  usleep (10);
  upperByte = spiTxRx(0);
  
  return lowerByte+256*upperByte;

}
