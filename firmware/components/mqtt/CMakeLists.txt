idf_component_register(SRCS "mqttManager.cpp"
						INCLUDE_DIRS .
						REQUIRES esp_common config messages valves
						PRIV_REQUIRES freertos
)