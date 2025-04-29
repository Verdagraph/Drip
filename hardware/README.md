# PCB Design

https://www.electronics-tutorials.ws/blog/relay-switch-circuit.html
https://www.digikey.ca/en/blog/logic-level-shifting-basics  


There's two main type of pressure transducers: ones that have two wires and output a current, and ones that have 3 wires and output a voltage.
We can use the first type by adding a resistor with a known value to the path. Because we know the sensor is outputting a given current, and we know that the resistor must dissipate all the voltage before it as its connected to ground, we can take the voltage before the resistor and connect it to our ADC for a measurements.
The resistor is commonly 250 ohms which translates to 1-5V. For the voltage type, it is as simple as connecting the voltage to the ADC. For the current type, we measure the voltcage through a 10k ohm resistor, and remember that the 250 ohm resistor must sink the power (THT). We use a 220 ohm to give a little margin for error.

The reset button is connected to the CHIP_PU or chip power up pin on the ESP. It is pulled up by default, enabling the MCU. When the button is pressed it is pulled to GND, with a small filtering capacitor to smooth it out, and the device resets.

The boot button is connectod to GPIO9. When GPIO0 is pulled high, the chip will boot into its saved program. When its pulled low, it will enter into the USB/UART boot mode. This person (https://www.instructables.com/Build-Custom-ESP32-Boards-From-Scratch-the-Complet/) also added a pullup here and found it reduced boot errors.

The RGB in use is the WS2812B-2020, but this can be any addressable RGB LED. Its recommended to connect vdd to 5v, but it also works at 3.3v and since this is jsut an indicator light its fine like this.

The USBC receptable connects the positive and negative data terminals on either side together and shunts them along with the USB bus voltage with zener diodes for ESD protection and the schtokky diode on the voltage line for reverse current protection, along with a filtering capacitor. THe CC1 and CC2 lines are pulled down for USBC functinoality

To the microcontroller: the power line is again filtered so that the voltage regulators can be farther from the rest. The chip_pu is pulled up again and filtered. The USB data lines are filtered again as well. There is a common bug in the devkits where because the CHIP_EN pin is used to reset the chip, and the GPIO9 pin is used to select the boot mode, the GPIO9 pin must be high at least 3ms before the CHIP_EN pin to avoid puttin the board in boot. To accomplish this, we add a pullup to GPIO9 to get around the weak internal pullup. This works because the GPIO9 is pulled up to the 3v3 output of the regulator, but the CHIP_EN is only brouhgut up once the ESP's 3v3 rail stabalizes, and it has extra filtering capacitors so it takes longer.

Our 12v-5v regulator is a switching buck converter LM2575, the fixed 5V output version. According to the typical application circuit, it has a filtering capacitor on the input, an LC filter on the output. There is also a schtokky diode to ground which allows a path for the inductor transient response to dissipate. To choose the L we follow the datasheet. Vin - Vout = 12-5 = 7. Vin - Vout * Vout / Vin * 1000 / 52 = 56V x us. We use the figure 12 to find the intersection of this and 1A means 220uH is  good. To choose output cap we use the formula in the datasheet which yields 56 uF. This is only for stability though. For ripple We round up to 330. 

the 5V to 3.3V regulator is the AMS1117 linear regulator and it has the same 2 capacitor filter on either end with a power LED.

The relays. These are 5 terminal 5V relay modules. The coils need 5V and 360mw to turn on so they need .36 / 5 = 72ma to run. Except the CE has voltage at saturation so we factor in a 0.7V drop. We also double the minimum current we need to make sure it works which gives us 167mA. We need to drive it with a transistor to get the 5V signal. The relay has a flyback diode. We use the BC337 but assume the transistor to have abeta of at least 100 which will cover common transistors at these conditions. This gives us a base current of 1.67mA. Given we're using 3.3v to run this that means the voltage across the resistor must be (3.3V - VBE) = 3.3 - 1.2 (which is the max BE voltage on the BC337 and 2n2222a) = 2.1V. R = V / I = 2.1V / 1.67ma = 1254ohm rounded to 1000.

The flow sensor: we use the TX20102 logic level shifter to go between the 5V pulses of the flow sensor and the 3.3V logic of the ESP. The enable pin is pulled down and connected to a GPIO so we can disable the pulses into the microcontroller when we don't care about the flow. We also have 0.1uF filters to stabailize the reference voltages as much as possible.

The ADC is the ADC088S022, as the ESP ADC is not reliable. This has both the 5V and 3.3V lines as the analog and digital power lines with filtering. The I2C is used to communicate with the ESP, while the power lines are filted as well. The max that we can go on any input is the analog supply + 0.3V, meaning we shouldn't make it go beyond 5.3V


https://www.aliexpress.com/item/1005006568129795.html?spm=a2g0o.productlist.main.25.45f354a8S2jKQZ&algo_pvid=02c4b3cc-5f71-485b-a4de-3bd9254a7f95&algo_exp_id=02c4b3cc-5f71-485b-a4de-3bd9254a7f95-24&pdp_ext_f=%7B%22order%22%3A%2298%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21CAD%2150.61%2146.85%21%21%2135.71%2133.06%21%402101eac917449551732753582e5d94%2112000037696338130%21sea%21CA%210%21ABX&curPageLogUid=sR35G4mEGZbu&utparam-url=scene%3Asearch%7Cquery_from%3A