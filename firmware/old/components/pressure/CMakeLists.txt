idf_component_register(SRCS "pressureManager.cpp" "pressureDriver.cpp"
						INCLUDE_DIRS .
						REQUIRES config
						PRIV_REQUIRES
)