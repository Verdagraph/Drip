/**
 * @file enums.h
 * @brief Core enumerated values.
 */

#ifndef ENUMS_H
#define ENUMS_H

/**
 * @brief Describes the two types of valves controlled by this application.
 */
enum class DripValveType_e {
    /** @brief Used to output a target water time or duration. */
    Dispense,
    /** @brief Used to empty rain barrels or other variable water supplies. */
    Drain
};

/**
 * @brief Describes the supported shapes for a tank,
 * for the purposes of volume calculation.
 */
enum TankShapes_e {
    TANK_RECTANGLE,
    TANK_CYLINDER
};

/**
 * @brief The three supported valves.
 */
enum class DripValves_e {
    Null,
    /** @brief Dispense valve of a constant pressure source (eg. home water supply).  */
    SourceDispense,
    /** @brief Dispense valve of a variable pressure tank (eg. rain barrel). */
    TankDispense,
    /** @brief Drain valve of a variable pressure tank (eg. rain barrel). */
    TankDrain
};

/**
 * @brief The three supported relays.
 * These correspond to the silkscreen on the PCB.
 */
enum DripRelays_e {
    Null,
    Relay1,
    Relay2,
    Relay3,
};


#endif
