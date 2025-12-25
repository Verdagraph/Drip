idf_component_register(SRCS "stateController.cpp" "stateMachine.cpp"
						INCLUDE_DIRS .
						REQUIRES container
						PRIV_REQUIRES freertos
)