# Shift Registers Manager for GPIO Digital Outputs Expander library for ESP32 (Arduino) (ShiftRegGPIOXpander_ESP32)

For 74HCx595 and compatible SIPO Shift Registers

## [Complete library documentation HERE!](https://gabygold67.github.io/ShiftRegGPIOXpander_ESP32/)

## The main concepts driving this development are:

- Easy output pins addition to a project without making substantial changes to the standard programming practices referring to output pins values writes and reads.  
- Methods and resources are provided to minimize full outputs updating, usually associated with the nature of the GPIO expanders based on SIPO shift registers without preset size limits (change buffering).
- Pin state reading consistency, even for buffered changes.
- Easily extend the number of output pins by adding Shift Register modules and just change an instantiation parameter value.  
- A number of services added to manage the new extra pins:
  - Pin state toggling
  - Setting, resetting and toggling several pins simultaneously by providing a mask.
  - Pins values shifting.
  - Virtual Ports construction and management as independent units for easy devices management, with the required API to provide usual internal ports management.

---  
## Fast Setup:

- Connect as many shift registers as you'll need in daisy chain configuration. 
  - Consider you'll be connecting the first shift register to the MCU, that will take 3 available output pins, and give you 8, making that a net addition of 5 output pins to your resources. 
  - For each following shift register connected to the daisy chain you'll get 8 extra pins.  
- Create a ShiftRegGPIOXpander object using the pin numbers connected to the first shift register of the chain and the quantity of shift registers connected in that daisy chain (the minimum value admitted is 1): `ShiftRegGPIOXpander mySrgx(ds, sh_cp, st_cp, srQty);`
- Initiate the ShiftRegGPIOXpander object: `mySrgx.begin();`
- Use the ShiftRegGPIOXpander object pins as normal MCU pins: `mySrgx.digitalWrite(pinToModify, LOW);` or `mySrgx.digitalWrite(pinToModify, HIGH);`
- The current pin setting might be checked by using `mySrgx.digitalRead(pinToRead);` (remember: all the ShiftRegGPIOXpander pins are set to **Output**)

In short:
```
ShiftRegGPIOXpander mySrgx(ds, sh_cp, st_cp, srQty);
mySrgx.begin();
mySrgx.digitalWrite(pinToModify, HIGH);
```

You then have a plethora of methods to use for managing the **ShiftRegGPIOXpander** pins. [Check the complete library documentation CLICKING HERE!](https://gabygold67.github.io/ShiftRegGPIOXpander_ESP32/)

---  

The library provides tools to generate "animations", coordinated pin changes, buffered changes, etc. For those actions the following tools are provided:

- Auxiliary buffer
- Mask setup for set, reset and flip operations.

- Pinshifting methods: Standard shifting, Rottate shifting, Arithmetic shifting for the whole set of pins, and for a segment of the pins
- Instant buffer output invertion
