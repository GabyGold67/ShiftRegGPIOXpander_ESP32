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
  * @attention	This library was developed by Timo Denk and others for personal use only according to their needs.
  * 
  * @warning **Use of this library is under your own responsibility**
  ******************************************************************************
*/

#pragma once
#include <Arduino.h>

template<uint8_t Size>
class ShiftRegister74HC595 
{
private:
    uint8_t _serialDataPin;
    uint8_t _clockPin;
    uint8_t _latchPin;

    uint8_t  _digitalValues[Size];
public:
    ShiftRegister74HC595(const uint8_t serialDataPin, const uint8_t clockPin, const uint8_t latchPin);
    
    uint8_t get(const uint8_t pin);
    uint8_t* getAll(); 
    void set(const uint8_t pin, const uint8_t value);
    void setAll(const uint8_t * digitalValues);
    void setAllHigh(); 
    void setAllLow();
    void setNoUpdate(const uint8_t pin, uint8_t value);
    void updateRegisters();
};

#include "ShiftRegister74HC595.hpp"
