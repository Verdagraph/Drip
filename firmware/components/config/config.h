#ifndef CONFIG_H
#define CONFIG_H

#define MAX_PRESSURE_CALIBRATION_POINTS 50

enum TankShapes_e {
    TANK_RECTANGLE,
    TANK_CYLINDER
};

enum Valves_e {
    SOURCE_DISPENSE,
    TANK_DISPENSE,
    TANK_DRAIN
};

enum Relays_e {
    RELAYS_MIN,
    RELAY1,
    RELAY2,
    RELAY3,
    RELAYS_MAX
};

struct SystemConfig_t {
    uint32_t sleepInterval;
};

struct ValveConfig_t {
    bool sourceDispenseEnabled;
    bool tankDispenseEnabled;
    bool tankDrainEnabled;
    Relays_e sourceDispenseRelay;
    Relays_e tankDispenseRelay;
    Relays_e tankDrainRelay;
    Valves_e preferredDispenseValve;
};

struct DispenseConfig_t {
    float dataResolutionLiters;
};

struct SourceConfig_t {
    float staticFlowRate;
};

struct TankConfig_t {
    TankShapes_e shape;
    float dimension1;
    float dimension2;
    float dimension3;
    uint16_t tank_timeout;
};

struct FlowSensorConfig_t {
    float defaultPulsesPerLiter;
    float minFlowRate;
    float calibrationTimeout;
    float calibrateMaxVolume;
};

struct PressureSensorConfig_t {
    float reportMode;
};

struct PressureSensorCalibrationPoint_t {
    uint16_t analogVoltage;
    uint16_t volume;
};

struct Config_t {
    SystemConfig_t system;
    ValveConfig_t valves;
    DispenseConfig_t dispense;
    SourceConfig_t source;
    TankConfig_t tank;
    FlowSensorConfig_t flowSensor;
    PressureSensorConfig_t pressureSensor;
    PressureSensorCalibrationPoint_t pressureCalibrationTable[MAX_PRESSURE_CALIBRATION_POINTS];
};






/**
 * @file config.h
 * @brief Configuration settings for Wi-Fi provisioning and storage.
 */

/**
 * @defgroup WiFiProvisioning Wi-Fi Provisioning Configuration
 * @brief Configuration parameters for Wi-Fi Access Point (AP) provisioning mode.
 * @{
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
 * @brief Configuration for storing Wi-Fi credentials in NVS.
 * @{
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
 * @brief Configuration for LittleFS filesystem.
 * @{
 */

/** @brief Label for the storage partition defined in partition_custom.csv. */
#define LFS_PARTITION_LABEL "storage"

/** @brief Base path for mounting the LittleFS filesystem. */
#define LFS_BASE_PATH "/littlefs"


#endif
