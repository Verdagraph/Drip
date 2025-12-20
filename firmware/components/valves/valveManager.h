#ifndef VALVE_MANAGER_H
#define VALVE_MANAGER_H

#include <etl/const_map.h>

#include "stateController.h"
#include "message.h"
#include "config.h"
#include "configManager.h"
#include "gpioDriver.h"
#include "dataContainer.h"

#include "valveData.h"

#define DRIP_DRAIN_PROCESS_MAX_TIMEOUT_MIN 120

struct DripValveManagerFsmEventTimers_t {
    uint32_t dispenseBeganTicks;
    uint32_t dispenseSourceBeganTicks;
    uint32_t dispenseTankBeganTicks;
    uint32_t drainBeganTicks;
    uint32_t lastFlowSensorAboveMinimumFlowTicks;
};

/**
 * @brief Handles the dispensation and draining process.
 */
class ValveManager : public StateController<DripValveManagerStateId_e, DripValveManagerEventId_e, DripRxMessage<DripValveManagerEventId_e>, ValveManager> {
public:
    /**
     * @brief Constructor.
     */
    ValveManager(DataContainer &dataContainer);

    /**
     * @brief Begin the ValveManager.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t initialize();

    /**
     * @brief True if the valves are all currently closed.
     */
    bool isIdle() const;

    /**
     * @brief True if the valves are currently dispensing.
     */
    bool isDispensing() const;

    /**
     * @brief True if the tank drain valve is open.
     */
    bool isDraining() const;

    /**
     * @defgroup StateHandler State handlers.
     */

    /**
     * @brief Handlers for state idle.
     */
    void idleEntry();
    void idleUpdate();
    void idleExit();

    /**
     * @brief Handlers for state dispense start.
     */
    void dispenseStartEntry();
    void dispenseStartUpdate();
    void dispenseStartExit();

    /**
     * @brief Handlers for state dispense source.
     */
    void dispenseSourceEntry();
    void dispenseSourceUpdate();
    void dispenseSourceExit();

    /**
     * @brief Handlers for state dispense tank.
     */
    void dispenseTankEntry();
    void dispenseTankUpdate();
    void dispenseTankExit();

    /**
     * @brief Handlers for state dispense exit.
     */
    void dispenseExitEntry();
    void dispenseExitUpdate();
    void dispenseExitExit();

    /**
     * @brief Handlers for state drain tank.
     */
    void drainTankEntry();
    void drainTankUpdate();
    void drainTankExit();

    /**
     * @brief Handlers for state deactivate.
     */
    void deactivateEntry();
    void deactivateUpdate();
    void deactivateExit();

    /** @} */

    /**
     * @defgroup EventHandlers Event handlers.
     */

    /**
     * @brief Handles state change for a dispense request in the idle state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type DispenseActivateRxMessage.
     */
    void handleDispenseRequestStateIdle(DripValveManagerEventId_e id, const DripRxMessage<DripValveManagerEventId_e> *message);

    /**
     * @brief Handles state change for a drain request when in the idle state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type DrainActivateRxMessage.
     */
    void handleDrainRequestStateIdle(DripValveManagerEventId_e id, const DripRxMessage<DripValveManagerEventId_e> *message);

    /**
     * @brief Handles state change for a deactivation request in the dispense or drain state.
     *
     * @param[in] id Message ID.
     * @param[in] message MQTT received message of type DeactivateRxMessage.
     */
    void handleDeactivateRequestStateDispenseOrDrain(DripValveManagerEventId_e id, const DripRxMessage<DripValveManagerEventId_e> *message);

    /** @} */


private:
    DripValveManagerFsmEventTimers_t eventTimers_;

    DataContainer dataContainer_;

    DripValves_e openValve_;

    
    /**
     * @brief Closes the currently open valve.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t closeValves();
    
    /**
     * @brief Open the tank dispense valve.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t openTankDispenseValve();
    
    /**
     * @brief Open the tank drain valve.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t openTankDrainValve();
    
    /**
     * @brief Open the source dispense valve.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t openSourceDispenseValve();

    /**
     * @brief Sets the open/closed state of valve.
     * 
     * @param valve The valve to set.
     * @param state The state to set on the valve output.
     * 
     * @retval ESP_OK If the valve state was set successfully.
     */
    esp_err_t setValveState(DripValves_e valve, DripGpioToggleState_e state);

    esp_err_t updateDispenseProcessSlice();
};

#endif