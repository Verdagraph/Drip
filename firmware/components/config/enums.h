/**
 * @file symbols.h
 * @brief Core enumerated values.
 */

#ifndef SYMBOLS_H
#define SYMBOLS_H

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
enum Valves_e {
    /** @brief Dispense valve of a constant pressure source (eg. home water supply).  */
    SOURCE_DISPENSE,
    /** @brief Dispense valve of a variable pressure tank (eg. rain barrel). */
    TANK_DISPENSE,
    /** @brief Drain valve of a variable pressure tank (eg. rain barrel). */
    TANK_DRAIN
};

/**
 * @brief The three supported relays.
 * These correspond to the silkscreen on the PCB.
 */
enum Relays_e {
    RELAY_NONE,
    RELAYS_MIN,
    RELAY1,
    RELAY2,
    RELAY3,
    RELAYS_MAX
};


#endif
