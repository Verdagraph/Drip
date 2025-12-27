// pins.cpp

#include <Arduino.h>
#include "Adafruit_MPRLS.h"

#include "services.h"
#include "app.h"

#include "pins.h"

//Every time the flow sensor sends a pulse, increment the pulse count
ICACHE_RAM_ATTR void Flow(){
  app::env.sensor.pulses++;
}

Adafruit_MPRLS pressure_sensor = Adafruit_MPRLS(-1, -1);

void pins::init_pins() {

  if (USING_SOURCE_) {
    SLOG.println("Initializing source output valve pin");
    pinMode(SOURCE_OUTPUT_VALVE_PIN_, OUTPUT);
    digitalWrite(SOURCE_OUTPUT_VALVE_PIN_, LOW);
    SLOG.println("Source output valve pin initialized");
  }

  if (USING_TANK_) {
    SLOG.println("Initializing tank output valve pin");
    pinMode(TANK_OUTPUT_VALVE_PIN_, OUTPUT);
    digitalWrite(TANK_OUTPUT_VALVE_PIN_, LOW);
    SLOG.println("Tank output valve pin initialized");
    if (USING_DRAIN_VALVE_) {
      SLOG.println("Initializing tank drain valve pin");
      pinMode(TANK_DRAIN_VALVE_PIN_, OUTPUT);
      digitalWrite(TANK_DRAIN_VALVE_PIN_, LOW);
      SLOG.println("Tank drain valve pin initialized");
    }
  }

  if (USING_FLOW_SENSOR_) {
    SLOG.println("Initializing flow sensor pin");
    pinMode(FLOW_SENSOR_PIN_, INPUT_PULLUP);
    //Attach the Flow() interrupt function to the flow sensor pin
    attachInterrupt(digitalPinToInterrupt(USING_FLOW_SENSOR_), Flow, RISING);
    SLOG.println("Flow sensor pin initialized");
  }

  if (USING_PRESSURE_SENSOR_) {
    SLOG.println("Initializing pressure sensor pin");
    if (!pressure_sensor.begin()) {
      char message[] = "Pressure sensor failed to initialize... Restarting";
      srvc::error(message);
      delay(5000);
      ESP.restart();
    }
    SLOG.println("Pressure sensor initialized");
    srvc::publish_pressure_report();
  }


}

void pins::open_source_output() {
  if (!USING_SOURCE_) {return;}
  digitalWrite(SOURCE_OUTPUT_VALVE_PIN_, HIGH);
}

void pins::close_source_output() {
  if (!USING_SOURCE_) {return;}
  digitalWrite(SOURCE_OUTPUT_VALVE_PIN_, LOW);
}

void pins::open_tank_output() {
  if (!USING_TANK_) {return;}
  digitalWrite(TANK_OUTPUT_VALVE_PIN_, HIGH);
}

void pins::close_tank_output() {
  if (!USING_TANK_) {return;}
  digitalWrite(TANK_OUTPUT_VALVE_PIN_, LOW);
}

void pins::open_tank_drain() {
  if (!USING_DRAIN_VALVE_) {return;}
  digitalWrite(TANK_DRAIN_VALVE_PIN_, HIGH);
}

void pins::close_tank_drain() {
  if (!USING_DRAIN_VALVE_) {return;}
  digitalWrite(TANK_DRAIN_VALVE_PIN_, LOW);
}

float pins::read_pressure() {
  if (!USING_PRESSURE_SENSOR_) {return -1;}
  return pressure_sensor.readPressure() - app::env.pressure_sensor_config.atmosphere_pressure;
}