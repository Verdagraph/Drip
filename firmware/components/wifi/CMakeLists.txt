idf_component_register(SRCS "wifiManager.cpp"
						INCLUDE_DIRS .
						REQUIRES esp_common esp_wifi esp_http_server esp_littlefs nvs_flash config container
						PRIV_REQUIRES
)