# Arduino or AVR As ADC, polled through SPI or I2C
Arduino or AVR as Analog Digital Converter via SPI or I2C

Turn your Arduino into an Analog to Digital Converter (ADC) that you can poll via SPI or I2C in slave mode. Example code for a Raspberry Pi as master is included.

<pre>!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!                         !!WARNING!!                        !!   
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!                                                            !!
!! WARNING: DO NOT CONNECT a 5V Arduino directly to the       !!
!! Raspberry Pi. This will destroy your Raspberry Pi.         !!
!! YOU MUST USE A 5V-to-3.3V LEVEL SHIFTER.                   !!
!! I shall not be liable for any damage to your equipment     !!
!!                                                            !!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!</pre>

While the 10-bit resolution is competitive with dedicated SPI and I2C ADCs the speed is not. If you need a fast ADC a dedicated chip is better. But if you need a quick solution and have an Arduino and level shifter sitting on your bench this can help.

One advantage of using an AVR/Arduino in lieu of a dedicated ADC chip is the ability to include thresholds and other logic in the AD converter.  

# SPI: Arduino as SPI Analog to Digital Converter

![alt tag] (https://github.com/kirchnet/ArduinoAvrAsADC/blob/master/SPI%20Arduino%20Raspi%20Connection_bb.jpg)

Files: Arduino code in SPIadc.ino, Raspberry Pi code in SPIadc.cpp

On the Raspi side you need to enable SPI before running the code. 
To enable SPI:
<ul>  <li>In a terminal window run <code>sudo raspi-config</code> </li> 
 <li>  Go to Advanced Options<br> </li> 
 <li>  Select SPI<br> </li> 
 <li>  Confirm YES and hit OK<br> </li> </ul>
  
All six channels can be used for AD conversion. The Arduino/AVR operates in SPI slave mode. It loops through all six channels and converts the input analog values, which are stored in an array. When the master needs a value it polls the Arduino/AVR requesting the channel for which it needs the analog value. The Arduino/AVR slave returns the value from the array.
The code uses two interrupts:
1. A timer to launch the AD conversion for a channel. This timer fires every millisecond (can be configured to fire faster) so that, using one channel, the frequency is 1kHz. For all six channels the frequency is 167Hz. For higher frequency you need to adjust the timer settings.
2. When the AD conversion is finished a second interrupt is fired that captures the value from the ADC register and stores it in the array.

# I2C: Arduino as I2C Analog to Digital Converter

![alt tag] (https://github.com/kirchnet/ArduinoAvrAsADC/blob/master/I2C%20Arduino%20Raspi%20Connection_bb.jpg)

Files: Arduino code in i2cADC.ino, Raspberry Pi code in i2cADC.cpp

On the Raspi side you need to enable I2C before running the code. 
To enable I2C:<br>
<ul>  <li>In a terminal window run <code>sudo raspi-config</code><br> </li>
  <li>Go to Advanced Options<br> </li>
  <li>Select I2C<br> </li>
  <li>Confirm YES and hit OK<br> </li> </ul>
In addition to enabling I2C you also need to install libi2c-dev
<code>sudo apt-get install libi2c-dev</code><br>

In I2C mode only four ADC channels can be used becaus unfortunately two analog pins are used by the IDC interface.

On the Arduino side the code for the AD conversion is identical to what is described above in the SPI section.
