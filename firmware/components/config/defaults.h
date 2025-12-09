/**
 * @file defaults.h
 * @brief Default configuration values.
 */

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include "config.h"

constexpr DripConfig_t getDefaultConfig() {
    return {
        .system = {
            .sleepIntervalMs = 1000U,
            .processSliceUploadIntervalMs = 100U,
        },
        .valves = {
            .sourceDispenseRelay = DripRelays_e::Null,
            .tankDispenseRelay = DripRelays_e::Null,
            .tankDrainRelay = DripRelays_e::Null,
            .preferredDispenseValve = DripValves_e::SourceDispense,
            .maxDispenseTargetSeconds = 12U * 60U * 60U,
            .maxDispenseTargetLiters = 1000U,
            .defaultDispenseTimeoutMin = 120U 
        },
        .source = {
            .enabled = true,
            .staticFlowRateLpm = 0.0f
        },
        .tank = {
            .enabled = false,
            .shape = TANK_CYLINDER,
            .dimension1 = 0.0f,
            .dimension2 = 0.0f,
            .dimension3 = 0.0f,
            .volume = 0.0f,
            .tankTimeoutMs = 5000U
        },
        .flowSensor = {
            .enabled = false,
            .defaultPulsesPerLiter = 0U,
            .calibratedPulsesPerLiter = 0U,
            .minFlowRateLps = 0.0f,
            .calibrationTimeoutMs = 120000U,
            .calibrateMaxVolume = 6.0f
        },
        .pressureSensor = {
            .enabled = false,
            .broadcastEnabled = false,
            .broadcastIntervalMs = 30000,
        }
    };
};

#endif
