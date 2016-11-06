# Arduino or AVR As ADC, polled through SPI
Arduino or AVR as Analog Digital Converter via SPI

Turn your Arduino into an Analog to Digital Converter (ADC) that you can poll via SPI in slave mode. Example code for a Raspberry Pi as master is included.

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

While the 10-bit resolution is competitive with dedicated SPI ADCs the speed is not. If you need a fast ADC a dedicated chip is better. But if you need a quick solution and have an Arduino and level shifter sitting on your bench this can help.
