/**
  ******************************************************************************
  * @file	: ShiftRegGPIOXpander_Example04.ino
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
  * @date First release: 01/06/2025 
  *       Last update:   02/06/2025 14:00 GMT+0200 DST
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
void Error_Handler(int8_t errorCode); /*!<Error Handler function prototype, to be implemented by the user*/
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
       Error_Handler(0x01);
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

   /*Constants for pin based methods*/
   const uint16_t redPin{0x00}; // Red light pin
   const uint16_t yellowPin{0x01}; // Yellow light pin
   const uint16_t greenPin{0x02}; // Green light pin

   /*Constants for mask based methods*/
   const uint16_t redMsk{0x01 << redPin}; // Red light
   const uint16_t yellowMsk{0x01 << yellowPin}; // Yellow light
   const uint16_t greenMsk{0x01 << greenPin}; // Green light
   
   uint8_t SRGXstrtngVal [1] {0x00};
   uint16_t portStrtngVal{0x0000}; 

   Serial.begin(9600);  //FTPO

   ShiftRegGPIOXpander srgx(ds, sh_cp, st_cp, srQty);
   srgx.begin(SRGXstrtngVal); 

   Serial.println("Instantiating myVPortNS while _maxSRGXPin value: " + String(srgx.getMaxSRGXPin()));
   SRGXVPort myVPortNS = srgx.createSRGXVPort(0, 3);
   if(!(srgx.isValid(myVPortNS)))
      Error_Handler(0x02); // Error creating the virtual port, exit the task
   myVPortNS.begin(0x0005);

   SRGXVPort myVPortEW = srgx.createSRGXVPort(4, 3);
   if(!(srgx.isValid(myVPortEW)))
      Error_Handler(0x03); // Error creating the virtual port, exit the task
   myVPortEW.begin(0x0002);

   vTaskDelay(2000);

   for(;;){
      Serial.println("Set the port pins one by one using the digitalWriteSr() method on both virtual ports");
      for(int pinUpdtd{0}; pinUpdtd < 3; pinUpdtd++){
         Serial.println("Setting pins " + String(pinUpdtd) + " to HIGH");
         myVPortNS.digitalWriteSr(pinUpdtd, HIGH);
         vTaskDelay(1000);
         myVPortEW.digitalWriteSr(pinUpdtd, HIGH);
         vTaskDelay(2000);
      }      
      vTaskDelay(2000);

      Serial.println("Reset the port pins one by one using the digitalWriteSr() method on both virtual ports");
      for(int pinUpdtd{2}; pinUpdtd >= 0; pinUpdtd--){
         Serial.println("Setting pin " + String(pinUpdtd) + " to LOW");
         myVPortNS.digitalWriteSr(pinUpdtd, LOW);
         vTaskDelay(1000);
         myVPortEW.digitalWriteSr(pinUpdtd, LOW);
         vTaskDelay(2000);
      }      
      vTaskDelay(2000);

      Serial.println("Set the port pins one by one using the setBit() method on both virtual ports");
      for(int pinUpdtd{0}; pinUpdtd < 3; pinUpdtd++){
         Serial.println("Setting pins " + String(pinUpdtd) + " to HIGH");
         myVPortNS.setBit(pinUpdtd);
         vTaskDelay(1000);
         myVPortEW.setBit(pinUpdtd);
         vTaskDelay(2000);
      }      
      vTaskDelay(2000);

      Serial.println("Reset the port pins one by one using the resetBit() method on both virtual ports");
      for(int pinUpdtd{2}; pinUpdtd >= 0; pinUpdtd--){
         Serial.println("Setting pin " + String(pinUpdtd) + " to LOW");
         myVPortNS.resetBit(pinUpdtd);
         vTaskDelay(1000);
         myVPortEW.resetBit(pinUpdtd);
         vTaskDelay(2000);
      }      
      vTaskDelay(2000);

      Serial.println("Set several port pins at once by using the writePort() method on both virtual ports");
      Serial.println("Setting value 2 in the myVPortNS virtual port");
      myVPortNS.writePort(0x02);
      vTaskDelay(1000);
      Serial.println("Setting value 5 in the myVPortEW virtual port");
      myVPortEW.writePort(0x05);
      vTaskDelay(2000);
      
      Serial.println("Toggle the port pins one by one using the flipBit() method on both virtual ports");
      for(int pinUpdtd{0}; pinUpdtd < 3; pinUpdtd++){
         Serial.println("Flipping pins " + String(pinUpdtd));
         myVPortNS.flipBit(pinUpdtd);
         vTaskDelay(1000);
         myVPortEW.flipBit(pinUpdtd);
         vTaskDelay(2000);
      }      
      vTaskDelay(2000);

      
      Serial.println("\nMaximum value holdable by the vPorts: " + String(myVPortNS.getVPortMaxVal(), HEX));
            
      Serial.println("\nRead the port pins state as a single value using the readPort() method on both virtual ports");
      Serial.println("Value in the myVPortNS pins: " + String(myVPortNS.readPort(), HEX));
      vTaskDelay(1000);
      Serial.println("Value in the myVPortEW pins: " + String(myVPortEW.readPort(), HEX));
      vTaskDelay(2000);
      vTaskDelay(2000);


      // Serial.println("Flip all the bits using the digitalToggleSrAll() method");
      // srgx.digitalToggleSrAll();
      // vTaskDelay(4000);

      // Serial.println("Set all the bits to 0 using the digitalWriteSrAllReset() method");
      // srgx.digitalWriteSrAllReset();
      // vTaskDelay(1000);

      // Serial.println("Copy the Main Buffer to the Auxiliary Buffer with all it's bits reset using the copyMainToAux() method");
      // srgx.copyMainToAux();
      // Serial.println("Flip the two rightmost and the two leftmost bits using the digitalToggleSrToAux() method");
      // srgx.digitalToggleSrToAux(0);
      // srgx.digitalToggleSrToAux(1);
      // srgx.digitalToggleSrToAux(6);
      // srgx.digitalToggleSrToAux(7);
      // Serial.println("Move the Auxiliary Buffer to the Main Buffer and flush it using the moveAuxToMain() method");
      // srgx.moveAuxToMain();
      // vTaskDelay(4000);

      // Serial.println("Clear al bits of the Shift Register GPIO Expander using the digitalWriteSrAllReset() method");
      // srgx.digitalWriteSrAllReset();
      // vTaskDelay(2000);
   
      // Serial.println("Stamps both ports with the 'Second traffic lights' state");
      // // myVPortNS.writePort(red); 
      // myVPortEW.writePort(yellow); 
      // vTaskDelay(3000);

      // Serial.println("Stamps both ports with the 'Third traffic lights' state");
      // myVPortNS.writePort(green); 
      // myVPortEW.writePort(red);
      // vTaskDelay(3000);

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
 void Error_Handler(int8_t errorCode){
   Serial.println("Error Handler called with error code: " + String(errorCode));
   for(;;)
   {    
   }
   
   return;
 }
//=========================================>> User Functions Implementations END