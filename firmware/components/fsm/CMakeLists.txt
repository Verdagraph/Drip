idf_component_register(SRCS "stateManager.cpp"
						INCLUDE_DIRS .
						REQUIRES esp_common config messages container mqtt connection valves
						PRIV_REQUIRES freertos
)