/**
 * @file sdk_compat.h
 * @brief Precompiled header for standard ESP-IDF and C++ STL includes.
 *
 * This header consolidates all standard includes required for ESP-IDF and C++ STL,
 * ensuring compatibility between C and C++ codebases.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup FreeRTOSIncludes FreeRTOS Core Includes
 * @brief Core FreeRTOS headers for task and event management.
 * 
 */
#include "freertos/FreeRTOS.h"      
#include "freertos/task.h"          
#include "freertos/event_groups.h"  


/**
 * @defgroup ESPIDFCoreIncludes ESP-IDF Core Includes
 * @brief Core ESP-IDF headers for system and storage operations.
 * 
 */
#include "esp_err.h"                
#include "esp_log.h"                
#include "esp_system.h"             
#include "nvs_flash.h"              
#include "esp_mac.h"                


/**
 * @defgroup ESPIDFDrivers ESP-IDF Drivers and Libraries
 * @brief ESP-IDF headers for networking and filesystem drivers.
 * 
 */
#include "esp_wifi.h"               
#include "esp_event.h"              
#include "esp_netif.h"              
#include "esp_http_server.h"        
#include "esp_littlefs.h"           

#ifdef __cplusplus
}
#endif

/**
 * @defgroup CPPIncludes C++ Standard Library Includes
 * @brief Standard C++ libraries for general-purpose programming.
 * 
 */
#include <string>                  
#include <vector>                   
#include <functional>               