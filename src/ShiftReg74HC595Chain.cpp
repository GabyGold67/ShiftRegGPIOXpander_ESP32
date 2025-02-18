#include <ShiftReg74HC595Chain.h>

ShiftReg74HC595Chain::ShiftReg74HC595Chain(uint8_t ds, uint8_t sh_cp, uint8_t st_cp, uint8_t srQty)
:_ds{ds}, _sh_cp{sh_cp}, _st_cp{st_cp}, _srQty{srQty}
{
   Serial.println("Setting pins LOW"); //FTPO Delete after test

   digitalWrite(_sh_cp, LOW);
   digitalWrite(_ds, LOW);
   digitalWrite(_st_cp, LOW);

   Serial.println("Setting pins OUTPUT"); //FTPO Delete after test
   pinMode(_sh_cp, OUTPUT);
   pinMode(_ds, OUTPUT);
   pinMode(_st_cp, OUTPUT);

   _srArryBuffPtr = new uint8_t[_srQty];
   for(uint8_t i{0}; i < _srQty; i++)
      *(_srArryBuffPtr + i) = 0x00;
   _maxPin = _srQty * 8 - 1;
   sendBuffr();
}

uint8_t ShiftReg74HC595Chain::digitalRead(const uint8_t &pin){

   return (_srArryBuffPtr[pin / 8] >> (pin % 8)) & 0x01;
}

void ShiftReg74HC595Chain::digitalWrite(const uint8_t pin, const uint8_t value){
   digitalWriteBuff(pin, value);
   sendBuffr();

   return;
}

void ShiftReg74HC595Chain::digitalWriteAllReset(){
   for(uint8_t i{0}; i < _srQty; i++)
      *(_srArryBuffPtr + i) = 0x00;
      sendBuffr();

   return;
}

void ShiftReg74HC595Chain::digitalWriteAllSet(){
   for(uint8_t i{0}; i < _srQty; i++)
      *(_srArryBuffPtr + i) = 0xFF;
      sendBuffr();

   return;
}

void ShiftReg74HC595Chain::digitalWriteBuff(const uint8_t pin, const uint8_t value){
   if(value)
     *(_srArryBuffPtr + (pin/8)) |= (0x01 << (pin % 8));
   else
     *(_srArryBuffPtr + (pin/8)) &= ~(0x01 << (pin % 8));

  return;

}

void ShiftReg74HC595Chain::digitalWriteOver(const uint8_t* &newValues){
   memcpy( _srArryBuffPtr, newValues, _srQty);   // dest, src, size
   sendBuffr();

   return;
}

uint8_t* ShiftReg74HC595Chain::getArryBuffPtr(){

   return _srArryBuffPtr; 
}

uint8_t ShiftReg74HC595Chain::getMaxPin(){
   
   return _maxPin;
}

uint8_t ShiftReg74HC595Chain::getSrQty(){

   return _srQty;
}

/*
void ShiftReg74HC595Chain::updShftRgstrs(){
    for (int i = _srQty - 1; i >= 0; i--) {
        shiftOut(_ds, _sh_cp, MSBFIRST, *(_srArryBuffPtr + i));
    }    
    digitalWrite(_st_cp, HIGH); 
    digitalWrite(_st_cp, LOW); 
}
*/

void ShiftReg74HC595Chain::sendBuffr(){
   uint8_t content{0};

   digitalWrite(_st_cp, LOW); // Lower the latch pin

   for (int srPos = _srQty - 1; srPos >= 0; srPos--) {
      content = *(_srArryBuffPtr + srPos);

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

/*
void ShiftReg74HC595Chain::send(uint8_t content){
   for (int i {7}; i >= 0; i--){   //Send each of the 8 bits representing the character
       if (content & 0x80)
           digitalWrite(_ds, HIGH);
       else
           digitalWrite(_ds, LOW);
       content <<= 1;
       digitalWrite(_sh_cp, LOW);
       digitalWrite(_sh_cp, HIGH);
   }

   return;
}
*/

/*
void ShiftReg74HC595Chain::send(const uint8_t &segments, const uint8_t &port){
   digitalWrite(_st_cp, LOW);
   send(segments);
   send(port);
   digitalWrite(_st_cp, HIGH);

  return;
}
*/