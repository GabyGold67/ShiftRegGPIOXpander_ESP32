/**
 ******************************************************************************
 * @file ShiftRegGPIOXpander.h
 * @brief Header file for the ShiftRegGPIOXtender_ESP32 library 
 * 
 * @details The library provides the means to extend the GPIO available pins -for digital output only- by providing a pin output manipulation API similar to the provided by Arduino for shift registers attached to the controller. The class and related definitions are provided for 74HCx595 shift registers connected to the MCU by the required three pins the first chip, daisy-chained to other similar chips as much as needed and technically supported (please read the datasheets of the selected model for references about those limits).
 * 
 * Repository: https://github.com/GabyGold67/ShiftRegGPIOXpander_ESP32
 * 
 * Framework: Arduino  
 * Platform: ESP32  
 * 
 * @author Gabriel D. Goldman  
 * mail <gdgoldman67@hotmail.com>  
 * Github <https://github.com/GabyGold67>  
 * 
 * @version 1.1.2
 * 
 * @date First release: 12/02/2025  
 *       Last update:   01/04/2025 11:10 (GMT+0200) DST  
 * 
 * @copyright Copyright (c) 2025  GPL-3.0 license
 *******************************************************************************
  * @attention	This library was developed as part of the refactoring process for
  * an industrial machines security enforcement and productivity control
  * (hardware & firmware update). As such every class included complies **AT LEAST**
  * with the provision of the attributes and methods to make the hardware & firmware
  * replacement transparent to the controlled machines. Generic use attributes and
  * methods were added to extend the usability to other projects and application
  * environments, but no fitness nor completeness of those are given but for the
  * intended refactoring project.  
  * 
  * @warning **Use of this library is under your own responsibility**  
  * 
  * @warning The use of this library falls in the category described by The Alan 
  * Parsons Project (c) 1980 Games People play disclaimer:   
  * Games people play, you take it or you leave it  
  * Things that they say aren't alright  
  * If I promised you the moon and the stars, would you believe it?  
 *******************************************************************************
 */
#ifndef _ShiftRegGPIOXpander_ESP32_H_
#define _ShiftRegGPIOXpander_ESP32_H_

#include <Arduino.h>
#include <stdint.h>

/**
 * @brief A class that models a GPIO outputs pins expander through the use of 8-bits serial in paralell out (SIPO) shift registers
 * 
 * The GPIO pins expansion modeled adds digital output pins managed by the use of an API similar to the built in Arduino platform tools. As the hardware is built using daisy-chained 74HCx595 shift registers, the connection pins to the hardware are needed as parameters to build the object, as the number of shift registers daisy-chain connected is needed
 * 
 * @class ShiftRegGPIOXpander
 */
class ShiftRegGPIOXpander{
private:
   uint8_t _ds{};
   uint8_t _sh_cp{};
   uint8_t _st_cp{};
   uint8_t _srQty{};

   uint8_t* _srArryBuffPtr{};
   uint8_t* _auxArryBuffPtr{nullptr};
   uint8_t _maxSrPin{0};

   bool _sendSnglSRCntnt(uint8_t data); // Sends the content of a single byte to a Shift Register filling it's internal buffer, but it does not latch it (it does not set the output pins of the shfit register to the buffered value). The latching must be done by the calling party.
public:
   /**
    * @brief Class default constructor
    */
   ShiftRegGPIOXpander();
   /**
    * @brief Class constructor
    * 
    * Instantiates a ShiftRegGPIOXpander object, the parameters provide the pins used to communicate with the shift registers and the number of shift registers composing the expander. 8-bits will be added with every shift register connected in a daisy-chain arrangement.  
    * 
    * @param ds MCU GPIO pin connected to the DS pin -a.k.a. serial data input (DIO)- pin of the 74HCx595 to send data serially to the expander
    * @param sh_cp MCU GPIO pin connected to the SH_CP pin -a.k.a. shift register clock input- of the 74HCx595 to manage the communication's clock line to the expander
    * @param st_cp MCU GPIO pin connected to the ST_CP pin -a.k.a. storage register clock input- to set (latch) the output pins from the internal buffer of the expander
    * @param srQty Optional parameter. Quantity of shift registers set in daisy-chain configuration composing the expander.
    * @param initCntnt Optional parameter. Initial value to be loaded into the Main Buffer, and thus will be the inital state of the Shift Register output pins. The value is provided in the form of a uint8_t*, and the constructor expects the data to be set in the memory area from the pointed address to the pointed address + (srQty - 1) consecutive bytes. If the parameter is not provided, or set to nullptr the inital value to be loaded into the Main Buffer will be 0x00 to all the shift registers positions.
    * 
    * @note The object will create a dynamic array to buffer the information written to the shift registers, it will be referred to as the **Main Buffer**, **the Buffer** or **the Main**.  
    * The action of sending the Buffer contents to the shift registers array will be reffered as **Flushing**. Every time the Buffer is **flushed** to the shift registers array the whole contents of that array will be sent.  
    * A secondary dynamic array will be created for delayed operations purposes, that buffer will be referred to as **the Auxiliary Buffer** or **the Auxiliary**. The Auxiliary will be created every time it's needed and destroyed after it's temporary use becomes unnecesary. The Auxiliary will be used to allow several bit changing operations without the need of flushing the whole buffer for each bit change. The usual propper use of the mechanism will make all the bits changes that occur simultaneously to the Auxiliary Buffer and then **moving** the Auxiliary Buffer to the Main Buffer and flushing the Buffer.  
    * 
    * @attention There is no mechanism to flush the Auxiliary straight to the shift registers, every method that invokes a Main Buffer modification -see void digitalWrite(const uint8_t, const uint8_t) - and/or flushing -see bool sendAllSRCntnt() - will force first the Auxiliary to be moved over the Main Buffer, destroy the Auxiliary, perform the intended operation over the Main Buffer and then finally flush the resulting Main Buffer contents to the shift registers. This procedure is enforced to guarantee buffer contents consistency and avoid any loss of modifications done to the Auxiliary.  
    */
   ShiftRegGPIOXpander(uint8_t ds, uint8_t sh_cp, uint8_t st_cp, uint8_t srQty = 1, uint8_t* initCntnt = nullptr);
   /**
    * @brief Class destructor
    * 
    * Takes care of resources releasing
    */
   ~ShiftRegGPIOXpander();
   /**
    * @brief Copies the Buffer content to the Auxiliary Buffer  
    * 
    * If there's no existent Auxiliary the method creates it and copies the content  
    * If there's an Auxiliary, the method will proceed acording to the value passed in the parameter:
    * - If the parameter passed is true the Auxiliary contents will be overwritten with the Buffer contents
    * - If the parameter passed is true the Auxiliary will not be modified and the method will return a false to flag the failure of the operation
    * 
    * @param overWriteIfExists Indicates the authorization to overwrite the Auxiliary with the contents of the Main Buffer if the Auxiliary exists at the moment of the invocation.
    * 
    * @return The success of the overwriting operation
    * @retval true The Auxiliary was non-existent, or existed and the parameter was true
    * @retval false The Auxiliary was existent and the parameter was false, not allowing the Auxiliary to be overwritten
    */
   bool copyMainToAux(bool overWriteIfExists = true);
   /**
    * @brief Deletes the Auxiliary Buffer
    * 
    * Discards the contents of the Auxiliary Buffer, frees the memory allocated to it and nullyfies the corresponding memory pointer
    */
   void discardAux();
   /**
    * @brief Returns the state of the requested pin.
    * 
    * @param srPin Pin whose current value is required.
    * 
    * @return The state value of the requested pin, either HIGH (0x01) or LOW (0x00)
    * @retval 0x00 The pin state was LOW
    * @retval 0x01 The pin state was HIGH
    * @retval 0xFF ERROR, the pin number was beyond implemented limit
    * 
    * @attention The value is retrieved from the object's Buffer, not the real chip. If a change to the buffer was made by using the digitalWriteAux(const uint8_t, uint8_t) method (deferred update digital output pin value setting), without updating the Main Buffer an inconsistency difference might be expected, so the method checks for the Auxiliary existence, if it exists a moveAuxToMain(true) will be executed before returning the pin state.  
    * @warning If a moveAuxToMain(true) had to be executed, the Auxiliary will be destroyed. It's supposed every new need of the Auxiliary will automatically create a new instance of that buffer, but keep this concept in mind.    
    */
   uint8_t digitalReadSr(const uint8_t &srPin);
   /**
   * @brief Set a specific pin to either HIGH (0x01) or LOW (0x00).
   * 
   * @param srPin a positive value indicating which pin to set. The valid range is 0 <= srPin <= (srQty*8)-1
   * @param value Value to set for the indicated Pin
   */
   void digitalWriteSr(const uint8_t srPin, const uint8_t value);
   /**
   * @brief Sets all the pins to LOW (0x00).
   */
   void digitalWriteSrAllReset();
   /**
   * @brief Sets all the pins to HIGH (0x01).
   */
   void digitalWriteSrAllSet();
  /**
   * @brief Modifies the Main buffer contents by resetting simultaneously certain pins.
   * 
   * The pins to be reset are provided as a parameter pointer to a mask. Every bit position set (HIGH, 0x01) on the mask will be reset in the buffer, leaving the reset pins (LOW, 0x00) positions of the mask unmodified in the Main Buffer.
   * 
   * @param newResetMask Pointer to the array containing the mask to modify the Main.
   * 
   * @note The method provides a mechanism for clearing (reseting or lowering) various Main buffer bit positions in a single operation.
   * 
   * @attention Any modifications made in the Auxiliary will be moved to the Main before applying the mask.
   * 
   * @attention The Main Buffer will be flushed after the mask modificatons are applied.
   */
   void digitalWriteSrMaskReset(uint8_t* newResetMask);
  /**
   * @brief Modifies the Main buffer contents by setting simultaneously certain pins.
   * 
   * The pins to be set are provided as a parameter pointer to a mask. Every bit position set (HIGH, 0x01) on the mask will be set in the buffer, leaving the reset pins (LOW, 0x00) positions of the mask unmodified in the Main Buffer.
   * 
   * @param newResetMask Pointer to the array containing the mask to modify the Main.
   * 
   * @note The method provides a mechanism for seting (rising) various Main buffer bit positions in a single operation.
   * 
   * @attention Any modifications made in the Auxiliary will be moved to the Main before applying the mask.
   * 
   * @attention The Main Buffer will be flushed after the mask modificatons are applied.
   */
  void digitalWriteSrMaskSet(uint8_t* newSetMask);
   /**
   * @brief Set a specific pin to either HIGH (0x01) or LOW (0x00) in the Auxiliary Buffer
   * 
   * @param srPin a positive value indicating which pin to set. The valid range is 0 <= srPin <= (srQty*8)-1
   * @param value Value to set for the indicated Pin
   * 
   * @attention The pin modified in the Auxiliary will not be reflected on the pins of the GPIOXpander until the Auxiliary Buffer is copied into the Main Buffer and the latter one is flushed. This method main purpose is to modify more than one pin that must be modified at once and then proceed with the bool moveAuxToMain(bool)
   * 
   * @note An alternative procedure in the line of the use of .print() and .println() methods is to invoke the several digitalWriteToAux() methods needed and then invoking a digitalWrite(const uint8_t, const uint8_t) for the last pin writing. This last method will take care of moving the Auxiliary to the Main, set that last pin value and flush the Main Buffer.
   */
   void digitalWriteSrToAux(const uint8_t srPin, const uint8_t value);
   /**
    * @brief Retrieves the pointer to the Main Buffer.
    * 
    * @return Pointer to the array of uint8_t holding the buffered shift registers values
    *
    * @note The returned array's length is equal to the number of shift registers set in daisy-chain, see uint8_t getSrQty() for information.
    */
   uint8_t* getMainBuffPtr();
   /**
     * @brief Return the greatest valid pin number
     * 
     * The greatest valid pin number is directly related to the quantity of shift registers connected
     * 
     * @return uint8_t maxPin value
     */
   uint8_t getMaxPin();
   /**
     * @brief Return the quantity of shift registers composing the GPIOXtender object
     * 
     * The value is passed as a parameter in the class constructor
     * 
     * @return uint8_t The number of shift registers represented by the object
     */
   uint8_t getSrQty();
  /**
    * @brief Moves the data in the Auxiliary to the Main
    * 
    * Moving the contents from the Auxiliary to the Main implies several steps:  
    * - Check the existence of the Auxiliary
    * - Copy the contents from the Auxiliary to the Main
    * - Delete the Auxiliary
    * 
    * @return The success of moving the data from the Auxiliar to the Main
    * @retval true There was an Auxiliary an it's value could be moved
    * @retval false There was no Auxiliary present, no data have been moved
    */
  bool moveAuxToMain(bool flushASAP = true);
  /**
   * @brief Sets all of the output pins of the shift register to new provided values at once.  
   * 
   * The method gives a tool to change all the shift registers pins in one operation instead of pin by pin. The method uses the provided pointer as a data source to overwrite the Main Buffer with the new contents.
   * 
   * @param newCntntPtr A uint8_t* pointing the memory area containing the new values to be set to the buffer
   * @return Success in overwritting the Buffer
   * @retval true The Buffer was overwritten and the new contents flushed
   * @retval false The parameter passed was a nullptr/NULL and no overwritting was possible.  
   * 
   * @attention If there was an ongoing set of deferred pin modifications through the use of writes to the Auxiliary they will be all lost, as the first data consistency prevention of the method is to delete the Auxiliary.
   * 
   * @warning As soon as the Main is overwritten with the new values, the Buffer will be flushed.  
   */
  bool stampOverMain(uint8_t* newCntntPtr);
   /**
    * @brief Flushes the contents of the Buffer to the GPIO Expander pins.
    * 
    * The method will ensure the object buffer is updated -if there are modifications pending in the Auxiliary Buffer- enable the hardware to receive the information and invoke the needed methods to send serially the information required to each physical shift register.
    * 
    * @param flushASAP indicates if the method should take care of sending the updated Main Buffer to the shift registers before exiting, or avoid doing so. The avoidance is related to the use of the method by another method or procedure that will take care of invoking a bool sendAllSRCntnt() by itself.
    * 
    * @return true if the operation succeeds
    * 
    * @note The adoption of a boolean type return value is a consideration for future development that may consider the method operation to fail. At the time of this writing there's no conditions that would indicate otherwise
    * 
    * @warning The Auxiliary buffer is a non permanent memory array, it will be deleted after moving it's contents to the Main Buffer 
    */
   bool sendAllSRCntnt();
};

#endif //ShiftRegGPIOXpander_ESP32_H_