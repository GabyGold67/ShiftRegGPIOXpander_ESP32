/**
  ******************************************************************************
  * @file	: ShiftRegGPIOXpander_Example03.cpp
  * @brief  : Code example of the use of the ShiftRegGPIOXpander_ESP32 library
  * 
  * Repository: https://github.com/GabyGold67/ShiftRegGPIOXpander_ESP32
  * Simulation url: 
  * 
  * Framework: Arduino
  * Platform: ESP32
  * 
  * @author	: Gabriel D. Goldman
  * mail <gdgoldman67@hotmail.com>
  * Github <https://github.com/GabyGold67>
  *
  * @date First release: 24/05/2025 
  *       Last update:   24/05/2025 22:00 GMT+0200 DST
  ******************************************************************************
  * @warning **Use of this library is under your own responsibility**
  * 
  * @warning The use of this library falls in the category described by The Alan 
  * Parsons Project (c) 1980 "Games People play" disclaimer:  
  * Games people play, you take it or you leave it  
  * Things that they say aren't alright  
  * If I promised you the moon and the stars, would you believe it?  
  * 
  * Released into the public domain in accordance with "GPL-3.0-or-later" license terms.
  ******************************************************************************
*/
#include <Arduino.h>
#include <ShiftRegGPIOXpander_ESP32.h>

//==============================================>> General use definitions BEGIN
#define LoopDlyTtlTm 1500 // Time between task unblocking, time taken from the start of the task execution to the next execution 
#define MainCtrlTskPrrtyLvl 5 // Task priority level

static BaseType_t xReturned; /*!<Static variable to keep returning result value from Tasks and Timers executions*/
static BaseType_t errorFlag {pdFALSE};

BaseType_t ssdExecTskCore = xPortGetCoreID();
BaseType_t ssdExecTskPrrtyCnfg = MainCtrlTskPrrtyLvl;
//================================================>> General use definitions END
 
//======================================>> General use function prototypes BEGIN
void Error_Handler();
//========================================>> General use function prototypes END
 
//====================================>> Task Callback function prototypes BEGIN
void mainCtrlTsk(void *pvParameters);
//======================================>> Task Callback function prototypes END
 
//===========================================>> Tasks Handles declarations BEGIN
TaskHandle_t mainCtrlTskHndl {NULL};
//=============================================>> Tasks Handles declarations END

void setup() { 

   // Create the Main control task for setup and execution of the main code
    xReturned = xTaskCreatePinnedToCore(
       mainCtrlTsk,  // Callback function/task to be called
       "MainControlTask",  // Name of the task
       2048,   // Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
       NULL,  // Pointer to the parameters for the function to work with
       ssdExecTskPrrtyCnfg, // Priority level given to the task
       &mainCtrlTskHndl, // Task handle
       ssdExecTskCore // Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
    );
    if(xReturned != pdPASS)
       Error_Handler();
}

void loop() {
   vTaskDelete(NULL); // Delete this task -the ESP-Arduino LoopTask()- and remove it from the execution list
}  

 //===============================>> User Tasks Implementations BEGIN
 void mainCtrlTsk(void *pvParameters){
   delay(10);  //FTPO Part of the WOKWI simulator additions, for simulation startup needs

   uint8_t ds{33};
   uint8_t sh_cp{26};
   uint8_t st_cp{25};
   uint8_t srQty{1};
   
   uint8_t strtngVals [1] {0x00};
   uint8_t* stVlsPtr = strtngVals;

   Serial.begin(9600);  

   ShiftRegGPIOXpander srgx(ds, sh_cp, st_cp, srQty);
   srgx.begin(stVlsPtr);

   uint8_t mask{0b00001111};
   uint8_t* maskPtr = &mask;

   for(;;){
      Serial.println("Flip bits one by one using the digitalToggleSr() method");
      for(int pinUpdtd{0}; pinUpdtd < 16; pinUpdtd++){
         srgx.digitalToggleSr(pinUpdtd % 8);
         vTaskDelay(1000);
      }      
      vTaskDelay(2000);

      Serial.println("Flip the rightmost 4 bits using the digitalToggleSrMask() method with mask 0b00001111");
      srgx.digitalToggleSrMask(maskPtr);
      vTaskDelay(4000);

      Serial.println("Flip bits using the digitalToggleSrMask() method with mask 0b01010101");
      mask = 0b01010101;
      srgx.digitalToggleSrMask(maskPtr);
      vTaskDelay(4000);

      Serial.println("Flip the rightmost 4 bits using the digitalToggleSrMask() method with mask 0b00001111");
      mask = 0b00001111;
      srgx.digitalToggleSrMask(maskPtr);
      vTaskDelay(4000);

      Serial.println("Flip all the bits using the digitalToggleSrAll() method");
      srgx.digitalToggleSrAll();
      vTaskDelay(4000);

      Serial.println("Set all the bits to 0 using the digitalWriteSrAllReset() method");
      srgx.digitalWriteSrAllReset();
      vTaskDelay(1000);

      Serial.println("Copy the Main Buffer to the Auxiliary Buffer with all it's bits reset using the copyMainToAux() method");
      srgx.copyMainToAux();
      Serial.println("Flip the two rightmost and the two leftmost bits using the digitalToggleSrToAux() method");
      srgx.digitalToggleSrToAux(0);
      srgx.digitalToggleSrToAux(1);
      srgx.digitalToggleSrToAux(6);
      srgx.digitalToggleSrToAux(7);
      Serial.println("Move the Auxiliary Buffer to the Main Buffer and flush it using the moveAuxToMain() method");
      srgx.moveAuxToMain(true);
      vTaskDelay(4000);

      Serial.println("Clear al bits of the Shift Register GPIO Expander using the digitalWriteSrAllReset() method");
      srgx.digitalWriteSrAllReset();
      vTaskDelay(2000);

   }
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