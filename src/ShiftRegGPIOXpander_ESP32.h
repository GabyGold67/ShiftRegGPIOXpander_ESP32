/**
 ******************************************************************************
 * @file ShiftRegGPIOXpander_ESP32.h
 * @brief Header file for the ShiftRegGPIOXtender_ESP32 library 
 * 
 * @details The library provides the means to extend the GPIO available pins -**for digital output only**- by providing a pin output manipulation API similar to the provided by Arduino, for it's own GPIO pins, for shift registers attached to the controller. The class and related definitions are provided for 74HCx595 shift registers connected to the MCU by the required three pins for the first chip, daisy-chained to other similar chips as much as needed and technically supported (please read the datasheet of the selected model for references about those limits).
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
 * @version 3.0.0
 * 
 * @date First release: 12/02/2025  
 *       Last update:   03/06/2025 18:50 (GMT+0200) DST  
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
#ifndef _SHIFTREGGPIOXPANDER_ESP32_H_
#define _SHIFTREGGPIOXPANDER_ESP32_H_

#include <Arduino.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

class SRGXVPort;

/**
 * @brief A class that models a GPIO outputs pins expander through the use of 8-bits Serial In Paralell Out (SIPO) shift registers
 * 
 * The GPIO pins expansion modeled adds digital output pins managed by the use of an API similar to the built in Arduino platform tools. As the hardware is built using daisy-chained 74HCx595 shift registers, the connection pins to the hardware are needed as parameters to build the object, as is the number of shift registers daisy-chain connected is needed.  
 * 
 * Being those three parameters hardware construction related, no mechanisms are provided to modify them after the object is created.
 * 
 * @class ShiftRegGPIOXpander
 */
class ShiftRegGPIOXpander{

private:
   uint8_t _ds{};
   uint8_t _sh_cp{};
   uint8_t _st_cp{};

   /**
    * @brief A private version of the copyMainToAux() method
    * 
    * This method is used internally to copy the contents of the Main Buffer to the Auxiliary Buffer, without taking care of the mutexes, it is used by calling parties that already have the mutexes taken, and thus are not in danger of concurrent access to the Auxiliary Buffer, and deadlockings due to nested mutexes.
    * 
    * @note The method is used by the copyMainToAux() method, which takes care of the mutexes before calling this method.
    */
   bool _copyMainToAux(const bool &overWriteIfExists = true);
   /**
    * @brief A private version of the discardAux() method
    * 
    * This method is used internally to discard the Auxiliary Buffer, without taking care of the mutexes, it is used by calling parties that already have the mutexes taken, and thus are not in danger of concurrent access to the Auxiliary Buffer, and deadlockings due to nested mutexes.
    */
   void _discardAux();
   /**
    * @brief A private version of the moveAuxToMain() method
    * 
    * This method is used internally to move the contents of the Auxiliary Buffer to the Main Buffer, without taking care of the mutexes, it is used by calling parties that already have the mutexes taken, and thus are not in danger of concurrent access to the Auxiliary Buffer, and deadlockings due to nested mutexes.
    * 
    * @return A boolean value indicating the success of the operation.
    * @retval true The Auxiliary Buffer was successfully moved to the Main Buffer, and the Auxiliary Buffer was discarded.
    * @return false The Auxiliary Buffer move operation failed, either because the Auxiliary Buffer does not exist or because the Main Buffer is not available for writing.
    */
   bool _moveAuxToMain();
   /**
    * @brief Flushes the contents of the Main Buffer to the GPIO Expander pins.  
    * 
    * The **private method** will ensure the object's Main Buffer is updated -if there are modifications pending in the Auxiliary Buffer- enable the hardware to receive the information, invoke the needed methods to send the information required to each physical shift register and activate the shift registers latching function, that sets the output pins levels to the Main Buffer values.  
    * 
    * @return true if the operation succeeds.  
    * 
    * @note The adoption of a boolean type return value is a consideration for future development that may consider the method operation to fail. At this development stage there's no conditions that would produce such outcome.  
    * 
    * @warning The Auxiliary buffer is a non permanent memory array, it will be deleted after moving it's contents to the Main Buffer 
    */
   bool _sendAllSRCntnt();
   /**
    * @brief Sends the content of a single byte to a Shift Register. 
    * 
    * The method's action is limited to filling the shift register's internal buffer, but it does not latch it (it does not set the output pins of the shfit register to the buffered value). The latching must be done by the calling party, when the contents of all the shift registers are set to the desired values. The usual calling of this method is done by the _sendAllSRCntnt() method, which will flush the contents of the Main Buffer to the shift registers array.  
    * 
    * @param data The byte to be sent to the shift register, the bits in the byte will be sent in the order from MSB to LSB (MSB First). 
    * 
    * @return true Allways true, as the method does not have any condition that would produce a failure in the operation. The boolean type return value is a consideration for backward compatibility with previous versions.
    */
   bool _sendSnglSRCntnt(const uint8_t &data); 

protected:
   SemaphoreHandle_t _SRGXAuxBffrMutex; // Mutex to protect the Auxiliary Buffer from concurrent access
   SemaphoreHandle_t _SRGXMnBffrMutex; // Mutex to protect the Main Buffer from concurrent access

   uint8_t* _mainBuffrArryPtr{};
   uint8_t* _auxBuffrArryPtr{nullptr};
   uint8_t _maxSRGXPin{};
   uint8_t _srQty{};

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
    * 
    * @note The object will create a dynamic array to buffer the information written to the shift registers, it will be referred to as the **Main Buffer**, **the Buffer** or **the Main**.  
    * The action of sending the Buffer contents to the shift registers array will be reffered as **Flushing**. Every time the Buffer is **flushed** to the shift registers array the whole contents of that array will be sent.  
    * A secondary dynamic array will be created for delayed operations purposes, that buffer will be referred to as the **Auxiliary Buffer** or **the Auxiliary**. The Auxiliary will be created every time it's needed and destroyed after it's temporary use becomes unnecessary. The Auxiliary will be used to allow several bit changing operations without the need of flushing the whole buffer for each bit change. The usual propper use of the mechanism will make all the bits changes that occur simultaneously to the Auxiliary Buffer and then **moving** the Auxiliary Buffer to the Main Buffer and flushing the Buffer.  
    * 
    * @note There is no mechanism to flush the **Auxiliary** straight to the shift registers.  
    * 
    * @attention Every method that invokes a Main Buffer modification -see digitalWriteSr(const uint8_t, const uint8_t), digitalWriteSrAllReset(), digitalWriteSrAllSet(), digitalWriteSrMaskReset(uint8_t*), digitalWriteSrMaskSet(uint8_t*) - and/or flushing -see bool _sendAllSRCntnt() - will force first the Auxiliary to be moved over the Main Buffer, destroy the Auxiliary, perform the intended operation over the Main Buffer and then finally flush the resulting Main Buffer contents to the shift registers. This procedure is enforced to guarantee buffer contents consistency and avoid any loss of modifications done to the Auxiliary. The digitalReadSr(const uint8_t) method will also invoke a moveAuxToMain() before returning the requested pin state. See digitalReadSr(const uint8_t) for more information.
    */
   ShiftRegGPIOXpander(uint8_t ds, uint8_t sh_cp, uint8_t st_cp, uint8_t srQty = 1);
   /**
    * @brief Class destructor
    * 
    * Takes care of resources releasing
    */
   ~ShiftRegGPIOXpander();
   /**
    * @brief GPIOXpander object setup and activation 
    * 
    * This method sets the controller pins configuration, and optionally sets the initial pin values for the GPIOXpander pins.
    * 
    * @param initCntnt Optional parameter. Initial value to be loaded into the Main Buffer, and thus will be the inital state of the Shift Register output pins. The value is provided in the form of a uint8_t*, and the constructor expects the data to be set in the memory area from the pointed address to the pointed address + (srQty - 1) consecutive bytes. If the parameter is not provided, or set to nullptr the inital value to be loaded into the Main Buffer will be 0x00 to all the shift registers positions, making all pins' output 0/LOW/RESET.  
    * 
    * @return The success of the operation.
    * @retval true The operation was successful, the GPIOXpander object is ready to be used.
    * @retval false The operation failed, either because the pins could not be set, or the mutexes could not be created.
    */
   bool begin(uint8_t* initCntnt = nullptr);   
   /**
    * @brief Copies the Buffer content to the Auxiliary Buffer  
    * 
    * - If there's no existent Auxiliary the method creates it and copies the content of the Main.  
    * - If there's an Auxiliary, the method will proceed according to the value passed in the parameter:  
    * - If the parameter passed is true the Auxiliary contents will be overwritten with the Main contents.  
    * - If the parameter passed is false the Auxiliary will not be modified and the method will return a false to flag the failure of the operation.  
    * 
    * @param overWriteIfExists Indicates the authorization to overwrite the Auxiliary with the contents of the Main Buffer if the Auxiliary exists at the moment of the invocation.  
    * 
    * @return The success of the overwriting operation.  
    * @retval true The Auxiliary was non-existent, or existed and the parameter allowing overwriting was true.  
    * @retval false The Auxiliary was existent and the parameter allowing overwriting was false, generating a failure in the operation.  
    */
   bool copyMainToAux(const bool &overWriteIfExists = true);
   /**
    * @brief Instantiate a SRGXVPort object
    * 
    * The method will create a SRGXVPort object, a virtual port that will allow the user to manipulate a set of pins as a single entity, with it's pins numbered from 0 to pinsQty - 1, where pinsQty is the number of pins that compose the virtual port. 
    * 
    * @param strtPin ShiftRegGPIOXpander pin number from which the virtual port will start. The valid range is 0 <= strtPin <= getMaxSRGXPin().
    * @param pinsQty Number of pins that will compose the virtual port. The valid range is 1 <= pinsQty <= (getMaxSRGXPin() - strtPin + 1).
    * @return SRGXVPort The SRGXVPort object created, or an empty SRGXVPort object if the parameters provided were not valid.
    * 
    * @attention Note that as described, the minimum amount of pins that can be set in a virtual port is 1, and the maximum amount of pins that can be set in a virtual port is equal to the number of shift registers multiplied by 8 minus the strtPin value, although using the maximum amount of pins available make no sense as the virtual port will be the same as the whole GPIOXpander object.  
    */
   SRGXVPort createSRGXVPort(const uint8_t &strtPin, const uint8_t &pinsQty);
   /**
    * @brief Returns a 16-bits value containing a zero-based segment of the Main Buffer.
    * 
    * The method will return a 16-bits value containing the bits from the Main Buffer delimited by the parameters provided, formatted as a zero-based segment (or right aligned value), meaning that the first bit of the segment will hold the value of the strtPin parameter, and subsequent bits will hold the values of the following pins, up to the pinsQty parameter.  
    * 
    * @note The retuning value will be a 16-bits value, even if the segment requested is smaller than 16 bits. The bits in the returned value will be right aligned, and zero padded, meaning that if the segment requested is smaller than 16 bits, the leftmost bits of the returned value will be set to 0x00.
    * 
    * @param strtPin The first pin number from which the segment will be taken. The valid range is 0 <= strtPin <= getMaxSRGXPin().
    * @param pinsQty The number of pins that will compose the segment. The valid range is 1 <= pinsQty <= (_maxSRGXPin - strtPin + 1).
    * @param buffSgmnt A reference to a uint16_t variable where the segment will be stored. The variable must be initialized before calling the method, and it will be set to 0 before setting the segment bits.
    * 
    * @return A boolean value indicating the success of the operation.
    * @retval true The segment was successfully retrieved and stored in the buffSgmnt variable.
    * @retval false The parameters provided were not valid, or the operation failed for any other reason.
    * 
    * @attention The method is tightly related to the SRGXVPort class, which uses it to retrieve the segment of the Main Buffer that corresponds to the virtual port being manipulated. The SRGXVPort class will ensure that the parameters provided are valid before calling this method.
    */
   bool digitalReadSgmntSr(const uint8_t &strtPin, const uint8_t &pinsQty, uint16_t &bffrSgmnt);   
   /**
    * @brief Returns the state of the requested pin.
    * 
    * @param srPin Pin whose current value is required.
    * 
    * @return The state value of the requested pin, either HIGH (0x01/Set) or LOW (0x00/Reset)
    * @retval 0x00 The pin state was LOW
    * @retval 0x01 The pin state was HIGH
    * @retval 0xFF ERROR, the pin number was beyond implemented limit
    * 
    * @attention The value is retrieved from the object's Buffer, not the real chip. If a change to the Auxiliary was made by using the digitalWriteAux(const uint8_t, uint8_t) method (deferred update digital output pin value setting), without updating the Main Buffer an inconsistency might appear if the srPin to be read value is different in the Main from the Auxiliary. For ensuring data consistency  the method checks for the Auxiliary existence, if the Auxiliary exists a moveAuxToMain(true) will be performed before returning the pin state.  
    * @warning If a moveAuxToMain(true) had to be executed, the Auxiliary will be destroyed. This will have no major consequences as every new need of the Auxiliary will automatically create a new instance of that buffer, but keep this concept in mind.  
    */
   uint8_t digitalReadSr(const uint8_t &srPin);   
   /**
     * @brief Toggles the state of a specific pin.
     * 
     * @details The method will set the pin to LOW (0x00) if it was HIGH (0x01) and vice versa. The method will flush the buffer, so the change will be reflected on the GPIO pin.
     * 
     * @param srPin A positive value indicating which pin to toggle. The valid range is 0 <= srPin <= getMaxSRGXPin()
     * 
     * @attention As the value is written to the object's Buffer, the existence of the Auxiliary (by a deferred update digital output pin value setting), an inconsistency might appear if the srPin to be written value is different in the Main from the Auxiliary. For ensuring data consistency  the method checks for the Auxiliary existence, if the Auxiliary exists a moveAuxToMain(false) will be performed before seting the new pin state.
     * 
     * @warning If a moveAuxToMain(false) had to be executed, the Auxiliary will be destroyed. This will have no major consequences as every new need of the Auxiliary will automatically create a new instance of that buffer, but keep this concept in mind.
     * 
     * @return A boolean value indicating the success of the operation.
     * @retval true The operation was successful, the pin was toggled in the Main Buffer and the change was flushed to the GPIO pin.
     * @retval false The operation failed, either because the pin number was beyond the implemented limit or because the mutexes could not be taken.
     */
   bool digitalToggleSr(const uint8_t &srPin);
   /**
    * @brief Toggles the state of all the pins.
    * 
    * @details The method will set all the pins to LOW (0x00) if they were HIGH (0x01) and vice versa. The method will flush the buffer, so the change will be reflected on all the GPIO pins.
    * 
    * @attention As the values are written to the object's Buffer, the existence of the Auxiliary (by a deferred update digital output pin value setting), an inconsistency might appear if the pins to be written values are different in the Main from the Auxiliary. For ensuring data consistency  the method checks for the Auxiliary existence, if the Auxiliary exists a discardAux() will be performed before seting the new pin states.
    * 
    * @return A boolean value indicating the success of the operation.
    * @retval true The operation was successful, all the pins were toggled in the Main Buffer and the change was flushed to the GPIO pins.
    * @retval false The operation failed, because the mutexes could not be taken.
    */
   bool digitalToggleSrAll();
   /**
    * @brief Toggles the state of the pins in the Main Buffer according to the provided mask.
    * 
    * The pins to be toggled are provided as a pointer to a mask parameter. Every bit position set (HIGH, 0x01) on the mask will be modified in the Main Buffer, reset pins (LOW, 0x00) positions of the mask will remain unmodified in the Main Buffer. The modification performed will be toggling the bit position.
    * 
    * @param toggleMask Pointer to the array containing the mask to modify the Main.
    * 
    * @note The method provides a mechanism for toggling various Main buffer bit positions in a single operation.
    * 
    * @attention Any modifications made in the Auxiliary will be moved to the Main and it will be deleted before applying the mask.
    * 
    * @result A boolean value indicating the success of the operation.
    * @retval true The operation was successful, the pins were toggled in the Main Buffer and the change was flushed to the GPIO pins.
    * @retval false The operation failed, either because the mask was null or because the mutexes could not be taken.
    */
   bool digitalToggleSrMask(uint8_t* toggleMask);
   /**
    * @brief Toggles the state of a specific pin in the Auxiliary Buffer.
    * 
    * @param srPin A positive value indicating which pin to toggle. The valid range is 0 <= srPin <= getMaxSRGXPin()  
    * 
    * @attention The pin modified in the Auxiliary will not be reflected on the pins of the GPIOXpander until the Auxiliary Buffer is copied into the Main Buffer and the latter one is flushed. This method main purpose is to modify more than one pin that must be modified at once and then proceed with the bool moveAuxToMain(bool).
    * 
    * @note An alternative procedure analog to the use of .print() and .println() methods is to invoke the several digitalToggleSrToAux() methods needed but the last, and then invoking a digitalToggleSr(const uint8_t) for the last pin writing. This last method will take care of moving the Auxiliary to the Main, set that last pin value and flush the Main Buffer.
    * 
    * @return A boolean value indicating the success of the operation.
    * @retval true The operation was successful, the pin was toggled in the Auxiliary Buffer.
    * @retval false The operation failed, either because the pin number was beyond the implemented limit or because the mutexes could not be taken.
    */
   bool digitalToggleSrToAux(const uint8_t &srPin); 
   /**
   * @brief Set a specific pin to either HIGH (0x01) or LOW (0x00).
   * 
   * @param srPin a positive value indicating which pin to set. The valid range is 0 <= srPin <= getMaxSRGXPin()  
   * @param value Value to set the indicated Pin.  
   * 
   * @attention As the value is written to the object's Buffer, the existence of the Auxiliary (by a deferred update digital output pin value setting), an inconsistency might appear if the srPin to be written value is different in the Main from the Auxiliary. For ensuring data consistency  the method checks for the Auxiliary existence, if the Auxiliary exists a moveAuxToMain(false) will be performed before seting the new pin state.  
   * @warning If a moveAuxToMain(false) had to be executed, the Auxiliary will be destroyed. This will have no major consequences as every new need of the Auxiliary will automatically create a new instance of that buffer, but keep this concept in mind.  
   * 
   * @return A boolean value indicating the success of the operation.
   * @retval true The operation was successful, the pin was set in the Main Buffer and the change was flushed to the GPIO pin.
   * @retval false The operation failed, either because the pin number was beyond the implemented limit or because the mutexes could not be taken.
   */
   bool digitalWriteSr(const uint8_t &srPin, const uint8_t &value);
   /**
   * @brief Sets all the pins to LOW (0x00/Reset).
   * 
   * @attention As the new values are written to the object's Buffer, the existence of the Auxiliary might produce an inconsistency to appear. For ensuring data consistency the method checks for the Auxiliary existence, if the Auxiliary exists a discardAux() will be performed before setting the new values to the Main.  
   * @warning If discardAux() has to be executed, the Auxiliary will be destroyed. This will have no major consequences as every new need of the Auxiliary will automatically create a new instance of that buffer, but keep this concept in mind.  
   * 
   * @return A boolean value indicating the success of the operation.
   * @retval true The operation was successful, all the pins were set to LOW in the Main Buffer and the change was flushed to the GPIO pins.
   * @retval false The operation failed, because the mutexes could not be taken*/
   bool digitalWriteSrAllReset();
   /**
   * @brief Sets all the pins to HIGH (0x01).
   * 
   * @attention As the new values are written to the object's Buffer, the existence of the Auxiliary might produce an inconsistency to appear. For ensuring data consistency the method checks for the Auxiliary existence, if the Auxiliary exists a discardAux() will be performed before setting the new values to the Main.  
   * @warning If discardAux() had to be executed, the Auxiliary will be destroyed. This will have no major consequences as every new need of the Auxiliary will automatically create a new instance of that buffer, but keep this concept in mind.  
   * 
   * @return A boolean value indicating the success of the operation.
   * @retval true The operation was successful, all the pins were set to HIGH in the Main Buffer and the change was flushed to the GPIO pins.
   * @retval false The operation failed, because the mutexes could not be taken.
   */
   bool digitalWriteSrAllSet();
  /**
   * @brief Modifies the Main buffer contents by resetting simultaneously certain pins.
   * 
   * The pins to be reset are provided as a pointer to a mask parameter. Every bit position set (HIGH, 0x01) on the mask will be modified in the Main Buffer, reset pins (LOW, 0x00) positions of the mask will remain unmodified in the Main Buffer. The modification performed will be setting the bit position to LOW (0x00).
   * 
   * @param resetMask Pointer to the array containing the mask to modify the Main.
   * 
   * @note The method provides a mechanism for clearing (reseting/lowering) various Main buffer bit positions in a single operation.
   * 
   * @attention Any modifications made in the Auxiliary will be moved to the Main and will be deleted before applying the mask.  
   * 
   * @attention The Main Buffer will be flushed after the mask modifications are applied.
   * 
   * @return A boolean value indicating the success of the operation.
   * @retval true The operation was successful, the pins were reset in the Main Buffer and the change was flushed to the GPIO pins.
   * @retval false The operation failed, either because the mask was null or because the mutexes could not be taken.
   */
   bool digitalWriteSrMaskReset(uint8_t* resetMask);
   /**
   * @brief Modifies the Main buffer contents by setting simultaneously certain pins.
   * 
   * The pins to be set are provided as a parameter pointer to a mask. Every bit position set (HIGH, 0x01) on the mask will be modified in the Main Buffer, reset pins (LOW, 0x00) positions of the mask will remain unmodified in the Main Buffer. The modification performed will be setting the bit position to HIGH (0x01).  
   * 
   * @param setMask Pointer to the array containing the mask to modify the Main.
   * 
   * @note The method provides a mechanism for seting (rising) various Main buffer bit positions in a single operation.
   * 
   * @attention Any modifications made in the Auxiliary will be moved to the Main and will be deleted before applying the mask.  
   * 
   * @attention The Main Buffer will be flushed after the mask modifications are applied.
   * 
   * @return A boolean value indicating the success of the operation.
   * @retval true The operation was successful, the pins were set in the Main Buffer and the change was flushed to the GPIO pins.
   * @retval false The operation failed, either because the mask was null or because the mutexes could not be taken.
   */
   bool digitalWriteSrMaskSet(uint8_t* setMask);  
   /**
   * @brief Set a specific pin to either HIGH (0x01) or LOW (0x00) in the Auxiliary Buffer
   * 
   * @param srPin a positive value indicating which pin to set. The valid range is 0 <= srPin <= getMaxSRGXPin()
   * @param value Value to set for the indicated Pin.  
   * 
   * @attention The pin modified in the Auxiliary will not be reflected on the pins of the GPIOXpander until the Auxiliary Buffer is copied into the Main Buffer and the latter one is flushed. This method main purpose is to modify more than one pin that must be modified at once and then proceed with the bool moveAuxToMain(bool).  
   * 
   * @note An alternative procedure analog to the use of .print() and .println() methods is to invoke the several digitalWriteSrToAux() methods needed but the last, and then invoking a digitalWrite(const uint8_t, const uint8_t) for the last pin writing. This last method will take care of moving the Auxiliary to the Main, set that last pin value and flush the Main Buffer. 
   * 
   * @return A boolean value indicating the success of the operation.  
   * @retval true The operation was successful, the pin was set in the Auxiliary Buffer.
   * @retval false The operation failed, either because the pin number was beyond the implemented limit or because the mutexes could not be taken. 
   */
   bool digitalWriteSrToAux(const uint8_t srPin, const uint8_t value);
   /**
    * @brief Deletes the Auxiliary Buffer.  
    * 
    * Discards the contents of the Auxiliary Buffer, frees the memory allocated to it and nullifies the corresponding memory pointer. If the Auxiliary Buffer was not created, the method will do nothing. If the Auxiliary is not transferred to the Main Buffer before invoking this method, the modified contents of the Auxiliary will be lost.  
    * 
    * @return A boolean value indicating the success of the operation.
    * @retval true The Auxiliary Buffer was successfully discarded, and the memory allocated to it was freed.
    * @retval false The mutexes could not be taken.
    */
   bool discardAux();
   /**
    * @brief Method provided for ending any relevant activation procedures made by the begin(uint8_t*) method.  
    * 
    * The method will be invoked as part of the class destructor.  
    */
   void end();
   /**
    * @brief Toggles the state of a specific pin in the Main Buffer.  
    * 
    * @param srPin Pin whose state is to be toggled. The valid range is 0 <= srPin <= getMaxSRGXPin()  
    * @retval true The pin was in the valid range and was toggled in the Main Buffer.
    * @retval false The pin was not in the valid range, and no action was taken.
    * 
    * @note flipBit(n) is a synonym for digitalToggleSr(n), and is provided for shortening and using more meaningful name in the code.
    */
   bool flipBit(const uint8_t &srPin);
   /**
    * @brief Retrieves the pointer to the Main Buffer.  
    * 
    * @return Pointer to the array of uint8_t holding the buffered shift registers values.  
    *
    * @note The returned array's length is equal to the number of shift registers set in daisy-chain, see uint8_t getSrQty() for information.  
    */
   uint8_t* getMainBuffPtr();
   /**
     * @brief Return the greatest valid pin number.  
     * 
     * The greatest valid pin number is directly related to the quantity of shift registers connected, and is equal to the number of shift registers multiplied by 8 minus 1, (getSrQty() * 8) - 1.  
     * 
     * @return uint8_t maxPin number value.  
     * 
     * @note The value will be used as a limit for the pin numbers to be used in the digitalWriteSr(const uint8_t, const uint8_t) and digitalReadSr(const uint8_t) methods.  
     */
   uint8_t getMaxSRGXPin();
   /**
     * @brief Return the quantity of shift registers composing the GPIOXtender object.  
     * 
     * The value is passed as a parameter in the class constructor.  
     * 
     * @return uint8_t The number of shift registers composing the physical port extender modeled by the class.  
     */
   uint8_t getSrQty();
   /**
    * @brief Checks if the provided SRGXVPort object is valid.
    * 
    * The method is useful to verify after a createVXVPort(uint8_t&, uint8_t&) invocation that the SRGXVPort object created is valid, i.e. that it was created with a valid ShiftRegGPIOXpander object pointer and that the parameters provided to create the SRGXVPort object were valid.
    * 
    * @param VPort SRGXVPort object to be checked for validity.
    * @return true The created SRGXVPort object is valid and usable.  
    * @return false The created SRGXVPort object is not valid, and should not be used.
    * 
    * @note If the method returns false, the SRGXVPort object should not be used. Consider destructing it and creating a new one with valid parameters.
    */
   bool isValid(SRGXVPort &VPort);
   /**
    * @brief Moves the data in the Auxiliary to the Main
    * 
    * Moving the contents from the Auxiliary to the Main implies several steps:  
    * - Check the existence of the Auxiliary
    * - Copy the contents from the Auxiliary to the Main (see stampOverMain(uint8_t*))
    * - Delete the Auxiliary (see discardAux())
    * 
    * @return The success of moving the data from the Auxiliary to the Main.  
    * @retval true There was an Auxiliary and it's value could be moved.  
    * @retval false There was no Auxiliary present, no data have been moved.  
    */
   bool moveAuxToMain();
   /**
    * @brief Sets a specific pin to LOW (0x00/Reset) in the Main Buffer.
    * 
    * @param srPin Pin whose state is to be reset. The valid range is 0 <= srPin <= getMaxSRGXPin()
    * @retval true The pin was in the valid range and was reset to LOW (0x00/Reset) in the Main Buffer.
    * @retval false The pin was not in the valid range, and no action was taken.
    * 
    * @note resetBit(n) is a synonym for digitalWriteSr(n, LOW), and is provided for shortening and using more meaningful name in the code.
    */
   bool resetBit(const uint8_t &srPin);
   /**
    * @brief Sets a specific pin to HIGH (0x01/Set) in the Main Buffer.
    * 
    * @param srPin Pin whose state is to be set. The valid range is 0 <= srPin <= getMaxSRGXPin()
    * @retval true The pin was in the valid range and was set to HIGH (0x01/Set) in the Main Buffer.
    * @retval false The pin was not in the valid range, and no action was taken.
    * 
    * @note setBit(n) is a synonym for digitalWriteSr(n, HIGH), and is provided for shortening and using more meaningful name in the code.
    */
   bool setBit(const uint8_t &srPin);
   /**
    * @brief Sets the value of several scattered (or not) pins in the Main Buffer, according to the provided mask and values.
    * 
    * @param maskPtr Pointer to the memory area containing the mask to be applied to the Main Buffer. The mask is a bit array where each bit position set (HIGH, 0x01) indicates that the corresponding pin in the Main Buffer will be modified with the value provided in the newValsPtr parameter. 
    * @param valsPtr Pointer to the memory area containing the values to be set in the Main Buffer. The values are provided as a bit array, where each bit position corresponds to the pin in the Main Buffer that will be modified according to the mask provided in the newMaskPtr parameter: for the array positions set in the mask, the value of the bit in the newValsPtr will be set in the corresponding pin in the Main Buffer, for the array positions not set in the mask, the value in the Main Buffer will remain unchanged.
    * 
    * @retval true Main Buffer was modified with the new values and flushed to the shift registers.
    * @retval false Main Buffer was not modified, either because the parameters provided were not valid or because the operation failed for some other reason.
    */
   bool stampMaskOverMain(uint8_t* maskPtr, uint8_t* valsPtr);
   /**
   * @brief Sets all of the output pins of the shift register to new provided values at once.  
   * 
   * The method gives a tool to change all the shift registers pins in one operation instead of pin by pin. The method uses the provided pointer as a data source to overwrite the Main Buffer with the new contents.  
   * 
   * @param newCntntPtr A uint8_t* pointing the memory area containing the new values to be set to the buffer.  
   * @return Success in overwriting the Buffer.  
   * @retval true The Buffer was overwritten and the new contents flushed.  
   * @retval false The parameter passed was a nullptr/NULL and no overwriting was possible.  
   * 
   * @attention If there was an ongoing set of deferred pin modifications through the use of writes to the Auxiliary they will be all lost, as the first data consistency prevention of the method is to discard the Auxiliary.  
   * 
   * @warning As soon as the Main is overwritten with the new values, the Buffer will be flushed.  
   */
   bool stampOverMain(uint8_t* newCntntPtr);
   /**
    * @brief Sets the value of a set of consecutive pins (the segment) in the Main Buffer. 
    * 
    * Each pin value is set or reset according to the value provided in the data array pointed by the newSgmntPtr parameter as source.  
    * 
    * @param newSgmntPtr Pointer to the memory area containing the new values to be set to the segment of pins in the Main Buffer. The value to be set in each pin is provided as a bit in the data pointed by the newSgmntPtr parameter, where the first bit in the data pointed by newSgmntPtr will be set to the first pin in the segment, and so on. For that logic to work the data must be right aligned (LSb of the segment placed in position 0) in the data pointed by newSgmntPtr.  
    * @param strtPin First pin number in the Main buffer to be overwritten. The valid range is 0 <= strtPin <= getMaxSRGXPin().
    * @param pinsQty The number of pins to be set in the segment. The valid range is 1 <= pinsQty <= (getMaxSRGXPin() - strtPin + 1).
    * 
    * @return A boolean indicating the success of the operation.  
    * @retval true The segment was set in the Main Buffer and the Buffer was flushed.  
    * @retval false The segment could not be set in the Main Buffer, either because the parameters provided were not valid or because the operation failed for some other reason.  
    * 
    * @note The intended behavior of the method is to present the resulting Main in a single operation, so the method will make all the segment modifications to the Main Buffer before flushing it. If the Auxiliary Buffer contains any pending modifications, they will be moved to the Main Buffer before the segment stamping execution.  
    * 
    * @warning The method expects newSgmntPtr to point to a valid memory area containing the data to be set in the segment. If the pointer is nullptr/NULL, the method will return false and no operation will be performed.
    * 
    * @attention Although the method, as all similar methods in the class, expects the data pointed by newSgmntPtr to be the same length as the Main Buffer, it will only use the first pinsQty bits of the data pointed by newSgmntPtr, so the data pointed by newSgmntPtr must be at least pinsQty bits long, that means that the data pointed by newSgmntPtr must be at least ceil(pinsQty / 8) bytes long. 
    */
   bool stampSgmntOverMain(uint8_t* newSgmntPtr, const uint8_t &strtPin, const uint8_t &pinsQty);
};

//==========================================================>>

/**
 * @brief A class that models **Virtual Ports** from  the resources provided by a ShiftRegGPIOXpander object.  
 * 
 * The **Virtual Ports** are  sets of pins that can be used as a group, and are defined by the user. The pins are a contiguous subset of the pins available in the ShiftRegGPIOXpander object. The **Virtual Ports** main advantage is to provide a means to focus on a group of pins that are somehow associated: are meant to manage the communications with a specific device or group of devices, manage different related signals, etc.  
 * 
 * The class objects provide the means to translate the pin numbers from the **Virtual Port** to the real pins in the ShiftRegGPIOXpander object.  
 * 
 * A simple example use case is a crossroads traffic light controller, four different traffic lights, one for each direction, totalling 12 pins, might be solved by using a ShiftRegGPIOXpander object with 2 shift registers. Using the ShiftRegGPIOXpander object directly would require the user to remember that the first pin of the first traffic light is pin 0, the second traffic light starts at pin 3, and so on. Using a SRGXVPort object for each traffic light would allow the user to create a virtual port for each traffic light. Pin 0 of each virtual port would be the red light, pin 1 would be the yellow light, and pin 2 would be the green light. The user would then be able to use the virtual port objects to manipulate the traffic lights without having to convert each light to the real pin numbers in the ShiftRegGPIOXpander object, and make the addition of new traffic lights easier, as the managing methods would be the same for each virtual port.  
 * 
 * @note The SRGXVPort class has set a maximum of 16 pins that can be used in each virtual port. The number of pins assigned will be checked against the maximum number of pins available in the ShiftRegGPIOXpander object.  
 * 
 * @note The open possibility of creating virtual ports that overlap pins in the ShiftRegGPIOXpander object is not considered a problem, even if one virtual port includes all the pins of another virtual port. Take as an example the case of the x86 Intel processors, where the 64-bits registers are a superset of the 32-bits registers, the 32-bits registers are a superset of the 16-bits registers and the 16-bits are superset of 8-bits registers, with the possibility of managing the 64 bits at once or subsets by using the corresponding designations provided. The user can manage the virtual ports as they see fit, but the library will not provide any mechanism to prevent overlapping virtual ports.  
 * 
 * @warning The SRGXVPort class uses the buffer memory provided by the ShiftRegGPIOXpander object to handle the pins state. The user might decide to use the SRGXVPort objects to manipulate the pins state, or use the ShiftRegGPIOXpander object directly. The classes are designed to allow the user to use both methods, but the user must be aware that the state of a pin might be modified by the shiftRegGPIOXpander object directly, and by any and all the SRGXPVPort objects that same pin is part of.  
 * 
 * @class SRGXVPort
 */
class SRGXVPort: public ShiftRegGPIOXpander{
   /*Allows the ShiftRegGPIOXpander class to access the private members of the SRGXVPort
   class. In this case the ShiftRegGPIOXpander class will be able instantiate SRGXVPort
   objects, as the SRGXVPort class constructor is protected to avoid instantiation by 
   the user but through the use the ShiftRegGPIOXpander::createSRGXVPort 
   (uint8_t&, uint8_t&) method to create a SRGXVPort object.*/
   friend class ShiftRegGPIOXpander;
   /*_maxPortPinsQty: Maximum number of pins that can be used in a virtual port, the 
   maximum number of pins that can be used in a virtual port is defined as 16, library
   developers choice.*/
   const static uint8_t _maxPortPinsQty{16}; 

private:
   ShiftRegGPIOXpander* _SRGXPtr{nullptr};   //!< Pointer to the ShiftRegGPIOXpander object that provides the resources for the virtual port.   
   uint8_t _strtPin{0};
   uint8_t _pinsQty{0};

   uint8_t* _srgxStampMskPtr{nullptr}; // Pointer to the mask used to stamp the virtual port over the Main Buffer of the ShiftRegGPIOXpander object.
   uint16_t _vportBuffer{0};
   /*_vportMaxVal: Maximum value that can be set in the virtual port, calculated as 
   (2^pinsQty) - 1, where pinsQty is the number of pins that compose the virtual port. 
   The value is used to enforce the range of valid values. */
   uint16_t _vportMaxVal{0};
   bool _begun{false}; // Flag to indicate if the virtual port has been begun, i.e. if the begin() method has been called and the initial state of the virtual port has been set.
   bool _buildSRGXVPortMsk(uint8_t* &maskPtr);

protected:
   /**
    * @brief Default constructor
    */
   SRGXVPort();   
   /**
    * @brief Class constructor
    * 
    * This constructor will instantiate a virtual port object that will allow the user to manipulate a subset of the pins available in the ShiftRegGPIOXpander object as a unit. The main driving concept is that the contiguous subset of pins are logically related, and thus might benefit of the chance to be manipulated as a single entity. The pins are numbered from 0 to pinsQty - 1, where pinsQty is the number of pins that compose the virtual port.
    * 
    * @param SRGXPtr A pointer to the ShiftRegGPIOXpander object that will provide the resources (pins) for the virtual port.  
    * @param strtPin The pin number from the ShiftRegGPIOXpander to be used as the first pin (pin 0) of the virtual port. 
    * @param pinsQty The number of pins that will compose the virtual port. 
    * 
    * @attention This constructor is not accesible but through the ShiftRegGPIOXpander class, as it is a protected constructor. The user should use the createSRGXVPort(uint8_t&, uint8_t&) method to create a SRGXVPort object.
    */
   SRGXVPort(ShiftRegGPIOXpander* SRGXPtr, uint8_t strtPin, uint8_t pinsQty);

public:
   /**
    * @brief Class destructor
    */
   ~SRGXVPort();
   /**
    * @brief Begins the virtual port, setting the initial state of the VPort pins.
    * 
    * @param initCntnt Initial value to be loaded into the virtual port. 
    */
   bool begin(uint16_t initCntnt);
   /**
    * @brief Reads the state of a specific pin in the virtual port.  
    * 
    * @param srPin Pin number whose state is to be read. The valid range is 0 <= srPin < _pinsQty.  
    * 
    * @return The state value of the requested pin, either HIGH (0x01/Set) or LOW (0x00/Reset).  
    */
   uint8_t digitalReadSr(const uint8_t &srPin);
   /**
    * @brief Sets the state of a specific pin in the virtual port, either HIGH (0x01/Set) or LOW (0x00/Reset).
    * 
    * @param srPin Pin number whose state is to be set. The valid range is 0 <= srPin < _pinsQty.
    * @param value The value to set the pin to, either HIGH (0x01/Set) or LOW (0x00/Reset).
    * 
    * @return The success of the operation.
    * @retval true The pin was in the valid range and was set in the Main Buffer, the change was flushed to the GPIO pin.
    * @retval false The pin was not in the valid range, and no action was taken. 
    */
   bool digitalWriteSr(const uint8_t &srPin, const uint8_t &value);
   /**
    * @brief Toggles the state of a specific pin in the virtual port.
    * 
    * If the pin state was HIGH (0x01/Set) it will be set to LOW (0x00/Reset), and vice versa. The method will flush the buffer, so the change will be reflected on the GPIO pin.
    * 
    * @param srPin Pin number whose state is to be toggled. The valid range is 0 <= srPin < _pinsQty.
    * @return The success of the operation.
    * @retval true The pin was in the valid range and was toggled in the Main Buffer.
    * @retval false The pin was not in the valid range, and no action was taken.
    */
   bool flipBit(const uint8_t &srPin);
   /**
    * @brief Returns a pointer to the ShiftRegGPIOXpander object that provides the resources for the virtual port.
    * 
    * @return A ShiftRegGPIOXpander* pointer to the object that provides the resources for the virtual port.
    */
   ShiftRegGPIOXpander* getSRGXPtr();
   /**
    * @brief Returns a pointer to the mask built to stamp the virtual port over the Main Buffer of the ShiftRegGPIOXpander object.
    * 
    * The mask is compatible with the stampMaskOverMain(uint8_t*, uint8_t*) method, and might be used to modify the Main Buffer of the ShiftRegGPIOXpander object by setting or resetting the pins in the virtual port. 
    * 
    * @return uint8_t* 
    */
   uint8_t* getStampMask();
   /**
    * @brief Returns the maximum value that can be set in the virtual port.
    * 
    * The maximum value is calculated as (2^pinsQty) - 1, where pinsQty is the number of pins that compose the virtual port. The value is used to enforce the range of valid values that can be set in the virtual port.
    * 
    * @return uint16_t Maximum value that can be set in the virtual port.
    */
   uint16_t getVPortMaxVal();
   /**
    * @brief Reads the state of the virtual port as an integer value.
    * 
    * The method reads the state of all the pins in the virtual port as a pinsQty binary number and returns the value as an unsigned integer. The bits in the returned value are ordered from the least significant bit (LSB) to the most significant bit (MSB), where the LSB corresponds to pin 0 of the virtual port and the MSB corresponds to pin pinsQty - 1.
    * 
    * @return The state of the virtual port as an unsigned integer value, where each bit represents the state of a pin in the virtual port.
    */
   uint16_t readPort();   
   /**
    * @brief Sets the state of a pin to LOW (0x00/Reset) in the virtual port.
    * 
    * @param srPin The pin number whose state is to be reset. The valid range is 0 <= srPin < _pinsQty.
    * @return true The pin was in the valid range and was reset to LOW (0x00/Reset) in the Main Buffer.
    * @return false The pin was not in the valid range, and no action was taken.
    */
   bool resetBit(const uint8_t &srPin);
   /**
    * @brief Sets the state of a pin to HIGH (0x01/Set) in the virtual port.
    * 
    * @param srPin The pin number whose state is to be set. The valid range is 0 <= srPin < _pinsQty.
    * @return true The pin was in the valid range and was set to HIGH (0x01/Set) in the Main Buffer.
    * @return false The pin was not in the valid range, and no action was taken.
    */
   bool setBit(const uint8_t &srPin);
   /**
    * @brief Sets the state of the pins in the Main Buffer (i.e. the GPIOXpander pins) according to the provided value.
    * 
    * The value is provided as an unsigned integer, where each bit represents the state of a pin in the virtual port. The bits are ordered from the least significant bit (LSB) to the most significant bit (MSB), where the LSB corresponds to pin 0 of the virtual port and the MSB corresponds to pin pinsQty - 1.
    * 
    * @param newPortVal The value to set the virtual port to, where each bit represents the state of a pin in the virtual port. The valid range is 0 <= newPortVal <= _vportMaxVal.
    * @return 
    */
   bool writePort(uint16_t newPortVal);
};

//==========================================================>>

#endif //ShiftRegGPIOXpander_ESP32_H_
