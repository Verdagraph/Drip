#ifndef CONFIG_H
#define CONFIG_H

#define MAX_PRESSURE_CALIBRATION_POINTS 50

typedef enum TankShapes_e {
    TANK_RECTANGLE,
    TANK_CYLINDER
} TankShapes_e;

typedef struct SystemConfig_t {
    uint32_t sleepInterval;
} SystemConfig_t;

typedef struct DispenseConfig_t {
    float dataResolutionLiters;
} DispenseConfig_t;

typedef struct SourceConfig_t {
    float staticFlowRate;
} SourceConfig_t;

typedef struct TankConfig_t {
    TankShapes_e shape;
    float dimension1;
    float dimension2;
    float dimension3;
    uint16_t tank_timeout;
} TankConfig_t;

typedef struct FlowSensorConfig_t {
    float defaultPulsesPerLiter;
    float minFlowRate;
    float calibrationTimeout;
    float calibrateMaxVolume;
} FlowSensorConfig_t;

typedef struct PressureSensorConfig_t {
    float reportMode;
} PressureSensorConfig_t;

typedef struct PressureSensorCalibrationPoint_t {
    uint16_t analogVoltage;
    uint16_t volume;
} PressureSensorCalibrationPoint_t;

typedef struct Config_t {
    SystemConfig_t system;
    DispenseConfig_t dispense;
    SourceConfig_t source;
    TankConfig_t tank;
    FlowSensorConfig_t flowSensor;
    PressureSensorConfig_t pressureSensor;
    PressureSensorCalibrationPoint_t* pressureCalibrationTable;
} Config_t;

#endif