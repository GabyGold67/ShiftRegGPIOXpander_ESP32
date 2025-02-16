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

#pragma once
#include <Arduino.h>
/**
 * @brief A template class for easy 74HC595/74HCT595 shift register data management.
 * 
 * @tparam Size Quantity of registers set in cascade configuration.
 * @attention When more than one shift register is set as configuration they will be arranged as an array of 8 bits values (uint8_t, unsigned short int, char, byte) and not as a sequence of tparam*8 bits
 * @class ShiftRegister74HC595
*/
template<uint8_t Size>
class ShiftRegister74HC595 
{
private:
    uint8_t _serialDataPin;
    uint8_t _clockPin;
    uint8_t _latchPin;

    uint8_t  _digitalValues[Size];
public:
   /**
    * @brief Class constructor
    * 
    * @param serialDataPin GPIO pin connected to the SDA pin of the 74HC595 to send output data serially
    * @param clockPin GPIO pin connected to the CP pin of the 74HC595 to manage the communication's clock
    * @param latchPin GPIO pin connected to the LP pin of the 74HC595 to set (latch) the Q0~Q7 output pins to the loaded register value
    */
   ShiftRegister74HC595(const uint8_t serialDataPin, const uint8_t clockPin, const uint8_t latchPin);
   /**
    * @brief Returns the state of the requested pin.
    * 
    * @param pin Pin whose current value is required
    * @return uint8_t The state value of the requested pin, either HIGH (1) or LOW (0)
    */
   uint8_t get(const uint8_t pin);
   /**
    * @brief Retrieve all states of the shift registers' output pins.
    * 
    * @return Pointer to the array of uint8_t holding the registers values
    *
    * @note The returned array's length is equal to the number of shift registers.
    */
   uint8_t* getAll();
   /**
   * @brief Set a specific pin to either HIGH (1) or LOW (0).
   * 
   * @param pin a positive, zero-based integer, indicating which pin to set.
   * @param value Value for the indicated Pin
   */
    void set(const uint8_t pin, const uint8_t value);
    /**
     * @brief Set all pins of the shift registers at once.
     * 
     * @param digitalValues uint8_t array where the length is equal to the number of shift registers.
     */
    void setAll(const uint8_t * digitalValues);
    /**
     * @brief Sets all pins of all shift registers to HIGH (1).
     */
    void setAllHigh(); 
    /**
     * @brief Sets all pins of all shift registers to LOW (0).
     * 
     */
    void setAllLow();
   /**
    * @brief Equivalent to set(int pin, uint8_t value), except the physical shift register is not updated.
    * 
    * @param pin 
    * @param value 
    *
    * @attention Should be used in combination with updateRegisters().
   */
   void setNoUpdate(const uint8_t pin, uint8_t value);
   /**
    * @brief Updates the shift register pins to the stored output values.
    *
    * @note This is the function that actually writes data into the shift registers of the 74HC595.
   */
  void updateRegisters();
};

#include "ShiftRegister74HC595.hpp"
