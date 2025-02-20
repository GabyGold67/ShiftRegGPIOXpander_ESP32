/**
 * @file ShiftRegGPIOExtend.h
 * @brief Header file for the ShiftRegGPIOExtend_ESP32 library 
 * 
 * @details The library provides the means to extend the GPIO available pins -for digital output only- by providing a pin output manipulation API similar to the provided by Arduino for shift registers attached to the controller. The class and related definitions are provided for 74HCx595 shift registers connected to the MCU by the required three pins the first chip, daisy-chained to other similar chips as much as needed and technically supported (please read the datasheets of the selected model for references about those limits).
 * 
 * @author Gabriel D. Goldman
 * 
 * @version 1.0.0
 * 
 * @date First release: 12/02/2025 
 *       Last update:   20/02/2025 16:30 (GMT+0200)
 * 
 * @copyright Copyright (c) 2025
 *******************************************************************************
 * @attention 
 * @warning 
 *******************************************************************************
 */

#ifndef _ShiftRegGPIOXtender_ESP32_H_
#define _ShiftRegGPIOXtender_ESP32_H_

#include <Arduino.h>

/**
 * @brief 
 * 
 * @class ShiftRegGPIOXtender
 */
class ShiftRegGPIOXtender{
private:
   uint8_t _ds{};
   uint8_t _sh_cp{};
   uint8_t _st_cp{};
   const uint8_t _srQty{};

   uint8_t _maxPin{0};
   // uint8_t* _srArryBuffPtr{nullptr};
   static uint8_t* _srArryBuffPtr;

   uint8_t* _auxArryBuffPtr{nullptr};
   bool _mainBuffBlckd{false};

   /**
    * @brief 
    * 
    * @param data 
    * 
    * @return 
    */
   bool _sendSnglSRCntnt(uint8_t data);
public:
   /**
    * @brief Class default constructor
    * 
    */
   ShiftRegGPIOXtender();
   /**
    * @brief Class constructor
    * 
    * Instantiates a ShiftRegGPIOXtender object, a model of a GPIO pin expansion that adds digital output pins managed with an API similar to the built in platform tools. As the hardware is built using daisy-chained 74HCx595 shift registers, the connection parameters to the hardware is needed to build the object
    * 
    * @param ds MCU GPIO pin connected to the DS pin -a.k.a. serial data input (DIO)- pin of the 74HCx595 to send data serially to the expander
    * @param sh_cp MCU GPIO pin connected to the SH_CP pin -a.k.a. shift register clock input- of the 74HCx595 to manage the communication's clock line to the expander
    * @param st_cp MCU GPIO pin connected to the ST_CP pin -a.k.a. storage register clock input- to set (latch) the output pins from the internal buffer of the expander
    * @param srQty Quantity of shift registers set in daisy-chain configuration composing the expander.
    */
   ShiftRegGPIOXtender(uint8_t ds, uint8_t sh_cp, uint8_t st_cp, uint8_t srQty = 1);
   /**
    * @brief Class destructor
    * 
    * Takes care of resources releasing
    * 
    */
   ~ShiftRegGPIOXtender();
   /**
   * @brief Set a specific pin to either HIGH (0x01) or LOW (0x00).
   * 
   * @param pin a positive value indicating which pin to set. The valid range is 0 <= pin <= (srQty*8)-1
   * @param value Value to set for the indicated Pin
   */
  void digitalWrite(const uint8_t pin, const uint8_t value);
  /**
    * @brief Sends the content of the buffered expander output pins state to the hardware pins.
    * 
    * The method will ensure the object buffer is updated -if there are modifications pending in the auxiliary buffer- enable the hardware to receive the information and invoke the needed methods to send serially the information required to each physical shift register.
    * 
    * @return 
    */
   bool sendAllSRCntnt();

};


#endif //ShiftRegGPIOXtender_ESP32_H_

