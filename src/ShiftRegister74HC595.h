/**
  ******************************************************************************
  * @file   ShiftRegister74HC595.h
  * @brief  Header file for the ShiftRegister74HC595 Library for simplified control of 74HC595 shift registers.
  *
  * @details Additional information is available at https://timodenk.com/blog/shift-register-arduino-library/
  *
  * @author	: Timo Denk and contributors
  * 
  * @version v
  * @date First release: 11/2014 
  *       Last update:   12/02/2025 10:50 (GMT+0200)
  * 
  * @copyright Released into the public domain
  *
  ******************************************************************************
  * @attention	This library was developed by Timo Denk and others for personal use only according to their needs.  * 
  * @warning **Use of this library is under your own responsibility**
  ******************************************************************************
*/
#ifndef _SHIFTREGISTER74HC595_ESP32_H_
#define _SHIFTREGISTER74HC595_ESP32_H_

#include <Arduino.h>
#include <stdint.h>

/**
 * @brief A class for easy 74HC595/74HCT595 shift register data management.
 * 
 * @class ShiftRegister74HC595
*/
class ShiftRegister74HC595 {
private:
   uint8_t _serialDataPin;
   uint8_t _clockPin;
   uint8_t _latchPin;
   uint8_t _srQty;

   uint8_t _maxPin{};
   uint8_t* _srArryBuffPtr{nullptr};
public:
   /**
    * @brief Class constructor
    * 
    * Instantiates a ShiftRegister74HC595 object, a model of a 74HCx595 -or more than one in daisy-chain connection- for an easy transparent bit or multibit state management.
    * 
    * @param serialDataPin GPIO pin connected to the DS pin of the 74HC595 to send output data serially
    * @param clockPin GPIO pin connected to the SH_CP pin of the 74HC595 to manage the communication's clock
    * @param latchPin GPIO pin connected to the ST_CP pin of the 74HC595 to set (latch) the Q0~Q7 output pins from the internal buffer register.
    * @param srQty Quantity of shift registers set in daisy-chain configuration (cascading)
    * 
    * @attention The array of bytes set to hold the values to set to the shift registers will be arranged sequentially with a pointer to the first byte, meaning the pointer position +0 will hold bits 00~08, the pointer position +1 will hold bits 09~15 and so on, with the lower bit representing the LSb or rightmost bit
    */
   ShiftRegister74HC595(uint8_t serialDataPin, uint8_t clockPin, uint8_t latchPin, uint8_t srQty = 1);
   /**
    * @brief Returns the state of the requested pin.
    * 
    * @param pin Pin whose current value is required
    * @return The state value of the requested pin, either HIGH (0x01) or LOW (0x00)
    * 
    * @note The value is retrieved from the object's buffer, not the real chip
    */
   uint8_t digitalRead(const uint8_t &pin);
   /**
    * @brief Retrieves the states of the all the shift registers' output pins.
    * 
    * @return Pointer to the array of uint8_t holding the buffered shift registers values
    *
    * @note The returned array's length is equal to the number of shift registers set in daisy-chain.
    */
   uint8_t* getArryBuffPtr();
   /**
   * @brief Set a specific pin to either HIGH (0x01) or LOW (0x00).
   * 
   * @param pin a positive, zero-based integer, indicating which pin to set.
   * @param value Value to set for the indicated Pin
   */
    void digitalWrite(const uint8_t pin, const uint8_t value);
    uint8_t getMaxPin();
    uint8_t getSrQty();
    /**
     * @brief Set all pins of the shift registers at once.
     * 
     * @param digitalValues uint8_t array where the length is equal to the number of shift registers.
     */
    void digitalWriteOver(const uint8_t * digitalValues);
    /**
     * @brief Sets all pins of all shift registers to HIGH (1).
     */
    void digitalWriteAllSet(); 
    /**
     * @brief Sets all pins of all shift registers to LOW (0).
     * 
     */
    void digitalWriteAllReset();
   /**
    * @brief Equivalent to set(int pin, uint8_t value), except the physical shift register is not updated.
    * 
    * @param pin 
    * @param value 
    *
    * @attention Should be used in combination with updShftRgstrs().
   */
   void digitalWriteBuff(const uint8_t pin, uint8_t value);
   /**
    * @brief Updates the shift register pins to the stored output values.
    *
    * @note This is the function that actually writes data into the shift registers of the 74HC595.
   */
  void updShftRgstrs();
};

#endif //_SHIFTREGISTER74HC595_ESP32_H_
