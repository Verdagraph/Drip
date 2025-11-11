#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "stateManager.h"

/** Main task stack size, in words (4 bytes on Esp32c3) */
#define STACK_SIZE 512

/**
 * @brief Runs the finite state machine.
 * 
 * @param pvParameters Allows parameters to be passed from the main function. Currently unused. 
 */
void vMainTask(void *pvParameters) {
    /** Initialize the FSM. */
    StateManager stateManager = StateManager();
    stateManager.initialize();
    
    /** Run the FSM. */
    while (true) {
        stateManager.update();
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