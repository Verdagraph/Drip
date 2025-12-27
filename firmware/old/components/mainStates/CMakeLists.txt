idf_component_register(SRCS "mainStateController.cpp"
						INCLUDE_DIRS .
						REQUIRES esp_common config fsm messages utils container mqtt wifi valves flow
						PRIV_REQUIRES freertos
)