/**
 * @file consts.h
 * @brief Compile-time configuraion.
 */

#ifndef CONSTS_H
#define CONSTS_H

#define MAX_PRESSURE_CALIBRATION_POINTS 50

/**
 * @defgroup WiFiProvisioning Wi-Fi Provisioning Configuration
 */

/** @brief SSID for the provisioning Access Point. */
#define PROV_AP_SSID "ESP32-Provisioning"

/** @brief Password for the provisioning Access Point (minimum 8 characters). */
#define PROV_AP_PASS "password123"

/** @brief Maximum number of clients that can connect to the Access Point. */
#define PROV_AP_MAX_CONN 1

/** @} */

/**
 * @defgroup NVSConfig Non-Volatile Storage (NVS) Configuration
 *
 */

/** @brief Namespace used for NVS storage. */
#define NVS_NAMESPACE "storage"

/** @brief Key for storing Wi-Fi SSID in NVS. */
#define NVS_KEY_WIFI_SSID "wifi_ssid"

/** @brief Key for storing Wi-Fi password in NVS. */
#define NVS_KEY_WIFI_PASS "wifi_pass"

/** @} */

/**
 * @defgroup LittleFSConfig LittleFS Configuration
 */

/** @brief Label for the storage partition defined in partition_custom.csv. */
#define LFS_PARTITION_LABEL "storage"

/** @brief Base path for mounting the LittleFS filesystem. */
#define LFS_BASE_PATH "/littlefs"


#endif
