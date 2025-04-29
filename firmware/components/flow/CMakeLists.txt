idf_component_register(SRCS "flowManager.cpp" "flowDriver.cpp"
						INCLUDE_DIRS .
						REQUIRES esp_common
						PRIV_REQUIRES
)