/**
  ******************************************************************************
  * @file	: ShiftRegGPIOXpander_Example01.cpp
  * @brief  : 
  * 
  * Framework: Arduino
  * Platform: ESP32
  * 
  * @author	: Gabriel D. Goldman
  *
  * @date First release: 16/02/2025 
  *       Last update:   24/02/2025 17:20 GMT+0200
  ******************************************************************************
  * @attention	
  * 
  * Released into the public domain in accordance with "GPL-3.0-or-later" license terms.
  ******************************************************************************
*/
#include <Arduino.h>
#include <ShiftRegGPIOXpander.h>

//==============================================>> General use definitions BEGIN
#define LoopDlyTtlTm 1500 // Time between task unblocking, time taken from the start of the task execution to the next execution 
#define MainCtrlTskPrrtyLvl 5 // Task priority level

static BaseType_t xReturned; /*!<Static variable to keep returning result value from Tasks and Timers executions*/
static BaseType_t errorFlag {pdFALSE};

BaseType_t srgxExecTskCore = xPortGetCoreID();
BaseType_t srgxExecTskPrrtyCnfg = MainCtrlTskPrrtyLvl;
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
       srgxExecTskPrrtyCnfg, // Priority level given to the task
       &mainCtrlTskHndl, // Task handle
       srgxExecTskCore // Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
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

   TickType_t loopTmrStrtTm{0};
   TickType_t* loopTmrStrtTmPtr{&loopTmrStrtTm};
   TickType_t totalDelay {LoopDlyTtlTm};

   uint8_t ds{33};
   uint8_t sh_cp{26};
   uint8_t st_cp{25};
   uint8_t srQty{1};
   
   uint8_t strtngVals [1] {0xFF};
   uint8_t* stVlsPtr = strtngVals;

   ShiftRegGPIOXpander srgx(ds, sh_cp, st_cp, srQty, stVlsPtr);

   uint8_t pinUpdtd{0};
   uint8_t setVal{HIGH};

   for(;;){
      srgx.digitalWriteSr(pinUpdtd, setVal);
      vTaskDelay(1000);
      pinUpdtd++;
      if(pinUpdtd > srgx.getMaxPin()){
         pinUpdtd = 0;
         if(setVal == HIGH)
            setVal = LOW;
         else
            setVal = HIGH;
      }
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