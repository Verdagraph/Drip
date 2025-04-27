#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "configManager.h"
#include "mqttManager.h"
#include "connectionManager.h"
#include "valveManager.h"
#include "stateManager.h"

/** Main task stack size, in works (4 bytes on Esp32c3) */
#define STACK_SIZE 512

/**
 * @brief Runs the finite state machine.
 * 
 * @param pvParameters Allows parameters to be passed from the main function. Currently unused. 
 */
void vMainTask(void *pvParameters) {
    /** Initialize managers. */
    ConfigManager configManager = ConfigManager();
    MqttManager mqttManager = MqttManager();
    ConnectionManager connectionManager = ConnectionManager();
    ValveManager valveManager = ValveManager();
    StateManager stateManager = StateManager(&configManager, &mqttManager, &connectionManager, &valveManager);

    /** Initialize the FSM. */
    stateManager.initialize();
    
    /** Run the FSM. */
    while (true) {
        stateManager.handle_current_state();
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