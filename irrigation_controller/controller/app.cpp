// app.cpp

#include <Arduino.h>

#include "config.h"
#include "file.h"
#include "services.h"
#include "pins.h"

#include "app.h"

app::DeviceState app::env;

void app::init_app() {
  file::read_config(&app::env);
}

void loop_deactivate() {
  
}



// Update the state variables with the new flow sensor data
void update_output_volume_sensor(){
  if (app::env.sensor.pulses > app::env.sensor.last_pulses) {

    // Update pulse delta
    app::env.slice.pulses_elapsed = app::env.sensor.pulses - app::env.sensor.last_pulses;
    app::env.sensor.last_pulses = app::env.sensor.pulses;

    // Update volume delta
    app::env.slice.output_volume_elapsed = app::env.slice.pulses_elapsed / app::env.flow_sensor_config.pulses_per_l;
    app::env.slice.total_output_volume += app::env.slice.output_volume_elapsed;

    // Update flow rate delta and averages
    app::env.slice.flow_rate = (app::env.slice.output_volume_elapsed / app::env.slice.time_elapsed) * 1000 * 60;
    app::env.slice.current_avg_flow += app::env.slice.flow_rate;
    app::env.slice.avg_flow_count++;
  }
}

// Update state variables using static flow rate 
void update_output_volume_static(){
  if (app::env.slice.time_elapsed > 0) {

    // Update volume delta
    app::env.slice.output_volume_elapsed = (app::env.slice.time_elapsed / (1000 * 60)) * app::env.source_config.static_flow_rate;
    app::env.slice.total_output_volume += app::env.slice.output_volume_elapsed;

  }
}

// End the dispensation process
void close_dispense_process() {

  //If the dispensing process is ended without switching to the source, set the tank volume to the volume
  if (USING_TANK_ && !app::env.flag.resevoir_switch_flag) {
    app::env.report.total_tank_output_volume = app::env.slice.total_output_volume;
  }

  pins::close_tank_output();
  pins::close_source_output();

  app::env.flag.dispense_flag = false;
  app::env.time.process_begin_timestamp = 0;

}

void dispense_report_slice(bool cache) {

  float avg_flow = 0;
  // Update flow rate based on whether the flow sensor is in use
  if ((USING_TANK_ && USING_SOURCE_ && !app::env.flag.resevoir_switch_flag) || (USING_TANK_ && !USING_SOURCE_)) {
      avg_flow = app::env.slice.current_avg_flow / app::env.slice.avg_flow_count;
  } else if ((USING_SOURCE_ && USING_TANK_ && app::env.flag.resevoir_switch_flag) || (USING_SOURCE_ && !USING_TANK_)) {
    if (USING_SOURCE_FLOW) {
      avg_flow = app::env.slice.current_avg_flow / app::env.slice.avg_flow_count;
    } else {
      avg_flow = app::env.source_config.static_flow_rate;
    }
  }

  float avg_pressure = 0;
  if (USING_PRESSURE_SENSOR_) {
    avg_pressure = app::env.slice.current_avg_tank_pressure / app::env.slice.avg_tank_count;
  }

  srvc::publish_dispense_slice_report(cache, app::env.slice.total_time_elapsed, app::env.slice.total_output_volume, avg_flow, avg_pressure);

  app::env.slice.avg_flow_count = 0;
  app::env.slice.current_avg_flow = 0;
  app::env.slice.current_avg_tank_pressure = 0;
  app::env.slice.avg_tank_count = 0;

  
}

void dispense_report_summary() {

  float tank_volume = 0;
  unsigned long int tank_time = 0;
  if (USING_TANK_) {
    tank_volume = app::env.report.total_tank_output_volume;
    tank_time = app::env.time.resevoir_switch_timestamp - app::env.time.process_begin_timestamp;
  }

  srvc::publish_dispense_summary_report(app::env.slice.total_time_elapsed, app::env.slice.total_output_volume, tank_volume, tank_time);

}

void loop_dispensation(){

  if (app::env.flag.dispense_flag) {

    // First time slice
    if (app::env.time.process_begin_timestamp == 0) {

      app::env.flag.drain_flag = false;
      app::env.flag.deactivate_flag = false;
      app::env.flag.resevoir_switch_flag = false;

      app::env.time.process_begin_timestamp = millis();
      app::env.time.last_timestamp = app::env.time.process_begin_timestamp;
      app::env.time.resevoir_switch_timestamp = 0;

      app::env.sensor.pulses = 0;
      app::env.sensor.last_pulses = 0;
      app::env.sensor.pressure = 0;
      app::env.sensor.last_pressure = 0;

      if (USING_TANK_) {
        pins::open_tank_output();
        if (USING_SOURCE_){
          pins::close_source_output();
        }
      } else {
        pins::open_source_output();
      }

    }

    // Update state based on mode
    if ((USING_TANK_ && USING_SOURCE_ && !app::env.flag.resevoir_switch_flag) || (USING_TANK_ && !USING_SOURCE_)) {
      update_output_volume_sensor();
    } else if ((USING_SOURCE_ && USING_TANK_ && app::env.flag.resevoir_switch_flag) || (USING_SOURCE_ && !USING_TANK_)) {
      if (USING_SOURCE_FLOW) {
        update_output_volume_sensor();
      } else {
        update_output_volume_static();
      }
    }

    // Last time slice report and exit
    if (app::env.slice.total_output_volume >= app::env.target.target_output_volume) {

      close_dispense_process();
      dispense_report_slice(false);
      dispense_report_summary();

    // Normal slice report
    } else if ((app::env.slice.total_output_volume - app::env.report.last_output_volume_report) >= app::env.services_config.data_resolution_l) {

      dispense_report_slice(true);

    }

    // Tank timeout
    if (USING_TANK_ && (app::env.slice.flow_rate < app::env.flow_sensor_config.min_flow_rate) && (app::env.slice.total_time_elapsed > app::env.tank_config.tank_timeout)) {
      if (USING_SOURCE_) {

        app::env.flag.resevoir_switch_flag = true;
        app::env.report.total_tank_output_volume = app::env.slice.total_output_volume;

        pins::open_source_output();
        pins::close_tank_output();

      } else {

        close_dispense_process();
        dispense_report_slice(false);
        dispense_report_summary();

      }
    }

  }



}

void loop_drain(){

}

void app::loop_app(){

  // Update timestamps
  unsigned long int current_time = millis();
  app::env.slice.time_elapsed = current_time - app::env.time.last_timestamp;
  app::env.time.last_timestamp = current_time;
  if (app::env.time.process_begin_timestamp > 0) {
    app::env.slice.total_time_elapsed = current_time - app::env.time.process_begin_timestamp;
  }

  loop_deactivate();
  loop_dispensation();

  if (USING_DRAIN_VALVE_) {
    loop_drain();
  }
}