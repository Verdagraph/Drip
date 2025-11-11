idf_component_register(SRCS "stateManager.cpp"
						INCLUDE_DIRS .
						REQUIRES esp_common config messages utils container mqtt wifi valves flow
						PRIV_REQUIRES freertos
)