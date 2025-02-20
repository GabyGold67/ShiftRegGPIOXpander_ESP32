#include <Arduino.h>
#include <ShiftRegGPIOXtender.h>

uint8_t* ShiftRegGPIOXtender::_srArryBuffPtr = nullptr;

ShiftRegGPIOXtender::ShiftRegGPIOXtender()
{
}

ShiftRegGPIOXtender::ShiftRegGPIOXtender(uint8_t ds, uint8_t sh_cp, uint8_t st_cp, uint8_t srQty)
:_ds{ds}, _sh_cp{sh_cp}, _st_cp{st_cp}, _srQty{srQty}
{
   digitalWrite(_sh_cp, HIGH);
   digitalWrite(_ds, LOW);
   digitalWrite(_st_cp, HIGH);

   pinMode(_sh_cp, OUTPUT);
   pinMode(_ds, OUTPUT);
   pinMode(_st_cp, OUTPUT);

   _srArryBuffPtr = new uint8_t [_srQty]();
   _maxPin = (sizeof(uint8_t) * 8) - 1;

   for(int i{0}; i < _srQty; i++){
      *(_srArryBuffPtr + i) = (uint8_t)0x00;
   }


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

void ShiftRegGPIOXtender::digitalWrite(const uint8_t pin, const uint8_t value){
   *(_srArryBuffPtr + 0) = 0b11111111; //FTPO
   // *(_srArryBuffPtr + 1) = 0b10101010; //FTPO
   // *(_srArryBuffPtr + 2) = 0b01010101; //FTPO
   // *(_srArryBuffPtr + 3) = 0b00010001; //FTPO

   sendAllSRCntnt();

   return;
}

bool ShiftRegGPIOXtender::sendAllSRCntnt(){
   uint8_t curSRcntnt{0};
   bool result{true};

   if(_srQty > 0){
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
         // delayMicroseconds(10);         
      digitalWrite(_sh_cp, HIGH);   // End of next bit value addition to the shift register internal buffer -> Lower the clock pin
   }

   return result;
}
