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
 * @version 3.0.0
 * 
 * @date First release: 12/02/2025  
 *       Last update:   27/05/2025 22:20 (GMT+0200) DST  
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
:_ds{ds}, _sh_cp{sh_cp}, _st_cp{st_cp}, _srQty{srQty}, _mainBuffrArryPtr {new uint8_t [srQty]}
{
   _maxSrPin = (_srQty * 8) - 1;
}

ShiftRegGPIOXpander::~ShiftRegGPIOXpander(){
   end();
   if(_auxBuffrArryPtr !=nullptr){
      delete [] _auxBuffrArryPtr;
      _auxBuffrArryPtr = nullptr;
   }
   if(_mainBuffrArryPtr !=nullptr){
      delete [] _mainBuffrArryPtr;
      _mainBuffrArryPtr = nullptr;
   }
}

void ShiftRegGPIOXpander::begin(uint8_t* initCntnt){
   bool result{true};  //FFDR Not in use, reserved for future use

   digitalWrite(_sh_cp, HIGH);
   digitalWrite(_ds, LOW);
   digitalWrite(_st_cp, HIGH);
   pinMode(_sh_cp, OUTPUT);
   pinMode(_ds, OUTPUT);
   pinMode(_st_cp, OUTPUT);

   _SRGXMnBffrMutex = xSemaphoreCreateMutex();
   _SRGXAuxBffrMutex = xSemaphoreCreateMutex();

   if(_SRGXMnBffrMutex == nullptr || _SRGXAuxBffrMutex == nullptr){ 
      result = false;
   }

   if(result){
      if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
         if(initCntnt != nullptr)
            memcpy(_mainBuffrArryPtr, initCntnt, _srQty);
         else
            memset(_mainBuffrArryPtr,0x00, _srQty);
         _sendAllSRCntnt();
         xSemaphoreGive(_SRGXMnBffrMutex);
      }
   }
   return;
}

bool ShiftRegGPIOXpander::_copyMainToAux(const bool &overWriteIfExists){
   bool result {false};
   
   if((_auxBuffrArryPtr == nullptr) || overWriteIfExists){
      if(_auxBuffrArryPtr == nullptr)
         _auxBuffrArryPtr = new uint8_t [_srQty];
      memcpy(_auxBuffrArryPtr, _mainBuffrArryPtr, _srQty);
      result = true;
   }

   return result;
}

bool ShiftRegGPIOXpander::copyMainToAux(const bool &overWriteIfExists){
   bool result {false};
   
   if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
      if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
         if((_auxBuffrArryPtr == nullptr) || overWriteIfExists){
            if(_auxBuffrArryPtr == nullptr)
               _auxBuffrArryPtr = new uint8_t [_srQty];
            memcpy(_auxBuffrArryPtr, _mainBuffrArryPtr, _srQty);
            result = true;
         }
         xSemaphoreGive(_SRGXAuxBffrMutex);
         xSemaphoreGive(_SRGXMnBffrMutex);
      }
   }

   return result;
}

SRGXVPort ShiftRegGPIOXpander::createSRGXVPort(const uint8_t &strtPin, const uint8_t &pinsQty){
   if((pinsQty > 0) && ((strtPin + pinsQty - 1) <= _maxSrPin) && (pinsQty <= SRGXVPort::_maxPortPinsQty))
      return SRGXVPort(this, strtPin, pinsQty);
   else
      return SRGXVPort();
}

uint8_t ShiftRegGPIOXpander::digitalReadSr(const uint8_t &srPin){
   uint8_t result{0xFF};

   if(srPin <= _maxSrPin){
      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){         
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
            if(_auxBuffrArryPtr != nullptr)
               _moveAuxToMain();
            xSemaphoreGive(_SRGXAuxBffrMutex);
            result = (*(_mainBuffrArryPtr + (srPin / 8)) >> (srPin % 8)) & 0x01;
            xSemaphoreGive(_SRGXMnBffrMutex);
         }
      }
   }

   return result;
}

void ShiftRegGPIOXpander::digitalToggleSr(const uint8_t &srPin){
   if(srPin <= _maxSrPin){
      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){         
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
            if(_auxBuffrArryPtr != nullptr)
               _moveAuxToMain();
            xSemaphoreGive(_SRGXAuxBffrMutex);
            *(_mainBuffrArryPtr + (srPin / 8)) ^= (0x01 << (srPin % 8));
            _sendAllSRCntnt();
            xSemaphoreGive(_SRGXMnBffrMutex);
         }
      }
   }

   return;
}

void ShiftRegGPIOXpander::digitalToggleSrAll(){
   if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){         
      if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
         if(_auxBuffrArryPtr != nullptr)
            _moveAuxToMain();
         xSemaphoreGive(_SRGXAuxBffrMutex);
         for (int ptrInc{0}; ptrInc < _srQty; ptrInc++)
            *(_mainBuffrArryPtr + ptrInc) ^= 0xFF;
         _sendAllSRCntnt();
         xSemaphoreGive(_SRGXMnBffrMutex);
      }
   }
   return;
}

void ShiftRegGPIOXpander::digitalToggleSrMask(uint8_t *toggleMask){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   if(toggleMask != nullptr){
      taskENTER_CRITICAL(&mux);  // Enter critical section to avoid any interrupt, including task switching, to avoid toggleMask being modified while the copy operation is being performed
      uint8_t* localToggleMask = new uint8_t[_srQty];
      memcpy(localToggleMask, toggleMask, _srQty);
      taskEXIT_CRITICAL(&mux);   // Exit critical section

      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){         
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
            if(_auxBuffrArryPtr != nullptr)
               _moveAuxToMain();
            xSemaphoreGive(_SRGXAuxBffrMutex);
            for (int ptrInc{0}; ptrInc < _srQty; ptrInc++)
               *(_mainBuffrArryPtr + ptrInc) ^= *(localToggleMask + ptrInc);
            _sendAllSRCntnt();
            xSemaphoreGive(_SRGXMnBffrMutex);
         }
      }
      delete [] localToggleMask;
   }

   return;
}

void ShiftRegGPIOXpander::digitalToggleSrToAux(const uint8_t &srPin){

   if(srPin <= _maxSrPin){
      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
            if(_auxBuffrArryPtr == nullptr)
               _copyMainToAux();
            xSemaphoreGive(_SRGXMnBffrMutex);            
         }
         *(_auxBuffrArryPtr + (srPin / 8)) ^= (0x01 << (srPin % 8));
         xSemaphoreGive(_SRGXAuxBffrMutex);
      }
   }

   return;
}

void ShiftRegGPIOXpander::digitalWriteSr(const uint8_t &srPin, const uint8_t &value){
   if(srPin <= _maxSrPin){
      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){         
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
            if(_auxBuffrArryPtr != nullptr)
               _moveAuxToMain();
            xSemaphoreGive(_SRGXAuxBffrMutex);
            if(value)
               *(_mainBuffrArryPtr + (srPin / 8)) |= (0x01 << (srPin % 8));
            else
               *(_mainBuffrArryPtr + (srPin / 8)) &= ~(0x01 << (srPin % 8));
            _sendAllSRCntnt();
            xSemaphoreGive(_SRGXMnBffrMutex);
         }
      }
   }

   return;
}

void ShiftRegGPIOXpander::digitalWriteSrAllReset(){
   if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
      if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
         if(_auxBuffrArryPtr != nullptr)   //!< Altough the discardAux() method makes this check, it is better to do it here to avoid unnecessary calls to the method
            _discardAux();
         xSemaphoreGive(_SRGXAuxBffrMutex);
         memset(_mainBuffrArryPtr,0x00, _srQty);
         _sendAllSRCntnt();
         xSemaphoreGive(_SRGXMnBffrMutex);
      }
   }

   return;
}

void ShiftRegGPIOXpander::digitalWriteSrAllSet(){
   if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
      if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
         if(_auxBuffrArryPtr != nullptr)   //!< Altough the discardAux() method makes this check, it is better to do it here to avoid unnecessary calls to the method
            _discardAux();
         xSemaphoreGive(_SRGXAuxBffrMutex);
         memset(_mainBuffrArryPtr,0xFF, _srQty);
         _sendAllSRCntnt();
         xSemaphoreGive(_SRGXMnBffrMutex);
      }
   }

   return;
}

void ShiftRegGPIOXpander::digitalWriteSrMaskReset(uint8_t* resetMask){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   if(resetMask != nullptr){
      taskENTER_CRITICAL(&mux);  // Enter critical section to avoid any interrupt, including task switching, to avoid resetMask being modified while the operation is being performed
      uint8_t* localResetMask = new uint8_t[_srQty];
      memcpy(localResetMask, resetMask, _srQty);
      taskEXIT_CRITICAL(&mux);   // Exit critical section

      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
            if(_auxBuffrArryPtr != nullptr)
               _moveAuxToMain();
            xSemaphoreGive(_SRGXAuxBffrMutex);
            for (int ptrInc{0}; ptrInc < _srQty; ptrInc++)
               *(_mainBuffrArryPtr + ptrInc) &= ~(*(localResetMask + ptrInc));
            _sendAllSRCntnt();
            delete [] localResetMask;
            xSemaphoreGive(_SRGXMnBffrMutex);
         }
      }
   }

   return;
}

void ShiftRegGPIOXpander::digitalWriteSrMaskSet(uint8_t* setMask){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

   if(setMask != nullptr){
      taskENTER_CRITICAL(&mux);  // Enter critical section to avoid any interrupt, including task switching, to avoid resetMask being modified while the operation is being performed
      uint8_t* localSetMask = new uint8_t[_srQty];
      memcpy(localSetMask, setMask, _srQty);
      taskEXIT_CRITICAL(&mux);   // Exit critical section

      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
            if(_auxBuffrArryPtr != nullptr)
               _moveAuxToMain();
            xSemaphoreGive(_SRGXAuxBffrMutex);
            for (int ptrInc{0}; ptrInc < _srQty; ptrInc++)
               *(_mainBuffrArryPtr + ptrInc) |= *(localSetMask + ptrInc);
            _sendAllSRCntnt();
            delete [] localSetMask;
            xSemaphoreGive(_SRGXMnBffrMutex);
         }
      }
   }

   return;
}

void ShiftRegGPIOXpander::digitalWriteSrToAux(const uint8_t srPin, const uint8_t value){
   if(srPin <= _maxSrPin){
      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
            if(_auxBuffrArryPtr == nullptr)
               _copyMainToAux();
            xSemaphoreGive(_SRGXMnBffrMutex);
            if(value)
               *(_auxBuffrArryPtr + (srPin / 8)) |= (0x01 << (srPin % 8));
            else
               *(_auxBuffrArryPtr + (srPin / 8)) &= ~(0x01 << (srPin % 8));
            xSemaphoreGive(_SRGXAuxBffrMutex);
         }
      }
   }

   return;
}

void ShiftRegGPIOXpander::_discardAux(){
   if(_auxBuffrArryPtr != nullptr){
      delete [] _auxBuffrArryPtr;
      _auxBuffrArryPtr = nullptr;
   }
   
   return;
}

void ShiftRegGPIOXpander::discardAux(){
   if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
      if(_auxBuffrArryPtr != nullptr){
         delete [] _auxBuffrArryPtr;
         _auxBuffrArryPtr = nullptr;
      }
      xSemaphoreGive(_SRGXAuxBffrMutex);
   }
   
   return;
}

void ShiftRegGPIOXpander::end(){

   return;
}

bool ShiftRegGPIOXpander::flipBit(const uint8_t &srPin){
   bool result{false};

   if(srPin <= _maxSrPin){
      digitalToggleSr(srPin); // Toggle the pin state at position srPin
      result = true;
   }

   return result;
}

bool ShiftRegGPIOXpander::_getZeroBasedMainBffrSgmnt(const uint8_t &strtPin, const uint8_t &pinsQty, uint16_t &bffrSgmnt){
   bool result{false};

   if((pinsQty > 0) && (pinsQty <= 16 ) && ((strtPin + pinsQty - 1) <= _maxSrPin)){
      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
            if(_auxBuffrArryPtr != nullptr)
               _moveAuxToMain();
            xSemaphoreGive(_SRGXAuxBffrMutex);
            bffrSgmnt = 0; // Initialize the result segment to zero
            for (int ptrInc{0}; ptrInc < pinsQty; ptrInc++){
               if(*(_mainBuffrArryPtr + ((strtPin + ptrInc) / 8)) & (static_cast<uint8_t>(0x01) << ((strtPin + ptrInc) % 8)))  // If the bit is set in Main then it needs to be set in the result segment
                  bffrSgmnt |= (static_cast<uint16_t>(0x01) << ptrInc); // Set the bit in the result segment
            }
            xSemaphoreGive(_SRGXMnBffrMutex);
            result = true;
         }
      }
   }

   return result;
}

uint8_t* ShiftRegGPIOXpander::getMainBuffPtr(){

   return _mainBuffrArryPtr;
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

bool ShiftRegGPIOXpander::_moveAuxToMain(){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
   bool result {false};

   if(_auxBuffrArryPtr != nullptr){
      taskENTER_CRITICAL(&mux);  // Enter critical section to avoid any interrupt, including task switching
      memcpy( _mainBuffrArryPtr, _auxBuffrArryPtr, _srQty);
      _discardAux();
      taskEXIT_CRITICAL(&mux);   // Exit critical section
      _sendAllSRCntnt();
      result = true;}

   return result;
}

bool ShiftRegGPIOXpander::moveAuxToMain(){
   bool result {false};

   if(_auxBuffrArryPtr != nullptr){
      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
            result = _moveAuxToMain(); 
            xSemaphoreGive(_SRGXAuxBffrMutex);
            xSemaphoreGive(_SRGXMnBffrMutex);
         }
      }
   }

   return result;
}

bool ShiftRegGPIOXpander::resetBit(const uint8_t &srPin){
   bool result{false};

   if(srPin <= _maxSrPin){
      digitalWriteSr(srPin, LOW); // Set the pin to LOW
      result = true;
   }

   return result;
}

bool ShiftRegGPIOXpander::_sendAllSRCntnt(){
   uint8_t curSRcntnt{0};
   bool result{false};

   if((_srQty > 0) && (_mainBuffrArryPtr != nullptr)){
      digitalWrite(_st_cp, LOW); // Start of access to the shift register internal buffer to write -> Lower the latch pin
      for(int srBuffDsplcPtr{_srQty - 1}; srBuffDsplcPtr >= 0; srBuffDsplcPtr--){
         curSRcntnt = *(_mainBuffrArryPtr + srBuffDsplcPtr);
         result = _sendSnglSRCntnt(curSRcntnt);
      }
      digitalWrite(_st_cp, HIGH);   // End of access to the shift register internal buffer, copy the buffer values to the output pins -> Lower the latch pin
      result = true;
   }

   return result;
}

bool ShiftRegGPIOXpander::_sendSnglSRCntnt(const uint8_t &data){  
   uint8_t mask{0x80};
   bool result{true};

   for (int bitPos {7}; bitPos >= 0; bitPos--){   //Send each of the bits correspondig to one 8-bits shift register module
      digitalWrite(_sh_cp, LOW); // Start of next bit value addition to the shift register internal buffer -> Lower the clock pin         
      digitalWrite(_ds, (data & mask)?HIGH:LOW);
      mask >>= 1; // Shift the mask to the right to get the next bit value
      delayMicroseconds(10);  // Time required by the 74HCx595 to modify the SH_CP line by datasheet
      /* 
      delayMicroseconds(10) Equivalent:
         uint64_t micros = esp_timer_get_time();
         while((esp_timer_get_time() - micros) < 10){}; // Wait for the time required by the 74HCx595 to modify the SH_CP line by datasheet
      */
      digitalWrite(_sh_cp, HIGH);   // End of next bit value addition to the shift register internal buffer -> Lower the clock pin      
   }

   return result;
}

bool ShiftRegGPIOXpander::setBit(const uint8_t &srPin){
   bool result{false};

   if(srPin <= _maxSrPin){
      digitalWriteSr(srPin, HIGH); // Set the pin to HIGH
      result = true;
   }

   return result;
}

bool ShiftRegGPIOXpander::stampMaskOverMain(uint8_t* maskPtr, uint8_t* valsPtr){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
   bool result{false};  

   if((maskPtr != nullptr) && (valsPtr != nullptr)){
      taskENTER_CRITICAL(&mux);  // Enter critical section to avoid any interrupt, including task switching
      uint8_t* localMaskPtr = new uint8_t[_srQty];
      memcpy(localMaskPtr, maskPtr, _srQty);
      uint8_t* localValsPtr = new uint8_t[_srQty];
      memcpy(localValsPtr, valsPtr, _srQty);
      taskEXIT_CRITICAL(&mux);   // Exit critical section

      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){
            if(_auxBuffrArryPtr != nullptr)
               _moveAuxToMain(); // Move the Auxiliary Buffer to the Main Buffer, if it exists      
            xSemaphoreGive(_SRGXAuxBffrMutex);
            for (int ptrInc{0}; ptrInc < _maxSrPin; ptrInc++){
               if(*(localMaskPtr + (ptrInc / 8)) & (static_cast<uint8_t>(0x01) << (ptrInc % 8))){  // If the bit is set in the mask check if the bit state in the Main needs to be changed
                  if((*(_mainBuffrArryPtr + (ptrInc / 8)) & (static_cast<uint8_t>(0x01) << (ptrInc % 8))) != (*(localValsPtr + (ptrInc / 8)) & (0x01 << (static_cast<uint8_t>(0x01) % 8))))
                     *(_mainBuffrArryPtr + (ptrInc / 8)) ^= (static_cast<uint8_t>(0x01) << (ptrInc % 8));
               } // If the bit is set in the mask, then check if the bit state in the Main Buffer needs to be changed
            }      
            _sendAllSRCntnt(); // Flush the Main Buffer to the shift registers
            xSemaphoreGive(_SRGXMnBffrMutex);
            delete [] localMaskPtr; // Free the memory allocated for the local mask
            delete [] localValsPtr; // Free the memory allocated for the local values
            result = true; // If the parameters were valid, the operation was successful
         }
      }
   }

   return result;
}

bool ShiftRegGPIOXpander::stampOverMain(uint8_t* newCntntPtr){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
   bool result {false};

   if ((newCntntPtr != nullptr) && (newCntntPtr != NULL)){
      taskENTER_CRITICAL(&mux);
      uint8_t* localNewCntntkPtr = new uint8_t[_srQty];
      memcpy(localNewCntntkPtr, newCntntPtr, _srQty);
      taskEXIT_CRITICAL(&mux);

      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){            
            if(_auxBuffrArryPtr != nullptr)
               _discardAux();
            xSemaphoreGive(_SRGXAuxBffrMutex);
            memcpy(_mainBuffrArryPtr, newCntntPtr, _srQty);
            _sendAllSRCntnt();
            delete [] localNewCntntkPtr; // Free the memory allocated for the local new content
            xSemaphoreGive(_SRGXMnBffrMutex);
         }
      }      
      result = true;
   }
   
   return result;
}

bool ShiftRegGPIOXpander::stampSgmntOverMain(uint8_t *newSgmntPtr, const uint8_t &strtPin, const uint8_t &pinsQty){
   bool result{false};  

   if((newSgmntPtr != nullptr) && (pinsQty > 0) && ((strtPin + pinsQty - 1) <= _maxSrPin)){
      if(xSemaphoreTake(_SRGXAuxBffrMutex, portMAX_DELAY) == pdTRUE){
         if(xSemaphoreTake(_SRGXMnBffrMutex, portMAX_DELAY) == pdTRUE){            
            if(_auxBuffrArryPtr != nullptr)
               _moveAuxToMain(); // Move the Auxiliary Buffer to the Main Buffer, if it exists
            xSemaphoreGive(_SRGXAuxBffrMutex);
            for (int ptrInc{0}; ptrInc < pinsQty; ptrInc++){
               if((*(_mainBuffrArryPtr + ((strtPin + ptrInc) / 8)) & (0x01 << ((strtPin + ptrInc) % 8))) != (*(newSgmntPtr + (ptrInc / 8)) & (0x01 << (ptrInc % 8)))) // If the bit state in the segment is different from the bit state in the Main Buffer
                  *(_mainBuffrArryPtr + ((strtPin + ptrInc) / 8)) ^= (0x1 << ((strtPin + ptrInc) % 8));
            }
            _sendAllSRCntnt();
            xSemaphoreGive(_SRGXMnBffrMutex);
            result = true;
         }
      }
   }

   return result;
}

//=========================================================================> Class methods delimiter

SRGXVPort::SRGXVPort()
{
}

SRGXVPort::SRGXVPort(ShiftRegGPIOXpander* SRGXPtr, uint8_t strtPin, uint8_t pinsQty) 
:_SRGXPtr{SRGXPtr}, _strtPin{strtPin}, _pinsQty{pinsQty}
{
   if(!((strtPin <= _maxSrPin) && ((strtPin + pinsQty - 1) <= _maxSrPin) && (pinsQty <= _maxPortPinsQty))){ //!< Failed conditions
      _SRGXPtr = nullptr;
      _strtPin = 0;
      _pinsQty = 0;
   }
   else
      _vportMaxVal = static_cast<int16_t>((1UL << _pinsQty) - 1); // Calculate the maximum value that can be set in the virtual port, as (2^pinsQty) - 1   
}

SRGXVPort::~SRGXVPort(){
   if(_srgxStampMskPtr != nullptr){ // If the mask pointer is not null, delete the mask
      delete [] _srgxStampMskPtr;
      _srgxStampMskPtr = nullptr;
   }
   _SRGXPtr = nullptr; // Just to make it visible, the pointer will be destructed as the object is destructed
}

bool SRGXVPort::begin(uint16_t initCntnt){
   bool result{false};

   if(!_begun){
      if(_SRGXPtr != nullptr){
         if(initCntnt <= _vportMaxVal){ 
            // uint8_t* initCntntPtr = reinterpret_cast<uint8_t*>(&initCntnt); // Cast the initCntnt to a pointer to uint8_t, this is safe for the ESP32 as it uses little-endian byte order            
            uint8_t* initCntntPtr = new uint8_t[2];
            initCntntPtr[0] = static_cast<uint8_t>(initCntnt & 0x00FF); // Get the least significant byte
            initCntntPtr[1] = static_cast<uint8_t>((initCntnt >> 8) & 0x00FF); // Get the most significant byte            
            _buildSRGXVPortMsk(_srgxStampMskPtr);
            stampSgmntOverMain(initCntntPtr, _strtPin, _pinsQty);
            delete [] initCntntPtr; // Free the memory allocated for the initCntnt pointer
            _begun = true; 
            result = true; // If the SRGXPtr is not null, the port was successfully initialized
         }
      }
   }

   return result;
}

bool SRGXVPort::_buildSRGXVPortMsk(uint8_t* &maskPtr){
   bool result{false};
   
      if(maskPtr == nullptr){
         maskPtr = new uint8_t[_srQty];
         memset(maskPtr, 0x00, _srQty); // Initialize the mask to 0x00
         for(uint8_t pinInc{_strtPin}; pinInc < (_strtPin + _pinsQty); pinInc++)
            *(maskPtr + (pinInc / 8)) |= (0x01 << (pinInc % 8)); // Set the bit in the mask for the virtual port position
         result = true;
      }

   return result;    
}

uint8_t SRGXVPort::digitalReadSr(const uint8_t &srPin){
   uint8_t result{0xFF};

   if(_SRGXPtr != nullptr){
      if(srPin < _pinsQty){
         result = _SRGXPtr->digitalReadSr(_strtPin + srPin);
      }
   }

   return result; // Return an invalid value if the pin is out of range   
}

void SRGXVPort::digitalWriteSr(const uint8_t &srPin, const uint8_t &value){
   if(_SRGXPtr != nullptr){
      if(srPin < _pinsQty)
         _SRGXPtr->digitalWriteSr(_strtPin + srPin, value);
   }

   return;
}

bool SRGXVPort::flipBit(const uint8_t &srPin){
   bool result{false};

   if(_SRGXPtr != nullptr){
      if(srPin < _pinsQty)
         result = _SRGXPtr->flipBit(_strtPin + srPin);
   }

   return result;
}

ShiftRegGPIOXpander* SRGXVPort::getSRGXPtr(){

   return _SRGXPtr;
}

uint16_t SRGXVPort::getVPortMaxVal(){

   return _vportMaxVal;
}

uint16_t SRGXVPort::readPort(){
   uint16_t portVal{0};
   bool result{false};

   if(_SRGXPtr != nullptr){
      portVal = _getZeroBasedMainBffrSgmnt(_strtPin, _pinsQty, portVal);
      result = true;
   }

   return result;
}

bool SRGXVPort::resetBit(const uint8_t &srPin){
   bool result{false};

   if(_SRGXPtr != nullptr){
      if(srPin < _pinsQty){
         result = _SRGXPtr->resetBit(_strtPin + srPin);
      }
   }

   return result;
}

bool SRGXVPort::setBit(const uint8_t &srPin){
   bool result{false};

   if(_SRGXPtr != nullptr){
      if(srPin < _pinsQty){
         result = _SRGXPtr->setBit(_strtPin + srPin);
      }
   }

   return result;
}

bool SRGXVPort::writePort(uint16_t portVal){
   bool result{false};
   uint8_t* portValPtr {nullptr};

   if(_SRGXPtr != nullptr){
      if(portVal <= _vportMaxVal){
         portValPtr = reinterpret_cast<uint8_t*>(&portVal); // Cast the portVal to a pointer to uint8_t
         result = _SRGXPtr->stampSgmntOverMain(portValPtr, _strtPin, _pinsQty); // Stamp the segment over the Main Buffer
      }
   }

   return result;
}