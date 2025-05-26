/**
 ******************************************************************************
 * @file ShiftRegGPIOXpander_ESP32.cpp
 * @brief Code file for the ShiftRegGPIOXtender_ESP32 library 
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
 * @version 2.1.0
 * 
 * @date First release: 12/02/2025  
 *       Last update:   22/05/2025 17:50 (GMT+0200) DST  
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
#include <Arduino.h>
#include <ShiftRegGPIOXpander_ESP32.h>

ShiftRegGPIOXpander::ShiftRegGPIOXpander()
{
}

ShiftRegGPIOXpander::ShiftRegGPIOXpander(uint8_t ds, uint8_t sh_cp, uint8_t st_cp, uint8_t srQty)
:_ds{ds}, _sh_cp{sh_cp}, _st_cp{st_cp}, _srQty{srQty}, _srArryBuffPtr {new uint8_t [srQty]}
{
   _maxSrPin = (_srQty * 8) - 1;
}

ShiftRegGPIOXpander::~ShiftRegGPIOXpander(){
   end();
   if(_auxArryBuffPtr !=nullptr){
      delete [] _auxArryBuffPtr;
      _auxArryBuffPtr = nullptr;
   }
   if(_srArryBuffPtr !=nullptr){
      delete [] _srArryBuffPtr;
      _srArryBuffPtr = nullptr;
   }
}

void ShiftRegGPIOXpander::begin(uint8_t* initCntnt){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   digitalWrite(_sh_cp, HIGH);
   digitalWrite(_ds, LOW);
   digitalWrite(_st_cp, HIGH);
   pinMode(_sh_cp, OUTPUT);
   pinMode(_ds, OUTPUT);
   pinMode(_st_cp, OUTPUT);

   taskENTER_CRITICAL(&mux);
   if(initCntnt != nullptr)
      memcpy(_srArryBuffPtr, initCntnt, _srQty);
   else
      memset(_srArryBuffPtr,0x00, _srQty);
   sendAllSRCntnt();
   taskEXIT_CRITICAL(&mux);
}

bool ShiftRegGPIOXpander::copyMainToAux(const bool &overWriteIfExists){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
   bool result {false};
   
   if((_auxArryBuffPtr == nullptr) || overWriteIfExists){
      taskENTER_CRITICAL(&mux);
      if(_auxArryBuffPtr == nullptr)
         _auxArryBuffPtr = new uint8_t [_srQty];
      memcpy(_auxArryBuffPtr, _srArryBuffPtr, _srQty);
      taskEXIT_CRITICAL(&mux);
      result = true;
   }

   return result;
}

SRGXVPort ShiftRegGPIOXpander::createSRGXVPort(const uint8_t &strtPin, const uint8_t &pinsQty){
   if((strtPin <= _maxSrPin) && ((strtPin + pinsQty - 1) <= _maxSrPin))   
      return SRGXVPort(this, strtPin, pinsQty);
   else
      return SRGXVPort();
}

uint8_t ShiftRegGPIOXpander::digitalReadSr(const uint8_t &srPin){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
   uint8_t result{0xFF};

   if(srPin <= _maxSrPin){
      taskENTER_CRITICAL(&mux);
      if(_auxArryBuffPtr != nullptr)
         moveAuxToMain(true);
      result = (*(_srArryBuffPtr + (srPin / 8)) >> (srPin % 8)) & 0x01;
      taskEXIT_CRITICAL(&mux);
   }

   return result;
}

void ShiftRegGPIOXpander::digitalToggleSr(const uint8_t &srPin){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   if(srPin <= _maxSrPin){
      taskENTER_CRITICAL(&mux);
      if(_auxArryBuffPtr != nullptr)
         moveAuxToMain(false);
      *(_srArryBuffPtr + (srPin / 8)) ^= (0x01 << (srPin % 8));
      sendAllSRCntnt();
      taskEXIT_CRITICAL(&mux);
   }

   return;
}

void ShiftRegGPIOXpander::digitalToggleSrAll(){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   taskENTER_CRITICAL(&mux);
   if(_auxArryBuffPtr != nullptr)
      moveAuxToMain(false);
   for (int ptrInc{0}; ptrInc < _srQty; ptrInc++)
      *(_srArryBuffPtr + ptrInc) ^= 0xFF;
   sendAllSRCntnt();
   taskEXIT_CRITICAL(&mux);

   return;
}

void ShiftRegGPIOXpander::digitalToggleSrMask(uint8_t *newToggleMask)
{
   if(newToggleMask != nullptr){
      if(_auxArryBuffPtr != nullptr)
         moveAuxToMain(false);
      for (int ptrInc{0}; ptrInc < _srQty; ptrInc++)
         *(_srArryBuffPtr + ptrInc) ^= *(newToggleMask + ptrInc);
      sendAllSRCntnt();
   }

   return;
}

void ShiftRegGPIOXpander::digitalToggleSrToAux(const uint8_t &srPin){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   if(srPin <= _maxSrPin){
      taskENTER_CRITICAL(&mux);
      if(_auxArryBuffPtr == nullptr)
         copyMainToAux();
      *(_auxArryBuffPtr + (srPin / 8)) ^= (0x01 << (srPin % 8));
      taskEXIT_CRITICAL(&mux);
   }

   return;
}

void ShiftRegGPIOXpander::digitalWriteSr(const uint8_t &srPin, const uint8_t &value){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   if(srPin <= _maxSrPin){
      taskENTER_CRITICAL(&mux);
      if(_auxArryBuffPtr != nullptr)
         moveAuxToMain(false);
      if(value)
         *(_srArryBuffPtr + (srPin / 8)) |= (0x01 << (srPin % 8));
      else
         *(_srArryBuffPtr + (srPin / 8)) &= ~(0x01 << (srPin % 8));
      sendAllSRCntnt();
      taskEXIT_CRITICAL(&mux);
   }

   return;
}

void ShiftRegGPIOXpander::digitalWriteSrAllReset(){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   taskENTER_CRITICAL(&mux);
   if(_auxArryBuffPtr != nullptr)   //!< Altough the discardAux() method makes this check, it is better to do it here to avoid unnecessary calls to the method
      discardAux();
   memset(_srArryBuffPtr,0x00, _srQty);
   sendAllSRCntnt();
   taskEXIT_CRITICAL(&mux);

   return;
}

void ShiftRegGPIOXpander::digitalWriteSrAllSet(){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   taskENTER_CRITICAL(&mux);
   if(_auxArryBuffPtr != nullptr)   //!< Altough the discardAux() method makes this check, it is better to do it here to avoid unnecessary calls to the method
      discardAux();
   memset(_srArryBuffPtr,0xFF, _srQty);
   sendAllSRCntnt();
   taskEXIT_CRITICAL(&mux);

   return;
}

void ShiftRegGPIOXpander::digitalWriteSrMaskReset(uint8_t* newResetMask){
   if(newResetMask != nullptr){
      if(_auxArryBuffPtr != nullptr)
         moveAuxToMain(false);
      for (int ptrInc{0}; ptrInc < _srQty; ptrInc++)
         *(_srArryBuffPtr + ptrInc) &= ~(*(newResetMask + ptrInc));
      sendAllSRCntnt();
   }

   return;
}

void ShiftRegGPIOXpander::digitalWriteSrMaskSet(uint8_t* newSetMask){
   if(newSetMask != nullptr){
      if(_auxArryBuffPtr != nullptr)
         moveAuxToMain(false);
      for (int ptrInc{0}; ptrInc < _srQty; ptrInc++){
         *(_srArryBuffPtr + ptrInc) |= *(newSetMask + ptrInc);
      }
      sendAllSRCntnt();
   }

   return;
}

void ShiftRegGPIOXpander::digitalWriteSrToAux(const uint8_t srPin, const uint8_t value){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   if(srPin <= _maxSrPin){
      taskENTER_CRITICAL(&mux);
      if(_auxArryBuffPtr == nullptr)
         copyMainToAux();
      if(value)
         *(_auxArryBuffPtr + (srPin / 8)) |= (0x01 << (srPin % 8));
      else
         *(_auxArryBuffPtr + (srPin / 8)) &= ~(0x01 << (srPin % 8));
      taskEXIT_CRITICAL(&mux);
   }

   return;
}

void ShiftRegGPIOXpander::discardAux(){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   taskENTER_CRITICAL(&mux);
   if(_auxArryBuffPtr != nullptr){
      delete [] _auxArryBuffPtr;
      _auxArryBuffPtr = nullptr;
   }
   taskEXIT_CRITICAL(&mux);

   return;
}

void ShiftRegGPIOXpander::end(){

   return;
}

bool ShiftRegGPIOXpander::flipBit(const uint8_t &srPin){
   bool result{false};

   if(srPin <= _maxSrPin){
      digitalToggleSr(srPin); // Toggle the pin state at position srPin
   }
   else
      result = false;

   return result;
}

uint8_t* ShiftRegGPIOXpander::getMainBuffPtr(){

   return _srArryBuffPtr;
}

uint8_t ShiftRegGPIOXpander::getMaxPin(){

   return _maxSrPin;
}

uint8_t ShiftRegGPIOXpander::getSrQty(){

   return _srQty;
}

bool ShiftRegGPIOXpander::isValid(SRGXVPort &VPort){

   return (VPort.getSRGXPtr() != nullptr);
}

bool ShiftRegGPIOXpander::moveAuxToMain(const bool &flushASAP){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
   bool result {false};

   if(_auxArryBuffPtr != nullptr){
      taskENTER_CRITICAL(&mux);
      memcpy( _srArryBuffPtr, _auxArryBuffPtr, _srQty);
      discardAux();
      if(flushASAP)
         sendAllSRCntnt();
	   taskEXIT_CRITICAL(&mux);
   }

   return result;
}

bool ShiftRegGPIOXpander::resetBit(const uint8_t &srPin){
   bool result{false};

   if(srPin <= _maxSrPin){
      digitalWriteSr(srPin, LOW); // Set the pin to LOW
   }
   else
      result = false;

   return result;
}

bool ShiftRegGPIOXpander::sendAllSRCntnt(){
   uint8_t curSRcntnt{0};
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
   bool result{true};

	taskENTER_CRITICAL(&mux);
   if((_srQty > 0) && (_srArryBuffPtr != nullptr)){
      digitalWrite(_st_cp, LOW); // Start of access to the shift register internal buffer to write -> Lower the latch pin

      for(int srBuffDsplcPtr{_srQty - 1}; srBuffDsplcPtr >= 0; srBuffDsplcPtr--){
         curSRcntnt = *(_srArryBuffPtr + srBuffDsplcPtr);
         result = _sendSnglSRCntnt(curSRcntnt);
      }
      digitalWrite(_st_cp, HIGH);   // End of access to the shift register internal buffer, copy the buffer values to the output pins -> Lower the latch pin
   }
   else{
      result = false;
   }      
	taskEXIT_CRITICAL(&mux);

   return result;
}

bool ShiftRegGPIOXpander::_sendSnglSRCntnt(const uint8_t &data){  
   uint8_t  mask{0x80};
   bool result{true};

   for (int bitPos {7}; bitPos >= 0; bitPos--){   //Send each of the bits correspondig to one 8-bits shift register module
      digitalWrite(_sh_cp, LOW); // Start of next bit value addition to the shift register internal buffer -> Lower the clock pin         
      digitalWrite(_ds, (data & mask)?HIGH:LOW);
      // data <<= 1;
      mask >>= 1; // Shift the mask to the right to get the next bit value
      delayMicroseconds(10);  // Time required by the 74HCx595 to modify the SH_CP line by datasheet  //FFDR esp_timer_get_time() might be used instead of delayMicroseconds
      digitalWrite(_sh_cp, HIGH);   // End of next bit value addition to the shift register internal buffer -> Lower the clock pin      
   }

   return result;
}

bool ShiftRegGPIOXpander::setBit(const uint8_t &srPin){
   bool result{false};

   if(srPin <= _maxSrPin){
      digitalWriteSr(srPin, HIGH); // Set the pin to HIGH
   }
   else
      result = false;

   return result;
}

bool ShiftRegGPIOXpander::stampMaskOverMain(uint8_t* newMaskPtr, uint8_t* newValsPtr){
   bool mainBitState{false};
   bool maskBitState{false};
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
   bool newValBitState{false};
   bool result{false};  
   
   if((newMaskPtr != nullptr) && (newValsPtr != nullptr)){
      taskENTER_CRITICAL(&mux);
      if(_auxArryBuffPtr != nullptr)
         moveAuxToMain(false); // Move the Auxiliary Buffer to the Main Buffer, if it exists
      
      for (int ptrInc{0}; ptrInc < _maxSrPin; ptrInc++){
         if(*(newMaskPtr + (ptrInc / 8)) & (static_cast<uint8_t>(0x01) << (ptrInc % 8))){  // If the bit is set in the mask check if the bit state in the Main needs to be changed

            /*if(*(_srArryBuffPtr + (ptrInc / 8)) & (static_cast<uint8_t>(0x01) << (ptrInc % 8)))
               mainBitState = true;
            else
               mainBitState = false;

            if(*(newValsPtr + (ptrInc / 8)) & (0x01 << (static_cast<uint8_t>(0x01) % 8)))
               newValBitState = true;
            else
               newValBitState = false;

            if(mainBitState != newValBitState) // If the bit state in the new values is different from the bit state in the Main Buffer
               *(_srArryBuffPtr + (ptrInc / 8)) ^= (static_cast<uint8_t>(0x01) << (ptrInc % 8));
            */

            if((*(_srArryBuffPtr + (ptrInc / 8)) & (static_cast<uint8_t>(0x01) << (ptrInc % 8))) != (*(newValsPtr + (ptrInc / 8)) & (0x01 << (static_cast<uint8_t>(0x01) % 8))))
               *(_srArryBuffPtr + (ptrInc / 8)) ^= (static_cast<uint8_t>(0x01) << (ptrInc % 8));
         }
      }
      taskEXIT_CRITICAL(&mux);
      
      sendAllSRCntnt(); // Flush the Main Buffer to the shift registers
      result = true; // If the parameters were valid, the operation was successful
   }

   return result;
}

bool ShiftRegGPIOXpander::stampOverMain(uint8_t* newCntntPtr){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
   bool result {false};

   if ((newCntntPtr != nullptr) && (newCntntPtr != NULL)){
      taskENTER_CRITICAL(&mux);
      if(_auxArryBuffPtr != nullptr)
         discardAux();
      memcpy(_srArryBuffPtr, newCntntPtr, _srQty);
      sendAllSRCntnt();
      result = true;
      taskEXIT_CRITICAL(&mux);
   }
   
   return result;
}

bool ShiftRegGPIOXpander::stampSgmntOverMain(uint8_t *newSgmntPtr, const uint8_t &strtPin, const uint8_t &pinsQty){
   bool mainBitState{false};
   bool result{false};  
   bool sgmntBitState{false};
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   if((newSgmntPtr != nullptr) && (strtPin <= _maxSrPin) && (pinsQty > 0) && ((strtPin + pinsQty - 1) <= _maxSrPin)){
      taskENTER_CRITICAL(&mux);
      if(_auxArryBuffPtr != nullptr)
         moveAuxToMain(false); // Move the Auxiliary Buffer to the Main Buffer, if it exists
      
      for (int ptrInc{0}; ptrInc < pinsQty; ptrInc++){
         if(*(_srArryBuffPtr + ((strtPin + ptrInc) / 8)) & (0x01 << ((strtPin + ptrInc) % 8)))
            mainBitState = true; // The bit is set in the Main Buffer
         else
            mainBitState = false; // The bit is reset in the Main Buffer            

         if(*(newSgmntPtr + (ptrInc / 8)) & (0x01 << (ptrInc % 8)))
            sgmntBitState = true; // The bit is set in the segment
         else
            sgmntBitState = false; // The bit is reset in the segment
         
         if(sgmntBitState != mainBitState) // If the bit state in the segment is different from the bit state in the Main Buffer
            *(_srArryBuffPtr + ((strtPin + ptrInc) / 8)) ^= (0x01 << ((strtPin + ptrInc) % 8));
      }

      sendAllSRCntnt();
      result = true;
      taskEXIT_CRITICAL(&mux);
   }

   return result;
}

//=========================================================================> Class methods delimiter

SRGXVPort::SRGXVPort()
{
}

SRGXVPort::SRGXVPort(ShiftRegGPIOXpander* SRGXPtr, uint8_t strtPin, uint8_t pinsQty) 
:_srGpioXpdrPtr{SRGXPtr}, _strtPin{strtPin}, _pinsQty{pinsQty}
{
   if(!((strtPin <= _maxSrPin) && ((strtPin + pinsQty - 1) <= _maxSrPin) && (pinsQty <= _maxPortPinsQty))){ //!< Failed conditions
      _srGpioXpdrPtr = nullptr;
      _strtPin = 0;
      _pinsQty = 0;
   }
   else{
      //FFDR Create a buffer the size of the ShiftRegGPIOXpander Main for the virtual port positions mask
      // FFDR Build the mask for the virtual port positions
      //FFDR Calculate the maximum value holdable by the virtual port
   }
}

SRGXVPort::~SRGXVPort()
{
}

void SRGXVPort::begin(uint8_t* initCntnt){
}
   
uint8_t SRGXVPort::digitalReadSr(const uint8_t &srPin){
   uint8_t result{0xFF};

   if(_srGpioXpdrPtr != nullptr){
      if((srPin >= 0) && (srPin < _pinsQty))
         result = _srGpioXpdrPtr->digitalReadSr(_strtPin + srPin);
   }

   return result; // Return an invalid value if the pin is out of range   
}

void SRGXVPort::digitalWriteSr(const uint8_t &srPin, const uint8_t &value){
   if(_srGpioXpdrPtr != nullptr){
      if((srPin >= 0) && (srPin < _pinsQty)){
         _srGpioXpdrPtr->digitalWriteSr(_strtPin + srPin, value);
      }
   }

   return;
}

bool SRGXVPort::flipBit(const uint8_t &srPin){
   bool result{false};

   if(_srGpioXpdrPtr != nullptr){
      if((srPin >= 0) && (srPin < _pinsQty)){
         result = _srGpioXpdrPtr->flipBit(_strtPin + srPin);
      }
   }

   return result;
}

ShiftRegGPIOXpander* SRGXVPort::getSRGXPtr(){

   return _srGpioXpdrPtr;
}

bool SRGXVPort::resetBit(const uint8_t &srPin){
   bool result{false};

   if(_srGpioXpdrPtr != nullptr){
      if((srPin >= 0) && (srPin < _pinsQty)){
         result = _srGpioXpdrPtr->resetBit(_strtPin + srPin);
      }
   }

   return result;
}

bool SRGXVPort::setBit(const uint8_t &srPin){
   bool result{false};

   if(_srGpioXpdrPtr != nullptr){
      if((srPin >= 0) && (srPin < _pinsQty)){
         result = _srGpioXpdrPtr->setBit(_strtPin + srPin);
      }
   }

   return result;
}
