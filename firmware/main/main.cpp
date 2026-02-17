#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mainStateController.h"

/** Main task stack size, in words (4 bytes on Esp32c3) */
constexpr size_t STACK_SIZE = 512U;

/**
 * @brief Runs the finite state machine.
 * 
 * @param pvParameters Allows parameters to be passed from the main function. Currently unused. 
 */
void vMainTask(void *pvParameters) {
    DataContainer dataContainer = DataContainer();
    ConfigManager configManager = ConfigManager(dataContainer);
    //MqttManager mqttManager(dataContainer);
    //WifiManager wifiManager(dataContainer);
    //ValveManager valveManager(dataContainer);
    //FlowSensorManager flowSensorManager;

    /** Initialize the FSM. */
    MainStateController mainStateController = MainStateController(
        dataContainer,
        configManager
        //wifiManager,
        //mqttManager,
        //valveManager,
        //flowSensorManager
    );
    
    /** Run the FSM. */
    while (true) {
        mainStateController.update();
    }

    /** Should not reach here. */
    vTaskDelete( NULL );
}

/**
 * @brief Entrypoint. 
 */
extern "C" void app_main(void) {
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;

    /** Run task through FreeRTOS. */
    xReturned = xTaskCreate(vMainTask, 
        "FSM", 
        STACK_SIZE, 
        (void*) nullptr, 
        tskIDLE_PRIORITY, 
        &xHandle
    );

    /** Should not reach here. */
    if (xReturned == pdPASS) {
        vTaskDelete(xHandle);
    }
}