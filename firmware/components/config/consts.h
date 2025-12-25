/**
 * @file consts.h
 * @brief Compile-time configuraion.
 */

#ifndef CONSTS_H
#define CONSTS_H

#include <cstdint>

constexpr uint32_t MAX_PRESSURE_CALIBRATION_POINTS = 512;

/**
 * @defgroup WiFiProvisioning Wi-Fi Provisioning Configuration
 */

/** @brief SSID for the provisioning Access Point. */
constexpr char PROV_AP_SSID[] = "ESP32-Provisioning";

/** @brief Password for the provisioning Access Point (minimum 8 characters). */
constexpr char PROV_AP_PASS[] = "password123";

/** @brief Maximum number of clients that can connect to the Access Point. */
constexpr uint8_t PROV_AP_MAX_CONN = 1;

/** @} */

/**
 * @defgroup NVSConfig Non-Volatile Storage (NVS) Configuration
 *
 */

/** @brief Namespace used for NVS storage. */
constexpr char NVS_NAMESPACE[] = "storage";

/** @brief Key for storing Wi-Fi SSID in NVS. */
constexpr char NVS_KEY_WIFI_SSID[] = "wifi_ssid";

/** @brief Key for storing Wi-Fi password in NVS. */
constexpr char NVS_KEY_WIFI_PASS[] = "wifi_pass";

/** @} */

/**
 * @defgroup LittleFSConfig LittleFS Configuration
 */

/** @brief Label for the storage partition defined in partition_custom.csv. */
constexpr char LFS_PARTITION_LABEL[] = "storage";

/** @brief Base path for mounting the LittleFS filesystem. */
constexpr char LFS_BASE_PATH[] = "/littlefs";


#endif
