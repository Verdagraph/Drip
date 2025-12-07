/**
 * @file stateController.h
 * @author Nathaniel King
 * @brief Encapsulates the StateMachine.
 * @version 0.1
 * @date 2025-11-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "stateMachine.h"

template <typename TStateId_e, typename TEventId_e, typename TEvent, typename TStateController>
class StateController {
public:
    using StateMapKey = TStateId_e;
    using StateMapEntry_t = StateHandlerMapEntry_t<TStateController>;
    using StateHandlerMap = StateHandlerMap<TStateId_e, TStateController, TStateMapSize>;
    using EventMapKey = EventHandlerMapKey_t<TStateId_e, TEventId_e>;
    using EventMapEntry_t = EventHandlerMapEntry_t<TEventId_e, TEvent, TStateController>;
    using EventHandlerMap = EventHandlerMap<TStateId_e, TEventId_e, TEvent, TStateController, TEventMapSize>;

    StateController(
        TStateId_e initialState,
        const char* name,
        const StateHandlerMap &stateHandlerMap, 
        const EventHandlerMap &eventHandlerMap,
        const TStateController &stateController,
        const DataContainer &dataContainer
    );

    void update();

    void handleEvent(TEventId_e eventId, TEvent event);

protected:
    StateMachine<TStateId_e, TEventId_e, TEvent, TStateController> machine_;

    virtual void preUpdate();
    virtual void postUpdate();
};