#include "stateMachine.h"


template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
StateMachine<TStateId_e, TEventId_e, TEvent, TStateController>::StateMachine(
    TStateId_e initialState,
    const char* name,
    const StateHandlerMap &stateMap, 
    const EventHandlerMap &eventMap,
    const TStateController &stateController,
    const DataContainer &dataContainer
) : 
    currentState_(initialState), 
    lastStateHandlingTimestampTicks_(0U), 
    name_{}, 
    stateMap_(stateMap), 
    eventMap_(eventMap), 
    stateController_(stateController),
    dataContainer_(dataContainer) {

    strncpy(name_, name, DRIP_FSM_NAME_MAX_LEN);
}

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
esp_err_t StateMachine<TStateId_e, TEventId_e, TEvent, TStateController>::update() {
    TickType_t currentTick = xTaskGetTickCount();
    StateMapEntry_t currentStateMapEntry = {};

    /** Catch invalid state map. */
    if (!stateMap_.contains(currentState_)) {
        dataContainer_.logError(
            ESP_ERR_INVALID_STATE, 
            name_, 
            "Error when updating FSM state: %d. State is not contained within the handler map.", 
            currentState_
        );
        return ESP_ERR_INVALID_STATE;
    }
    
    /** Retrieve state map entry. */
    currentStateMapEntry = stateMap_.at(currentState_);

    /** Catch invalid state map. */
    if (currentStateMapEntry.updateFunc == nullptr) {
        dataContainer_.logError(
            ESP_ERR_INVALID_STATE, 
            name_, 
            "Error when updating FSM state: %d. State has no defined update function.", 
            currentState_
        );
        return ESP_ERR_INVALID_STATE;
    }

    /** Handle the current state on its handling interval. */
    if ( (currentTick - lastStateHandlingTimestampTicks_) >= pdMS_TO_TICKS(currentStateMapEntry.handlingIntervalMs) ) {
        lastStateHandlingTimestampTicks_ = currentTick;
        
        /** Update the state. */
        dataContainer_.logInfo(ESP_OK, name_, "Updating FSM state: %d", currentState_);
        (stateController.*currentStateEntry.updateFunc)();
    }

    return ESP_OK;
}

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
esp_err_t StateMachine<TStateId_e, TEventId_e, TEvent, TStateController>::transition(TStateId_e newState) {
    StateMapEntry_t currentStateMapEntry = {};
    StateMapEntry_t newStateMapEntry = {};

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
            currentState_
        );
        return ESP_ERR_INVALID_STATE;
        
    } else if (!stateMap_.contains(newState)) {
        dataContainer_.logError(
            ESP_ERR_INVALID_STATE, 
            name_, 
            "Error when entering FSM state: %d. State is not contained within the handler map.", 
            newState
        );
        return ESP_ERR_INVALID_STATE;
    }

    /** Retrieve state map entries. */
    currentStateMapEntry = stateMap_.at(currentState_);
    newStateMapEntry = stateMap_.at(newState);

    /** Catch invalid state map. */
    if (currentStateMapEntry.exitFunc == nullptr) {
        dataContainer_.logError(
            ESP_ERR_INVALID_STATE, 
            name_, 
            "Error when updating FSM state: %d. State has no defined exit function.", 
            currentState_
        );
        return ESP_ERR_INVALID_STATE;

    }
    if (newStateMapEntry.entryFunc == nullptr) {
        dataContainer_.logError(
            ESP_ERR_INVALID_STATE, 
            name_, 
            "Error when updating FSM state: %d. State has no defined enter function.", 
            newState
        );
        return ESP_ERR_INVALID_STATE;
    }

    /** Exit the current state. */
    dataContainer_.logInfo(ESP_OK, name_, "Exiting FSM state: %d", currentState_);
    (stateController_.*currentStateMapEntry.exitFunc)();

    /** Enter the next state. */
    currentState_ = newState;
    lastStateHandlingTimestampTicks_ = 0U;
    dataContainer_.logInfo(ESP_OK, name_, "Entering FSM state: %d", currentState_);
    (stateController.*newStateMapEntry.entryFunc)();

    return ESP_OK;
}

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
esp_err_t StateMachine<TStateId_e, TEventId_e, TEvent, TStateController>::handleEvent(TEventId_e eventId, TEvent event) {
    const EventHandlerKey_t key = {
        .stateId = currentState_,
        .eventId = eventId
    };
    EventMapEntry_t eventMapEntry = {};

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
            eventId,
            currentState_
        );
        return ESP_ERR_INVALID_STATE;
    }

    /** Handle the event. */
    (stateController.*eventEntry.handleFunc)(eventId, event);

    return ESP_OK;
}