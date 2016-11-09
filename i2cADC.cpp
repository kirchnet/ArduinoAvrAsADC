/**********************************************************
* i2c ADC
* Use Arduino as an Analog to Digital Converter 
* 
* First you need to install the i2c development package:
*   apt-get install libi2c-dev
* Then zou can compile this file with this command:
* 	g++ -o i2cADC i2cADC.cpp
* Execute with
* 	./i2cADC
* 
* On the Arduino side the corresponding sketch that must be
* loaded to work with this program is i2cADC.ino
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

#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <linux/i2c-dev.h>  //this is why you need libi2c-dev
#include <sys/ioctl.h>
#include <fcntl.h>

#define ADC_ADDRESS (0x02>>1) //The address here must match the one you give the device in the Arduino sketch

using namespace std;


int main(void)
  {
  int device, port, result;
  unsigned long funcs;
  char i, buf[2];

  /**********************************************
   * Open the i2c device
   * "/dev/i2c-1" is the i2c port on GPIO2 and 3,
   * which are physical pins 3 and 5 on the 40-pin
   * connector.
   * "/dev/i2c-0" will use physical pins 27 and 28
   ***********************************************/
  cout << "Opening device...";
  if ((device = open("/dev/i2c-1", O_RDWR)) < 0)
    {
    cout << "Failed to open i2c device" << endl;
    exit (1);
    }
  cout << " OK" << endl;

  /**********************************************
   * Check if i2c functions can be read
   ***********************************************/
  if (ioctl(device,I2C_FUNCS,&funcs) < 0)
    {
    cout << "Reading I2C_FUNCS failed" << endl;
    exit (1);
    }

  /**********************************************
   * Check value reeturned by the previous call
   ***********************************************/
  if (funcs & I2C_FUNC_I2C)
    cout << "Plain i2c level commands available" << endl;
  if (funcs & (I2C_FUNC_SMBUS_BYTE))
    cout << "Can handle 'read byte' and 'write byte' commands" << endl;

  /**********************************************
   * Scan the bus to see if we can find our Arduino
   ***********************************************/
  for (port = 0; port < 127; port++)
  {
	  /**********************************************
	   * Set I2C_SLAVE in order to use read and write
	   ***********************************************/
      if (ioctl(device, I2C_SLAVE, port) < 0)
      cout << "I2C_SLAVE failed  bus access" << endl;
    else
      {
      result = i2c_smbus_read_byte(device);
      if (result >= 0)
        cout << "i2c chip found at address: "<< port <<", value read: "<< result << endl;
      }
    }  

  /**********************************************
   * Set I2C_SLAVE in order to use read and write
   ***********************************************/
  if(ioctl(device,I2C_SLAVE,2)<0) cout << "Failed  bus access/device access" << endl;
  
  buf[1]=0;
  for(i=0; i<=3; i++)
  {
	  buf[0] =i;
	  if(write(device,buf,1)!=1) cout << "Failed write" << endl;
	  usleep(100);
	  if(read(device,buf,2)!=2) cout << "Failed read" << endl;
	  cout << "Channel " << int(i) << ": Upper byte: " << int(buf[0]) << " Lower byte: " << int(buf[1]);
	  cout << " Decimal: " << 256*buf[0]+buf[1] << endl;
	  sleep(1);
   }
  return 0;
  }
