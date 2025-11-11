idf_component_register(SRCS "main.cpp"
						INCLUDE_DIRS .
						REQUIRES esp_common fsm config wifi mqtt valves
						PRIV_REQUIRES freertos
)