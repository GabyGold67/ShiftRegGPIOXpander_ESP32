#include <ShiftReg74HC595DsyChn.h>

// uint8_t* ShiftReg74HC595DsyChn::_srArryBuffPtr = nullptr;
// uint8_t ShiftReg74HC595DsyChn::_srQty = 0; 

ShiftReg74HC595DsyChn::ShiftReg74HC595DsyChn(uint8_t ds, uint8_t sh_cp, uint8_t st_cp, uint8_t srQty)
:_ds{ds}, _sh_cp{sh_cp}, _st_cp{st_cp}
{
   _srQty = srQty;

   digitalWrite(sh_cp, HIGH);
   digitalWrite(ds, LOW);
   digitalWrite(st_cp, HIGH);

   pinMode(sh_cp, OUTPUT);
   pinMode(ds, OUTPUT);
   pinMode(st_cp, OUTPUT);

   // _srArryBuffPtr = new uint8_t[_srQty]();
   for(int i{0}; i < _srQty; i++)
      // *(_srArryBuffPtr + i) = (uint8_t)0x00;
      *(_testBuffPtr + i) = (uint8_t)0x00; //FTPO static allocated memory
   _maxPin = _srQty * 8 - 1;
   sendBuffr();
}

ShiftReg74HC595DsyChn::~ShiftReg74HC595DsyChn(){
   delete [] _srArryBuffPtr;
}

uint8_t ShiftReg74HC595DsyChn::digitalRead(const uint8_t &pin){

   return *(_srArryBuffPtr + (pin / 8)) >> (pin % 8) & 0x01;
}

void ShiftReg74HC595DsyChn::digitalWrite(const uint8_t pin, const uint8_t value){
   digitalWriteBuff(pin, value);
   sendBuffr();

   return;
}

void ShiftReg74HC595DsyChn::digitalWriteAllReset(){
   for(uint8_t i{0}; i < _srQty; i++)
      // *(_srArryBuffPtr + i) = 0x00;
      *(_testBuffPtr + i) = 0x00;   //FTPO
   sendBuffr();

   return;
}

void ShiftReg74HC595DsyChn::digitalWriteAllSet(){
   for(uint8_t i{0}; i < _srQty; i++)
      // *(_srArryBuffPtr + i) = (uint8_t)0xFF;
      *(_testBuffPtr + i) = (uint8_t)0xFF; //FTPO
   sendBuffr();

   return;
}

void ShiftReg74HC595DsyChn::digitalWriteBuff(const uint8_t pin, const uint8_t value){
   if(value)
     *(_srArryBuffPtr + (pin / 8)) |= (0x01 << (pin % 8));
   else
     *(_srArryBuffPtr + (pin / 8)) &= ~(0x01 << (pin % 8));

  return;

}

void ShiftReg74HC595DsyChn::digitalWriteOver(const uint8_t* &newValues){
   memcpy( _srArryBuffPtr, newValues, _srQty);   // dest, src, size
   sendBuffr();

   return;
}

uint8_t* ShiftReg74HC595DsyChn::getArryBuffPtr(){

   return _srArryBuffPtr; 
}

uint8_t ShiftReg74HC595DsyChn::getMaxPin(){
   
   return _maxPin;
}

uint8_t ShiftReg74HC595DsyChn::getSrQty(){

   return _srQty;
}

void ShiftReg74HC595DsyChn::sendBuffr(){
   uint8_t content{0};

   digitalWrite(_st_cp, LOW); // Lower the latch pin

   for (int srPos{_srQty - 1}; srPos >= 0; srPos--) {
      // content = *(_srArryBuffPtr + srPos);
      content = *(_testBuffPtr + srPos); //FTPO

      for (int bitPos {7}; bitPos >= 0; bitPos--){   //Send each of the 8 bits representing the character
         if (content & 0x80)
             digitalWrite(_ds, HIGH);
         else
             digitalWrite(_ds, LOW);
         content <<= 1;
         digitalWrite(_sh_cp, LOW);
         digitalWrite(_sh_cp, HIGH);
     }
   }
   
   digitalWrite(_st_cp, HIGH);   // Rise the latch pin

   return;
}
