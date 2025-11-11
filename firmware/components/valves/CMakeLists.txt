idf_component_register(SRCS "valveManager.cpp"
						INCLUDE_DIRS .
						REQUIRES config gpio container
						PRIV_REQUIRES
)