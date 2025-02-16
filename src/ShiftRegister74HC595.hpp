template<uint8_t Size>
ShiftRegister74HC595<Size>::ShiftRegister74HC595(const uint8_t serialDataPin, const uint8_t clockPin, const uint8_t latchPin)
:_clockPin{clockPin}, _serialDataPin {serialDataPin}, _latchPin{latchPin}
{
   // set pins low
   digitalWrite(_clockPin, LOW);
   digitalWrite(_serialDataPin, LOW);
   digitalWrite(_latchPin, LOW);

   // define pins as outputs
   pinMode(_clockPin, OUTPUT);
   pinMode(_serialDataPin, OUTPUT);
   pinMode(_latchPin, OUTPUT);
   
   memset(_digitalValues, 0, Size * sizeof(uint8_t)); // allocates the specified number of bytes and initializes them to zero
   updateRegisters();   // reset shift register
}

template<uint8_t Size>
uint8_t ShiftRegister74HC595<Size>::get(const uint8_t pin){
   return (_digitalValues[pin / 8] >> (pin % 8)) & 1;
}

template<uint8_t Size>
uint8_t* ShiftRegister74HC595<Size>::getAll(){
    return _digitalValues; 
}

template<uint8_t Size>
void ShiftRegister74HC595<Size>::set(const uint8_t pin, const uint8_t value){
   setNoUpdate(pin, value);
   updateRegisters();
}

template<uint8_t Size>
void ShiftRegister74HC595<Size>::setAll(const uint8_t* digitalValues){
   memcpy( _digitalValues, digitalValues, Size);   // dest, src, size
   updateRegisters();
}

template<uint8_t Size>
void ShiftRegister74HC595<Size>::setAllLow(){
    for (int i = 0; i < Size; i++) {
        _digitalValues[i] = 0;
    }
    updateRegisters();
}

template<uint8_t Size>
void ShiftRegister74HC595<Size>::setAllHigh(){
    for (int i = 0; i < Size; i++) {
        _digitalValues[i] = 0xFF;
    }
    updateRegisters();
}

template<uint8_t Size>
void ShiftRegister74HC595<Size>::setNoUpdate(const uint8_t pin, const uint8_t value){
    (value) ? bitSet(_digitalValues[pin / 8], pin % 8) : bitClear(_digitalValues[pin / 8], pin % 8);
}

template<uint8_t Size>
void ShiftRegister74HC595<Size>::updateRegisters(){
    for (int i = Size - 1; i >= 0; i--) {
        shiftOut(_serialDataPin, _clockPin, MSBFIRST, _digitalValues[i]);
    }    
    digitalWrite(_latchPin, HIGH); 
    digitalWrite(_latchPin, LOW); 
}
