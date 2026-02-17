/**
 * @file stateMachine.h
 * @author Nathaniel King
 * @brief Defines a basic state machine. 
 * @date 2025-11-29
 * 
 * @details The StateMachine class handles the transition between
 * states, and is expected to be composed within a StateController
 * class which defines the state handler functions.
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <etl/const_map.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#include "dataContainer.h"


/**
 * @defgroup Constants Constant definitions.
 */

/** @brief The maximum supported number of entries in the state and event maps. */
constexpr size_t DRIP_STATE_MAP_MAX_SIZE = 64U;
constexpr size_t DRIP_EVENT_MAP_MAX_SIZE = 64U;

/** @brief The maximum length of the FSM name. */
constexpr size_t DRIP_FSM_NAME_MAX_LEN = 64U; 

/** @} */

/**
 * @defgroup HandlerFunctionTypes Handler function type definitions.
 */

/**
 * @brief Describes a state entry function defined on the StateController.
 * This function is called once per state transition.
 * 
 * @tparam TStateController Parent state controller class.
 */
template <typename TStateController>
using StateHandlerEnterPtr = void (TStateController::*)();

/**
 * @brief Describes a state update function defined on the StateController.
 * This function is called on an interval defined in the state handler map.
 * 
 * @tparam TStateController Parent state controller class.
 */
template <typename TStateController>
using StateHandlerUpdatePtr = void (TStateController::*)();

/**
 * @brief Describes a state exit function defined on the StateController.
 * This function is called once per state transition.
 * 
 * @tparam TStateController Parent state controller class.
 */
template <typename TStateController>
using StateHandlerExitFuncPtr = void (TStateController::*)();

/**
 * @brief Describes an event handler function defined on the StateController.
 * This function is called once per event.
 * 
 * @tparam TEventId_e The enumerated ID of the event.
 * @tparam TEvent The event object.
 * @tparam TStateController Parent state controller class.
 */
template <typename TEventId_e, typename TEvent, typename TStateController>
using StateHandlerEventFuncPtr = void (TStateController::*)(TEventId_e eventId, const TEvent *event);

/** @} */

/**
 * @defgroup StateHandlerMaps State handling mapping definitions.
 */

/**
 * @brief Describes the handling of a state, 
 * contained within the state handler map.
 * 
 * @tparam TStateController Parent state controller class.
 */
template <typename TStateController>
struct StateHandlerMapEntry_t {
    /** @brief The state entry function. */
    const StateHandlerEnterPtr<TStateController> entryFunc;
    /** @brief The state update function. */
    const StateHandlerUpdatePtr<TStateController> updateFunc;
    /** @brief The state exit function. */
    const StateHandlerExitFuncPtr<TStateController> exitFunc;
    /** @brief An interval, in miliseconds, on which to call the update function while in this state. */
    uint32_t handlingIntervalMs;

    constexpr StateHandlerMapEntry_t()
            : entryFunc(nullptr), 
            updateFunc(nullptr), 
            exitFunc(nullptr), 
            handlingIntervalMs(0) 
            {}

    constexpr StateHandlerMapEntry_t(
            StateHandlerEnterPtr<TStateController> entry,
            StateHandlerUpdatePtr<TStateController> update,
            StateHandlerExitFuncPtr<TStateController> exit,
            uint32_t interval = 1U)
        : 
            entryFunc(entry), 
            updateFunc(update), 
            exitFunc(exit), 
            handlingIntervalMs(interval) 
        {}
};

/**
 * @brief Pair used to construct the state handler map.
 * 
 * @tparam TStateId_e The enumerated ID of the state.
 * @tparam TStateController Parent state controller class.
 */
template <typename TStateId_e, typename TStateController>
using StateHandlerMapPair = etl::pair<TStateId_e, StateHandlerMapEntry_t<TStateController>>;

/**
 * @brief Maps all values of the state ID enum to a state handler function.
 * 
 * @tparam TStateId_e The enumerated ID of the state.
 * @tparam TStateController Parent state controller class.
 */
template <typename TStateId_e, typename TStateController>
using StateHandlerMap = etl::const_map<TStateId_e, StateHandlerMapEntry_t<TStateController>, DRIP_STATE_MAP_MAX_SIZE>;

/** @} */

/**
 * @defgroup EventHandlerMaps Event handling mapping definitions.
 */

/**
 * @brief Describes the handling of an event, 
 * contained within the event handler map.
 * 
 * @tparam TEventId_e The enumerated ID of the event.
 * @tparam TEvent The event object.
 * @tparam TStateController Parent state controller class.
 */
template <typename TEventId_e, typename TEvent, typename TStateController>
struct EventHandlerMapEntry_t {
    /** @brief The event handler function. */
    const StateHandlerEventFuncPtr<TEventId_e, TEvent, TStateController> handleFunc;

    constexpr EventHandlerMapEntry_t()
            : handleFunc(nullptr) {}

    constexpr EventHandlerMapEntry_t(StateHandlerEventFuncPtr<TEventId_e, TEvent, TStateController> func) : 
            handleFunc(func) {}
};

/**
 * @brief Describes a key of the event handler map,
 * which maps an event to be handled and the state
 * in which it is handled to a specific handler function
 * 
 * @tparam TStateId_e The enumerated ID of the state. 0 must be an invalid value.
 * @tparam TEventId_e The enumerated ID of the event. 0 must be an invalid value.
 */
template <typename TStateId_e, typename TEventId_e>
struct EventHandlerMapKey_t {
    TStateId_e stateId;
    TEventId_e eventId;

    constexpr EventHandlerMapKey_t()
        : 
            stateId{},
            eventId{}
        {}

    constexpr EventHandlerMapKey_t(
            TStateId_e state,
            TEventId_e event)
        : 
            stateId(state), 
            eventId(event)
        {}

    /**
     * @brief Overloads the less-than operator (<).
     * Required for etl::const_map.
     * 
     * @param other The other struct to compare with.
     * 
     * @returns The result of the comparison.
     */
    constexpr bool operator < (const EventHandlerMapKey_t &other) const {
        if (eventId != other.eventId) {
            return eventId < other.eventId;
        }
        return stateId < other.stateId;
    }

    /**
     * @brief Overloads the equals operator (==).
     * Both structs must have the same state and event ID.
     * 
     * @param other The other struct to compare with.
     * 
     * @returns The result of the comparison.
     */
    constexpr bool operator == (const EventHandlerMapKey_t &other) const {
        return (stateId == other.stateId) && (eventId == other.eventId);
    }
};

/**
 * @brief Pair used to construct the event handler map.
 * 
 * @tparam TStateId_e The enumerated ID of the state.
 * @tparam TEventId_e The enumerated ID describing all possible events.
 * @tparam TEvent The type of event object passed to event handler functions.
 * @tparam TStateController Parent state controller class.
 */
template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
using EventHandlerMapPair = etl::pair<EventHandlerMapKey_t<TStateId_e, TEventId_e>, EventHandlerMapEntry_t<TEventId_e, TEvent, TStateController>>;

/**
 * @brief Maps all applicable combinations of state ID and event ID to an event handler function.
 * 
 * @tparam TStateId_e The enumerated ID of the state.
 * @tparam TEventId_e The enumerated ID describing all possible events.
 * @tparam TEvent The type of event object passed to event handler functions.
 * @tparam TStateController Parent state controller class.
 */
template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
using EventHandlerMap = etl::const_map<EventHandlerMapKey_t<TStateId_e, TEventId_e>, EventHandlerMapEntry_t<TEventId_e, TEvent, TStateController>, DRIP_EVENT_MAP_MAX_SIZE>;

/** @} */

/**
 * @brief State machine class which handles transitions between states
 * and the handling of events.
 * 
 * @tparam TStateId_e The enumerated ID describing all possible states.
 * @tparam TEventId_e The enumerated ID describing all possible events.
 * @tparam TEvent The type of event object passed to event handler functions.
 * @tparam TStateController Parent state controller class.
 */
template <
    typename TStateId_e, 
    typename TEventId_e, 
    typename TEvent, 
    typename TStateController
>
class StateMachine {
public:
    using StateMachineStateMapKey = TStateId_e;
    using StateMachineStateMapEntry_t = StateHandlerMapEntry_t<TStateController>;
    using StateMachineStateHandlerMap = StateHandlerMap<TStateId_e, TStateController>;
    using StateMachineEventMapKey = EventHandlerMapKey_t<TStateId_e, TEventId_e>;
    using StateMachineEventMapEntry_t = EventHandlerMapEntry_t<TEventId_e, TEvent, TStateController>;
    using StateMachineEventHandlerMap = EventHandlerMap<TStateId_e, TEventId_e, TEvent, TStateController>;

    /**
     * @brief Constructor.
     * 
     * @param initialState The initial state of the state machine. 
     * @param name A name for the FSM, used as the "TAG" parameter when logging.
     * Must be a pointer to a const null terminated string.
     * @param stateHandlerMap State handler map.
     * @param eventHandlerMap Event handler map.
     * @param stateController Reference to the parent StateController class.
     * @param dataContainer Reference to the DataContainer for logging.
     */
    StateMachine(
        TStateId_e initialState,
        const char* name,
        const StateMachineStateHandlerMap &stateHandlerMap, 
        const StateMachineEventHandlerMap &eventHandlerMap,
        TStateController &stateController,
        DataContainer &dataContainer
    );

    /**
     * @brief Get the Current FSM state.
     * 
     * @return TStateId_e Current state ID.
     */
    TStateId_e getCurrentState() const;

    /**
     * @brief Updates the current FSM state.
     * 
     * @retval ESP_OK. If the update handler function was called.
     * @retval ESP_ERR_INVALID_STATE If the state map is invalid.
     */
    esp_err_t update();

    /**
     * @brief Transitions to a new FSM state.
     * Calls the exit handler for the current state
     * and the entry handler for the new state. 
     * 
     * @param newState The state to enter.
     * 
     * @retval ESP_OK If the exit and entry handler
     * functions were called.
     * @retval ESP_ERR_INVALID_STATE If the state map is invalid.
     */
    esp_err_t transition(TStateId_e newState);

    /**
     * @brief Handles an event.
     * 
     * @param eventId The ID of the event to handle.
     * @param event The event object.
     * 
     * @retval ESP_OK If the event handler function was called.
     * @retval ESP_ERR_INVALID_STATE If the event map is invalid.
     */
    esp_err_t handleEvent(TEventId_e eventId, const TEvent *event);

private:
    /** @brief The current state. */
    TStateId_e currentState_;
    /** @brief The next state. */
    TStateId_e nextState_;
    /** @brief Stores the last tick at which the current state was handled. */
    TickType_t lastStateHandlingTimestampTicks_;
    /** @brief Title for the state machine. */
    const char *name_;
    /** @brief The state and event handler maps. */
    const StateMachineStateHandlerMap &stateMap_;
    const StateMachineEventHandlerMap &eventMap_;
    /** @brief Reference to the parent StateController class. */
    TStateController &stateController_;
    /** @brief Reference to the DataContainer. */
    DataContainer &dataContainer_;
};

/*************************
 * Implementation
 ************************/

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
StateMachine<TStateId_e, TEventId_e, TEvent, TStateController>::StateMachine(
    TStateId_e initialState,
    const char* name,
    const StateHandlerMap<TStateId_e, TStateController> &stateMap, 
    const EventHandlerMap<TStateId_e, TEventId_e, TEvent, TStateController> &eventMap,
    TStateController &stateController,
    DataContainer &dataContainer
) : 
    currentState_(initialState), 
    lastStateHandlingTimestampTicks_(0U), 
    name_(name),
    stateMap_(stateMap), 
    eventMap_(eventMap), 
    stateController_(stateController),
    dataContainer_(dataContainer) {
}
template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
TStateId_e StateMachine<TStateId_e, TEventId_e, TEvent, TStateController>::getCurrentState() const {
    return currentState_;
}

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
esp_err_t StateMachine<TStateId_e, TEventId_e, TEvent, TStateController>::update() {
    TickType_t currentTick = xTaskGetTickCount();

    /** Catch invalid state map. */
    if (!stateMap_.contains(currentState_)) {
        dataContainer_.logError(
            ESP_ERR_INVALID_STATE, 
            name_, 
            "Error when updating FSM state: %d. State is not contained within the handler map.", 
            static_cast<int>(currentState_)
        );
        return ESP_ERR_INVALID_STATE;
    }
    
    /** Retrieve state map entry. */
    const StateMachineStateMapEntry_t currentStateMapEntry = stateMap_.at(currentState_);

    /** Catch invalid state map. */
    if ( (currentStateMapEntry.entryFunc == nullptr) || 
         (currentStateMapEntry.updateFunc == nullptr) || 
         (currentStateMapEntry.exitFunc == nullptr) ) {
        dataContainer_.logError(
            ESP_ERR_INVALID_STATE, 
            name_, 
            "Error when updating FSM state: %d. State has an undefined handler function.", 
            static_cast<int>(currentState_)
        );
        return ESP_ERR_INVALID_STATE;
    }

    /* Handle transitions first. */
    if (nextState_ != currentState_) {
        /** Retrieve state map entry. */
        const StateMachineStateMapEntry_t nextStateMapEntry = stateMap_.at(nextState_);

        /** Catch invalid state map. */
        if ( (nextStateMapEntry.entryFunc == nullptr) || 
            (nextStateMapEntry.updateFunc == nullptr) || 
            (nextStateMapEntry.exitFunc == nullptr) ) {
            dataContainer_.logError(
                ESP_ERR_INVALID_STATE, 
                name_, 
                "Error when updating FSM state: %d to %d. Next state has an undefined handler function.", 
                static_cast<int>(currentState_),
                static_cast<int>(nextState_)
            );
            return ESP_ERR_INVALID_STATE;
        }

        /** Exit the current state. */
        dataContainer_.logInfo(ESP_OK, name_, "Exiting FSM state: %d", static_cast<int>(currentState_));
        (stateController_.*currentStateMapEntry.exitFunc)();

        /** Enter the next state. */
        currentState_ = nextState_;
        lastStateHandlingTimestampTicks_ = 0U;
        dataContainer_.logInfo(ESP_OK, name_, "Entering FSM state: %d", static_cast<int>(currentState_));
        (stateController_.*nextStateMapEntry.entryFunc)();

    } else {
        /** Handle the current state on its handling interval. */
        if ( (currentTick - lastStateHandlingTimestampTicks_) >= pdMS_TO_TICKS(currentStateMapEntry.handlingIntervalMs) ) {
            lastStateHandlingTimestampTicks_ = currentTick;
            
            /** Update the state. */
            dataContainer_.logInfo(ESP_OK, name_, "Updating FSM state: %d", static_cast<int>(currentState_));
            (stateController_.*currentStateMapEntry.updateFunc)();
        }
    }


    return ESP_OK;
}

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
esp_err_t StateMachine<TStateId_e, TEventId_e, TEvent, TStateController>::transition(TStateId_e newState) {
    /** Ignore no-op transitions. */
    if (newState == currentState_) {
        return ESP_OK;
    }

    /** Catch invalid state map. */
    if (!stateMap_.contains(currentState_)) {
        dataContainer_.logError(
            ESP_ERR_INVALID_STATE, 
            name_, 
            "Error when exiting FSM state: %d. State is not contained within the handler map.", 
            static_cast<int>(currentState_)
        );
        return ESP_ERR_INVALID_STATE;
        
    } else if (!stateMap_.contains(newState)) {
        dataContainer_.logError(
            ESP_ERR_INVALID_STATE, 
            name_, 
            "Error when entering FSM state: %d. State is not contained within the handler map.", 
            static_cast<int>(newState)
        );
        return ESP_ERR_INVALID_STATE;
    }

    /** Register pending transition. */
    nextState_ = newState;

    return ESP_OK;
}

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
esp_err_t StateMachine<TStateId_e, TEventId_e, TEvent, TStateController>::handleEvent(TEventId_e eventId, const TEvent *event) {
    /** Ignore invalid events. */
    if (nullptr == event) {
        return ESP_ERR_INVALID_ARG;
    }
    
    const StateMachineEventMapKey key = {
        .stateId = currentState_,
        .eventId = eventId
    };
    StateMachineEventMapEntry_t eventMapEntry = {};

    /** Ignore events in invalid states. */
    if (!eventMap_.contains(key)) {
        return ESP_OK;
    }

    /** Retrieve event map entry. */
    eventMapEntry = eventMap_.at(key);

    /** Catch invalid event map. */
    if (eventMapEntry.handleFunc == nullptr) {
        dataContainer_.logError(
            ESP_ERR_INVALID_STATE, 
            name_, 
            "Error when handling FSM event: %d in state: %d. Event has no defined handler function.", 
            static_cast<int>(eventId),
            static_cast<int>(currentState_)
        );
        return ESP_ERR_INVALID_STATE;
    }

    /** Handle the event. */
    (stateController_.*eventMapEntry.handleFunc)(eventId, event);

    return ESP_OK;
}

#endif
