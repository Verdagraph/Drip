idf_component_register(SRCS "main.cpp"
						INCLUDE_DIRS .
						REQUIRES esp_common fsm config connection mqtt
						PRIV_REQUIRES freertos
)