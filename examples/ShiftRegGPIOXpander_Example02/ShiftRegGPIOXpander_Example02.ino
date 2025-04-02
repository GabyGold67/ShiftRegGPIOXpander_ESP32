/**
  ******************************************************************************
  * @file	: ShiftRegGPIOXpander_Example02.cpp
  * @brief  : Code example of the use of the ShiftRegGPIOXpander_ESP32 library
  * 
  * Repository: https://github.com/GabyGold67/ShiftRegGPIOXpander_ESP32
  * Simulation url: https://wokwi.com/projects/423796255664866305
  * 
  * Framework: Arduino
  * Platform: ESP32
  * 
  * @author	: Gabriel D. Goldman
  * mail <gdgoldman67@hotmail.com>
  * Github <https://github.com/GabyGold67>
  *
  * @date First release: 16/02/2025 
  *       Last update:   02/04/2025 13:50 GMT+0200 DST
  ******************************************************************************
  * @warning **Use of this library is under your own responsibility**
  * 
  * @warning The use of this library falls in the category described by The Alan 
  * Parsons Project (c) 1980 "Games People play" disclaimer:
  * 
  * Games people play, you take it or you leave it
  * Things that they say aren't alright
  * If I promised you the moon and the stars, would you believe it?
  * 
  * Released into the public domain in accordance with "GPL-3.0-or-later" license terms.
  ******************************************************************************
*/
#include <Arduino.h>
#include <ShiftRegGPIOXpander.h>

//==============================================>> General use definitions BEGIN
//================================================>> General use definitions END
 
//======================================>> General use function prototypes BEGIN
void Error_Handler();
//========================================>> General use function prototypes END
 
//====================================>> Task Callback function prototypes BEGIN
//======================================>> Task Callback function prototypes END
 
//===========================================>> Tasks Handles declarations BEGIN
//=============================================>> Tasks Handles declarations END

uint8_t ds{33};
uint8_t sh_cp{26};
uint8_t st_cp{25};
uint8_t srQty{4};
uint8_t strtngVals [4] {0x00, 0x00,0x00,0x00};
uint8_t* stVlsPtr = strtngVals;

uint8_t mask_1 [] {0b10001000, 0b00100010, 0b00110011, 0b01110111};
uint8_t mask_2 [] {0b01010101, 0b01010101, 0b01010101, 0b01010101};

uint8_t pinUpdtd{0};
uint8_t setVal{};

ShiftRegGPIOXpander srgx(ds, sh_cp, st_cp, srQty, stVlsPtr);

void setup() { 
   Serial.begin(9600);
}

void loop() {

   Serial.println("Example starts");
   Serial.println("--------------");

   Serial.println("digitalWriteSrAllSet() method rises all pins");
   srgx.digitalWriteSrAllSet();
   vTaskDelay(1000);

   Serial.println("digitalWriteSrAllReset() method clears all pins");
   srgx.digitalWriteSrAllReset();
   vTaskDelay(1000);

   Serial.println("digitalWriteSr() method writing HIGH pin by pin");
   setVal = HIGH;
   for(uint8_t pinNum{0}; pinNum <= srgx.getMaxPin(); pinNum++){
      srgx.digitalWriteSr(pinNum, setVal);
      vTaskDelay(200);
   }

   Serial.println("digitalWriteSr() method writing LOW pin by pin");
   setVal = LOW;
   for(uint8_t pinNum{0}; pinNum <= srgx.getMaxPin(); pinNum++){
      srgx.digitalWriteSr(pinNum, setVal);
      vTaskDelay(200);
   }
   vTaskDelay(1000);

   Serial.println("digitalWriteSrToAux() method writing TO THE BUFFER HIGH pin by pin and 'move & flush automatic' every 3rd pin ");
   setVal = HIGH;
   for(uint8_t pinNum{0}; pinNum <= srgx.getMaxPin(); pinNum++){
      if((pinNum+1) % 3 != 0)
         srgx.digitalWriteSrToAux(pinNum, setVal);
      else
         srgx.digitalWriteSr(pinNum, setVal);
      vTaskDelay(200);
   }
   srgx.moveAuxToMain();
   vTaskDelay(1000);

   Serial.println("digitalWriteSrToAux() method writing TO THE BUFFER LOW pin by pin and 'move & flush automatic' every 4th pin");
   setVal = LOW;
   for(uint8_t pinNum{0}; pinNum <= srgx.getMaxPin(); pinNum++){
      if((pinNum+1) % 4 != 0)
         srgx.digitalWriteSrToAux(pinNum, setVal);
      else
         srgx.digitalWriteSr(pinNum, setVal);
      vTaskDelay(200);
   }
   srgx.moveAuxToMain();
   vTaskDelay(1000);

   Serial.println("digitalWriteSrMaskSet() method sets pins using the mask {0b10001000, 0b00100010, 0b00110011, 0b01110111}");
   srgx.digitalWriteSrMaskSet(mask_1);
   vTaskDelay(5000);

   Serial.println("digitalWriteSrMaskReset() method resets pins using the mask {0b01010101, 0b01010101, 0b01010101, 0b01010101}");
   srgx.digitalWriteSrMaskReset(mask_2);
   vTaskDelay(5000);

   Serial.println("digitalWriteSrAllReset() method clears all pins");
   srgx.digitalWriteSrAllReset();
   vTaskDelay(2000);

   Serial.println("stampOverMain() method overwrites the Main Buffer with the value {0b01010101, 0b01010101, 0b01010101, 0b01010101}");
   srgx.stampOverMain(mask_2);
   vTaskDelay(5000);

   Serial.println("digitalWriteSrAllReset() method clears all pins before restarting tests");
   Serial.println("-----------------------------------------------------------------------");
   srgx.digitalWriteSrAllReset();
   vTaskDelay(3000);
}  



//================================================>> General use functions BEGIN
//==================================================>> General use functions END

//=======================================>> User Functions Implementations BEGIN
 /**
  * @brief Error Handling function
  * 
  * Placeholder for a Error Handling function, in case of an error the execution
  * will be trapped in this endless loop
  */
 void Error_Handler(){
   for(;;)
   {    
   }
   
   return;
 }
//=========================================>> User Functions Implementations END