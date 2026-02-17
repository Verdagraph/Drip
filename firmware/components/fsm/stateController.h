/**
 * @file stateController.h
 * @author Nathaniel King
 * @brief Encapsulates the StateMachine.
 * @date 2025-11-30
 * 
 */

#ifndef STATE_CONTROLLER_H
#define STATE_CONTROLLER_H

#include "dataContainer.h"
#include "stateMachine.h"

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
class StateController {
public:
    StateController(
        TStateId_e initialState,
        const char* name,
        const StateHandlerMap<TStateId_e, TStateController> &stateHandlerMap, 
        const EventHandlerMap<TStateId_e, TEventId_e, TEvent, TStateController> &eventHandlerMap,
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
     * @brief Updates the state machine
     */
    void update();

    /**
     * @brief Handles an event.
     * 
     * @param eventId Event ID.
     * @param event Event object.
     */
    void handleEvent(TEventId_e eventId, const TEvent *event);

protected:
    StateMachine<TStateId_e, TEventId_e, TEvent, TStateController> machine_;

    /**
     * @brief Called prior to the state machine update.
     */
    virtual void preUpdate() {}

    /**
     * @brief Called after the state machine update.
     */
    virtual void postUpdate() {}
};

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
StateController<TStateId_e, TEventId_e, TEvent, TStateController>::StateController(
    TStateId_e initialState,
    const char* name,
    const StateHandlerMap<TStateId_e, TStateController> &stateHandlerMap, 
    const EventHandlerMap<TStateId_e, TEventId_e, TEvent, TStateController> &eventHandlerMap,
    TStateController &stateController,
    DataContainer &dataContainer
) : machine_(initialState, name, stateHandlerMap, eventHandlerMap, stateController, dataContainer) {}

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
TStateId_e StateController<TStateId_e, TEventId_e, TEvent, TStateController>::getCurrentState() const {
    return machine_.getCurrentState();
}

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
void StateController<TStateId_e, TEventId_e, TEvent, TStateController>::update() {
    preUpdate();
    machine_.update();
    postUpdate();
}

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
void StateController<TStateId_e, TEventId_e, TEvent, TStateController>::handleEvent(TEventId_e eventId, const TEvent *event) {
    machine_.handleEvent(eventId, event);
}

#endif