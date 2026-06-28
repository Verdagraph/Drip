#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mainStateController.h"

/** Main task stack size, in words (4 bytes on Esp32c3) */
constexpr size_t STACK_SIZE = 1024U;

/** ETL configuration. */
#define ETL_NO_STL

/**
 * @brief Runs the connection manager.
 * 
 * @param pvParameters Allows parameters to be passed from the main function. Currently unused. 
 */
void vConnectionTask(void *pvParameters) {

    /** Run the FSM. */
    while (true) {
        mainStateController.update();
        vTaskDelay(1U);
    }

    /** Should not reach here. */
    vTaskDelete( NULL );

/**
 * @brief Entrypoint. 
 */
extern "C" void app_main(void) {
    TaskHandle_t connectionTaskHandle = NULL;

    /** Define all inter-task constructs. */
    DataContainer dataContainer = new DataContainer();

    /** Run task through FreeRTOS. */
    xTaskCreate(vConnectionTask, 
        "FSM", 
        STACK_SIZE, 
        (void*) nullptr, 
        tskIDLE_PRIORITY, 
        &connectionTaskHandle
    );

    return;
}