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

SRGVXVPort ShiftRegGPIOXpander::createVXVPort(uint8_t strtPin, uint8_t pinsQty){
   if((strtPin <= _maxSrPin) && ((strtPin + pinsQty - 1) <= _maxSrPin))   
      return SRGVXVPort(this, strtPin, pinsQty);
   else
      return SRGVXVPort();
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

uint8_t* ShiftRegGPIOXpander::getMainBuffPtr(){

   return _srArryBuffPtr;
}

uint8_t ShiftRegGPIOXpander::getMaxPin(){

   return _maxSrPin;
}

uint8_t ShiftRegGPIOXpander::getSrQty(){

   return _srQty;
}

bool ShiftRegGPIOXpander::isValid(SRGVXVPort &VPort){

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

//=========================================================================> Class methods delimiter

SRGVXVPort::SRGVXVPort()
{
}

SRGVXVPort::SRGVXVPort(ShiftRegGPIOXpander* SRGXPtr, uint8_t strtPin, uint8_t pinsQty) 
:_srGpioXpdrPtr{SRGXPtr}, _strtPin{strtPin}, _pinsQty{pinsQty}
{
   if(!((strtPin <= _maxSrPin) && ((strtPin + pinsQty - 1) <= _maxSrPin))){ //!< Failed conditions
      _srGpioXpdrPtr = nullptr;
      _strtPin = 0;
      _pinsQty = 0;
   }
}

SRGVXVPort::~SRGVXVPort()
{
}


ShiftRegGPIOXpander* SRGVXVPort::getSRGXPtr(){

   return _srGpioXpdrPtr;
}

