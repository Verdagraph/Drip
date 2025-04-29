idf_component_register(SRCS "valveManager.cpp" "valveDriver.cpp"
						INCLUDE_DIRS .
						REQUIRES config
						PRIV_REQUIRES
)