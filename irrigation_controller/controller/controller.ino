// controller.ino

#include "config.h" // Provides default configuration values
#include "file.h" // Provides API for the SPIFFS file system
#include "network.h" // MQTT API and network functions
#include "services.h" // Main logic layer

void setup() {

  DEBUG_OUT.begin(115200);
  DEBUG_OUT.println();

  init_files();
  init_network();

}

void loop(){}