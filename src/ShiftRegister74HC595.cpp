#include <ShiftRegister74HC595.h>

ShiftRegister74HC595::ShiftRegister74HC595(uint8_t serialDataPin, uint8_t clockPin, uint8_t latchPin, uint8_t srQty)
:_serialDataPin{serialDataPin}, _clockPin{clockPin}, _latchPin{latchPin}, _srQty{srQty}
{
   digitalWrite(_clockPin, LOW);
   digitalWrite(_serialDataPin, LOW);
   digitalWrite(_latchPin, LOW);

   pinMode(_clockPin, OUTPUT);
   pinMode(_serialDataPin, OUTPUT);
   pinMode(_latchPin, OUTPUT);
   
   _srArryBuffPtr = new uint8_t[_srQty];
   for(uint8_t i{0}; i < _srQty; i++)
      *(_srArryBuffPtr + i) = 0x00;
   _maxPin = _srQty * 8 - 1;
   updShftRgstrs();
}

uint8_t ShiftRegister74HC595::digitalRead(const uint8_t &pin){

   return (_srArryBuffPtr[pin / 8] >> (pin % 8)) & 0x01;
}

uint8_t* ShiftRegister74HC595::getArryBuffPtr(){

   return _srArryBuffPtr; 
}

void ShiftRegister74HC595::digitalWrite(const uint8_t pin, const uint8_t value){
   digitalWriteBuff(pin, value);
   updShftRgstrs();

   return;
}

uint8_t ShiftRegister74HC595::getMaxPin(){
   
   return _maxPin;
}

uint8_t ShiftRegister74HC595::getSrQty(){

   return _srQty;
}

void ShiftRegister74HC595::digitalWriteOver(const uint8_t* digitalValues){
   memcpy( _srArryBuffPtr, digitalValues, _srQty);   // dest, src, size
   updShftRgstrs();

   return;
}

void ShiftRegister74HC595::digitalWriteAllReset(){
   for(uint8_t i{0}; i < _srQty; i++)
      *(_srArryBuffPtr + i) = 0x00;
   updShftRgstrs();

   return;
}

void ShiftRegister74HC595::digitalWriteAllSet(){
   for(uint8_t i{0}; i < _srQty; i++)
      *(_srArryBuffPtr + i) = 0xFF;
   updShftRgstrs();

   return;
}

void ShiftRegister74HC595::digitalWriteBuff(const uint8_t pin, const uint8_t value){
    if(value)
      *(_srArryBuffPtr + (pin/8)) |= (0x01 << (pin % 8));
    else
      *(_srArryBuffPtr + (pin/8)) &= ~(0x01 << (pin % 8));

   return;

}

void ShiftRegister74HC595::updShftRgstrs(){
    for (int i = _srQty - 1; i >= 0; i--) {
        shiftOut(_serialDataPin, _clockPin, MSBFIRST, _srArryBuffPtr[i]);
    }    
    digitalWrite(_latchPin, HIGH); 
    digitalWrite(_latchPin, LOW); 
}
