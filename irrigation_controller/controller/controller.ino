// controller.ino

#include "config.h"    // Provides default configuration values
#include "file.h"      // Provides API for the SPIFFS file system
#include "network.h"   // MQTT and network setup functions
#include "services.h"  // MQTT input and output relations to device state
#include "app.h"     // Device state management

void setup() {

  SLOG.begin(115200);
  SLOG.println();

  file::init_files();
  net::init_network();
  app::init_app();
}

void loop() {
  net::loop_mqtt();
  app::loop_app();
}