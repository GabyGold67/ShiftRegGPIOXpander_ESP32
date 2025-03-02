# GPIO Digital Outputs Expander Library (ShiftRegGPIOXpander_ESP32)  

## The main concepts driving this development are:
   - Output pins addition to a project without making substantial changes to the standard programming practices referring to output pin values writes and reads.
   - Methods and resources are provided to minimize full outputs updating, usually associated with the nature of the GPIO expanders based on SIPO shift registers whithout preset size limits (changes buffering).
   - Pin state reading consistency, even for buffered changes.
   - Easy addition of Shift Registers modules by just changing a instantiation parameter.