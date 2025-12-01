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

    void update();

    void handleEvent(TEventId_e eventId, TEvent event);

private:
    StateMachine<TStateId_e, TEventId_e, TEvent, TStateController> machine_;

    virtual void preUpdate();
    virtual void postUpdate();
};