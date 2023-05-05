// controller.ino

#include "config.h"    // Provides default configuration values
#include "pins.h"      // Interactions with pins and peripherals
#include "file.h"      // Provides API for the SPIFFS file system
#include "network.h"   // MQTT and network setup functions
#include "services.h"  // MQTT input and output relations to device state
#include "app.h"       // Device state management

void setup() {

  SLOG.begin(115200);
  SLOG.println();

  file::init_files();
  net::init_network();
  app::init_app();
  pins::init_pins();
  
}

void loop() {
  net::loop_mqtt();
  app::loop_app();
  delay(100);
}