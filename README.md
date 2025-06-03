# GPIO Digital Outputs Expander Library (ShiftRegGPIOXpander_ESP32)  

## [Complete library documentation HERE!](https://gabygold67.github.io/ShiftRegGPIOXpander_ESP32/)

## The main concepts driving this development are:
   - Easy output pins addition to a project without making substantial changes to the standard programming practices referring to output pin values writes and reads.  
   - Methods and resources are provided to minimize full outputs updating, usually associated with the nature of the GPIO expanders based on SIPO shift registers without preset size limits (changes buffering).
   - Pin state reading consistency, even for buffered changes.
   - Easy extend the number of output pins by adding Shift Registers modules, and just changing a instantiation parameter.  
   - A number of services added to manage the new extra pins:
      - Pin state toggling
      - Setting, resetting and toggling several pins indicated by providing a mask.
      - Virtual Ports construction and management as independent units for easy devices management.  

   **Note:**  
   I will agree with anybody who catalogs this as a **YAUSRL** (Yet Another Unneeded Shift Register Library). The thing is I tried to find an adequate library for my project but I failed to find something I felt comfortable working with. As a matter of fact I forked a repository, and after starting to modify it I ended up scrapping the old base concept and started a new structure, using the old code scraps where it was consistent with the new concept, rewriting the rest. As a matter of fact this repository will still be a fork of the original library for it was a source: of **code** when it fitted my vision, of **inspiration** when I considered it was outside my vision of the solution concept. The original library repository is at [ShiftRegister74HC595](https://github.com/Simsso/ShiftRegister74HC595).