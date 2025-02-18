/**
  ******************************************************************************
  * @file   ShiftReg74HC595Chain.h
  * @brief  Header file for the ShiftReg74HC595Chain Library for simplified control of 74HC595 shift registers.
  *
  * @details Additional information is available at https://timodenk.com/blog/shift-register-arduino-library/
  *
  * @author	: Gabriel D. Goldman
  * 
  * @version v1.0.0
  * @date First release: 12/02/2025 
  *       Last update:   12/02/2025 10:50 (GMT+0200)
  * 
  * @copyright Released into the public domain
  *
  ******************************************************************************
  * @attention	This library was developed by Timo Denk and others for personal use only according to their needs.
  * @warning **Use of this library is under your own responsibility**
  ******************************************************************************
*/
#ifndef _SHIFTREG74HC595CHAIN_ESP32_H_
#define _SHIFTREG74HC595CHAIN_ESP32_H_

#include <Arduino.h>
#include <stdint.h>

/**
 * @brief A class for easy 74HCx595 shift register data management.
 * 
 * The class models a 74HCx595 shift register daisy-chain object, from 1 to 256 registers
 * 
 * @attention Altough the number of shift registers connected has no theoretical limits, a clock line sincronization limits the practical number of registers to daisy-chain. Read the specific product data sheet for precise information
 * 
 * @class ShiftReg74HC595Chain
*/
class ShiftReg74HC595Chain {
private:
   uint8_t _ds;
   uint8_t _sh_cp;
   uint8_t _st_cp;
   uint8_t _srQty;

   uint8_t _maxPin{};
   uint8_t* _srArryBuffPtr{nullptr};
public:
   /**
    * @brief Class constructor
    * 
    * Instantiates a ShiftReg74HC595Chain object, a model of a 74HCx595 -or more than one in daisy-chain connection- for an easy transparent bit or multibit state management.
    * 
    * @param ds GPIO pin connected to the DS pin of the 74HC595 to send data output serially to the shift register
    * @param sh_cp GPIO pin connected to the SH_CP pin of the 74HC595 to manage the communication's clock
    * @param st_cp GPIO pin connected to the ST_CP pin of the 74HC595 to set (latch) the Q0~Q7 output pins from the internal buffer register.
    * @param srQty Quantity of shift registers set in daisy-chain configuration.
    * 
    * @attention The array of bytes set to hold the values to set to the shift registers will be arranged sequentially with a pointer to the first byte, meaning the pointer position +0 will hold bits 00~07, the pointer position +1 will hold bits 08~15 and so on, with the lower bit representing the LSb or rightmost bit
    */
   ShiftReg74HC595Chain(uint8_t ds, uint8_t sh_cp, uint8_t st_cp, uint8_t srQty = 1);
   /**
    * @brief Returns the state of the requested pin.
    * 
    * @param pin Pin whose current value is required.
    * 
    * @return The state value of the requested pin, either HIGH (0x01) or LOW (0x00)
    * 
    * @warning The value is retrieved from the object's buffer, not the real chip. If a change tothe buffer was made by using the digitalWriteBuff(const uint8_t, uint8_t) method without updating the shift register pins by using the updShftRgstrs() method, a difference might be expected.
    */
   uint8_t digitalRead(const uint8_t &pin);
   /**
   * @brief Set a specific pin to either HIGH (0x01) or LOW (0x00).
   * 
   * @param pin a positive value indicating which pin to set. The valid range is 0 <= pin <= (srQty*8)-1
   * @param value Value to set for the indicated Pin
   */
   void digitalWrite(const uint8_t pin, const uint8_t value);
   /**
     * @brief Sets all pins of all shift registers to LOW (0x00).
     */
   void digitalWriteAllReset();
   /**
    * @brief Sets all pins of all shift registers to HIGH (0x01).
    */
   void digitalWriteAllSet(); 
   /**
    * @brief Equivalent to void digitalWrite(const uint8_t, const uint8_t), except the buffer is not pushed into the physical shift register.
    * 
    * @param pin Number of bit being modified in the object's buffer
    * @param value The value to be set to the pin, either LOW (0x00) or HIGH (0x01)
    *
    * @attention Should be used in combination with updShftRgstrs(), or the object's buffer an the shift register pins will be uncoordinated. Use with precaution: is usefull to avoid excesive uneeded communications with the shift register, but might generate inconsistencies.
   */
   void digitalWriteBuff(const uint8_t pin, uint8_t value);
   /**
     * @brief Overwrites the object buffer with new content
     * 
     * @param digitalValues uint8_t* pointer to an array with the new content to be set at the shift register/s.
     * @note The method expects the pointer to be set to an array with the same lenght as srQty as it will be copied byte by byte over the object's buffer
     */
    void digitalWriteOver(const uint8_t* &newValues);
   /**
    * @brief Retrieves the pointer to the shift registers buffer.
    * 
    * @return Pointer to the array of uint8_t holding the buffered shift registers values
    *
    * @note The returned array's length is equal to the number of shift registers set in daisy-chain, see uint8_t getSrQty() for information.
    */
   uint8_t* getArryBuffPtr();
   /**
     * @brief Return the greatest valid pin number, according to the quantity of shift registers connected
     * 
     * @return uint8_t maxPin value
     */
   uint8_t getMaxPin();
   /**
     * @brief Return the quantity of shift registers composing the object
     * 
     * The value is passed as a parameter in the class constructor
     * 
     * @return uint8_t The number of shift registers represented by the object
     */
   uint8_t getSrQty();
   /**
    * @brief Updates the shift register pins to the stored object's buffer values.
    *
    * @note This is the function that actually writes data into the 74HC595 shift registers.
   */
   void updShftRgstrs();
/*
   void ShiftReg74HC595Chain::send(uint8_t content);
   void ShiftReg74HC595Chain::send(const uint8_t &segments, const uint8_t &port);
*/
  };

#endif //_SHIFTREGISTER74HC595_ESP32_H_
