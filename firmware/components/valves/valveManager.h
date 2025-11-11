#ifndef VALVE_MANAGER_H
#define VALVE_MANAGER_H

#include "config.h"
#include "configManager.h"
#include "gpioDriver.h"
#include "dataContainer.h"

#include "valveData.h"

#define VDG_DRAIN_PROCESS_MAX_TIMEOUT_MIN 120

/**
 * @brief Handles the dispensation and draining process.
 */
class ValveManager {
public:
    /**
     * @brief Constructor.
     */
    ValveManager(ConfigManager &configManager, DataContainer &dataContainer);

    /**
     * @brief Begin the ValveManager.
     * 
     * @return esp_err_t Return code. 
     */
    esp_err_t initialize();

    /**
     * @brief Begins a dispensation process.
     * 
     * @param[in] target Target for the process.
     * @param[out] process Output parameter for the current process after this function.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t beginDispenseProcess(VdgDispenseProcessTarget_t &target, VdgValveProcess_e &process);
    
    /**
     * @brief Begins a drain process.
     * 
     * @param[in] target Target for the process.
     * @param[out] process Output parameter for the current process after this function.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t beginDrainProcess(VdgDrainProcessTarget_t &target, VdgValveProcess_e &process);
    
    /**
     * @brief Updates the current process.
     * 
     * @param[out] process Output parameter for the current process after this function.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t updateProcess(VdgValveProcess_e &process);
    
    /**
     * @brief Ends the current process.
     * 
     * @param[out] process Output parameter for the current process after this function.
     * 
     * @return esp_err_t Return code.
     */
    esp_err_t endProcess(VdgValveProcess_e &process);

    esp_err_t getCurrentProcess(VdgValveProcess_e &process);

private:
    ConfigManager configManager_;
    DataContainer dataContainer_;

    VdgOpenValveState_e openValve_;
    VdgValveProcess_e currentProcess_;

    esp_err_t setValveState(Valves_e valve, VdgGpioToggleState_e state);

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
};

#endif