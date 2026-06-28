idf_component_register(SRCS "mainStateController.cpp"
						INCLUDE_DIRS .
						REQUIRES etl esp_common container config fsm messages
						PRIV_REQUIRES freertos
)