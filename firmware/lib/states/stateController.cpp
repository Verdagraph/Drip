#include "stateController.h"

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
StateController<TStateId_e, TEventId_e, TEvent, TStateController>::StateController(
    TStateId_e initialState,
    const char* name,
    const StateHandlerMap &stateHandlerMap, 
    const EventHandlerMap &eventHandlerMap,
    const TStateController &stateController,
    const DataContainer &dataContainer
) : machine_(initialState, name, stateHandlerMap, eventHandlerMap, stateController, dataContainer) {}

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
void StateController<TStateId_e, TEventId_e, TEvent, TStateController>::update() {
    machine_.update();
}

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
void StateController<TStateId_e, TEventId_e, TEvent, TStateController>::handleEvent(TEventId_e eventId, const TEvent *event) {
    machine_.handleEvent(eventId, event);
}
