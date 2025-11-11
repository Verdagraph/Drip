idf_component_register(SRCS "gpioDriver.cpp"
						INCLUDE_DIRS .
						REQUIRES config esp_driver_i2c
						PRIV_REQUIRES
)