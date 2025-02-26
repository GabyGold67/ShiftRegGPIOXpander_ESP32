/**
 * @file ShiftRegGPIOXtender.cpp
 * @brief Code file for the ShiftRegGPIOExtend_ESP32 library 
 * 
 * @author Gabriel D. Goldman
 * 
 * @version 1.0.0
 * 
 * @date First release: 12/02/2025 
 *       Last update:   24/02/2025 16:30 (GMT+0200)
 * 
 * @copyright Copyright (c) 2025
 *******************************************************************************
 * @attention 
 * @warning 
 *******************************************************************************
 */
#include <Arduino.h>
#include <ShiftRegGPIOXtender.h>

ShiftRegGPIOXtender::ShiftRegGPIOXtender()
{
}

ShiftRegGPIOXtender::ShiftRegGPIOXtender(uint8_t ds, uint8_t sh_cp, uint8_t st_cp, uint8_t srQty)
:_ds{ds}, _sh_cp{sh_cp}, _st_cp{st_cp}, _srQty{srQty}, _srArryBuffPtr {new uint8_t [srQty]}
{
   digitalWrite(_sh_cp, HIGH);
   digitalWrite(_ds, LOW);
   digitalWrite(_st_cp, HIGH);

   pinMode(_sh_cp, OUTPUT);
   pinMode(_ds, OUTPUT);
   pinMode(_st_cp, OUTPUT);

   _maxPin = (_srQty * 8) - 1;

   //-------------------->> Section that migh be replaced by a digitalWritteAllReset BEGIN
   for(int i{0}; i < _srQty; i++){  //TODO This implementation forces the Buffer to be started with all output pins set to LOW/0x00 and flush the buffer to the physical pins. Maybe the initial value might be a configurable parameter to avoid erroneous activation of LOW level activation devices, like some relays? Just like the MCU, you first select it's inital value, then you set the pins as outputs!! The initial value to set then would be passed as a construction parameter, a pointer to an array of values to put at instantiation time
      *(_srArryBuffPtr + i) = 0x00;
   }
   sendAllSRCntnt();
   //---------------------->> Section that migh be replaced by a digitalWritteAllReset END
}

ShiftRegGPIOXtender::~ShiftRegGPIOXtender(){
   if(_auxArryBuffPtr !=nullptr){
      delete [] _auxArryBuffPtr;
      _auxArryBuffPtr = nullptr;
   }
   if(_srArryBuffPtr !=nullptr){
      delete [] _srArryBuffPtr;
      _srArryBuffPtr = nullptr;
   }
}

bool ShiftRegGPIOXtender::copyMainToAux(bool overWriteIfExists){
   bool result {false};
   
   if((_auxArryBuffPtr == nullptr) || overWriteIfExists){
      if(_auxArryBuffPtr == nullptr){
         _auxArryBuffPtr = new uint8_t [_srQty];
      }
      memcpy(_auxArryBuffPtr, _srArryBuffPtr, _srQty);   // destPtr, srcPtr, size
      result = true;
   }

   return result;
}

void ShiftRegGPIOXtender::deleteAuxBuff(){
   if(_auxArryBuffPtr != nullptr){
      delete [] _auxArryBuffPtr;
      _auxArryBuffPtr = nullptr;
   }

   return;
}

uint8_t ShiftRegGPIOXtender::digitalRead(const uint8_t &pin){
   uint8_t result{0xFF};

   if(pin <= _maxPin){
      if(_auxArryBuffPtr != nullptr){
         moveAuxToMain(true);
      }   

      result = (*(_srArryBuffPtr + (pin / 8)) >> (pin % 8)) & 0x01;
   }

   return result;
}

void ShiftRegGPIOXtender::digitalWrite(const uint8_t pin, const uint8_t value){
   if(pin <= _maxPin){
      if(_auxArryBuffPtr != nullptr)
         moveAuxToMain(false);
      if(value)
         *(_srArryBuffPtr + (pin / 8)) |= (0x01 << (pin % 8));
      else
         *(_srArryBuffPtr + (pin / 8)) &= ~(0x01 << (pin % 8));
   }
   sendAllSRCntnt();

   return;
}

void ShiftRegGPIOXtender::digitalWriteAllReset(){
   if(_auxArryBuffPtr != nullptr)
      deleteAuxBuff();
   for(uint8_t i{0}; i < _srQty; i++)
      *(_srArryBuffPtr + i) = 0x00;
   sendAllSRCntnt();

   return;
}

void ShiftRegGPIOXtender::digitalWriteAllSet(){
   if(_auxArryBuffPtr != nullptr)
      deleteAuxBuff();
   for(uint8_t i{0}; i < _srQty; i++)
      *(_srArryBuffPtr + i) = (uint8_t)0xFF;
   sendAllSRCntnt();

   return;
}

void ShiftRegGPIOXtender::digitalWriteToAux(const uint8_t pin, const uint8_t value){
   if(pin <= _maxPin){
      if(_auxArryBuffPtr == nullptr){
         copyMainToAux();
      }
      if(value)
         *(_auxArryBuffPtr + (pin / 8)) |= (0x01 << (pin % 8));
      else
         *(_auxArryBuffPtr + (pin / 8)) &= ~(0x01 << (pin % 8));

   }

   return;
}

uint8_t* ShiftRegGPIOXtender::getMainBuffPtr(){

   return _srArryBuffPtr;
}

uint8_t ShiftRegGPIOXtender::getMaxPin(){

   return _maxPin;
}

uint8_t ShiftRegGPIOXtender::getSrQty(){

   return _srQty;
}

bool ShiftRegGPIOXtender::moveAuxToMain(bool flushASAP){
   bool result {false};

   if(_auxArryBuffPtr != nullptr){
      memcpy( _srArryBuffPtr, _auxArryBuffPtr, _srQty);   // destPtr, srcPtr, size
      deleteAuxBuff();
      if(flushASAP)
         sendAllSRCntnt();
   }

   return result;
}

bool ShiftRegGPIOXtender::sendAllSRCntnt(){
   uint8_t curSRcntnt{0};
   bool result{true};

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

   return result;
}

bool ShiftRegGPIOXtender::_sendSnglSRCntnt(uint8_t data){
   bool result{true};

   for (int bitPos {7}; bitPos >= 0; bitPos--){   //Send each of the bits correspondig to one 8-bits shift register module
      digitalWrite(_sh_cp, LOW); // Start of next bit value addition to the shift register internal buffer -> Lower the clock pin         
      digitalWrite(_ds, (data & 0x80)?HIGH:LOW);   // Set the value of the next bit value
      data <<= 1;
      delayMicroseconds(10);  // Time required by the 74HCx595 to modify the SH_CP line by datasheet
      digitalWrite(_sh_cp, HIGH);   // End of next bit value addition to the shift register internal buffer -> Lower the clock pin
   }

   return result;
}
