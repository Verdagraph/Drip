idf_component_register(SRCS "dataContainer.cpp"
						INCLUDE_DIRS .
						REQUIRES esp_common valves flow connection
						PRIV_REQUIRES
)