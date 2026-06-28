#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mainStateController.h"

/** Main task stack size, in words (4 bytes on Esp32c3) */
constexpr size_t STACK_SIZE = 1024U;

/**
 * @brief Runs the finite state machine.
 * 
 * @param pvParameters Allows parameters to be passed from the main function. Currently unused. 
 */
void vMainTask(void *pvParameters) {
    DataContainer dataContainer = DataContainer();
    ConfigManager configManager = ConfigManager(dataContainer);
    MqttManager mqttManager(dataContainer);
    WifiManager wifiManager(dataContainer);
    ValveManager valveManager(dataContainer);
    FlowSensorManager flowSensorManager;

    /** Initialize the FSM. */
    MainStateController mainStateController = MainStateController(
        dataContainer,
        configManager,
        wifiManager,
        mqttManager,
        valveManager,
        flowSensorManager
    );
    
    /** Run the FSM. */
    while (true) {
        printf("Hello from FSM Task! Free Heap: %" PRIu32 "\n", esp_get_free_heap_size());
        mainStateController.update();
        ESP_LOGI("Updating", "Updating");
        vTaskDelay(1U);
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
    ESP_LOGI("Updating", "Updating");
    ESP_LOGI("Updating", "Updating");
    ESP_LOGI("Updating", "Updating");
    ESP_LOGI("Updating", "Updating");

    ESP_LOGI("Updating", "Updating");

    printf("Hello from FSM Task! Free Heap: %" PRIu32 "\n", esp_get_free_heap_size());
    printf("Hello from FSM Task! Free Heap: %" PRIu32 "\n", esp_get_free_heap_size());
    printf("Hello from FSM Task! Free Heap: %" PRIu32 "\n", esp_get_free_heap_size());
    printf("Hello from FSM Task! Free Heap: %" PRIu32 "\n", esp_get_free_heap_size());
    printf("Hello from FSM Task! Free Heap: %" PRIu32 "\n", esp_get_free_heap_size());
    printf("Hello from FSM Task! Free Heap: %" PRIu32 "\n", esp_get_free_heap_size());

    /** Run task through FreeRTOS. */
    xReturned = xTaskCreate(vMainTask, 
        "FSM", 
        STACK_SIZE, 
        (void*) nullptr, 
        tskIDLE_PRIORITY, 
        &xHandle
    );

    return;
}