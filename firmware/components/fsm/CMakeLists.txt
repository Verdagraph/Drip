idf_component_register(SRCS "stateManager.cpp"
						INCLUDE_DIRS .
						REQUIRES esp_common config mqtt connection
						PRIV_REQUIRES freertos
)