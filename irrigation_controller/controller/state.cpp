// state.cpp

#include <Arduino.h>

#include "config.h"
#include "file.h"
#include "peripherals.h"

#include "state.h"

DeviceState global;

void init_state() {
  read_config(&global);
}

void loop_deactivate() {
  
}



// Update the state variables with the new flow sensor data
void update_output_volume_sensor(){
  if (global.sensor.pulses > global.sensor.last_pulses) {

    // Update pulse delta
    global.slice.pulses_elapsed = global.sensor.pulses - global.sensor.last_pulses;
    global.sensor.last_pulses = global.sensor.pulses;

    // Update volume delta
    global.slice.output_volume_elapsed = global.slice.pulses_elapsed / global.flow_sensor_config.pulses_per_l;
    global.slice.total_output_volume += global.slice.output_volume_elapsed;

    // Update flow rate delta and averages
    global.slice.flow_rate = (global.slice.output_volume_elapsed / global.slice.time_elapsed) * 1000 * 60;
    global.slice.current_avg_rate += global.slice.flow_rate;
    global.slice.avg_flow_count++;
  }
}

// Update state variables using static flow rate 
void update_output_volume_static(){
  if (global.slice.time_elapsed > 0) {

    // Update volume delta
    global.slice.output_volume_elapsed = (global.slice.time_elapsed / (1000 * 60)) * global.source_config.static_flow_rate;
    global.slice.total_output_volume += global.slice.output_volume_elapsed;

  }
}

// End the dispensation process
void close_dispense_process() {

  //If the dispensing process is ended without switching to the source, set the tank volume to the volume
  if (USING_TANK_ && !global.flag.resevoir_switch_flag) {
    global.report.total_tank_output_volume = global.slice.total_output_volume;
  }

}

void dispense_report_slice() {
  
}

void dispense_report_summary() {

}

void loop_dispensation(){

  if (global.flag.dispense_flag) {

    // First time slice
    if (global.time.process_begin_timestamp == 0) {

      global.flag.drain_flag = false;
      global.flag.deactivate_flag = false;
      global.flag.resevoir_switch_flag = false;

      global.time.process_begin_timestamp = millis();
      global.time.last_timestamp = global.time.process_begin_timestamp;
      global.time.resevoir_switch_timestamp = 0;

      global.sensor.pulses = 0;
      global.sensor.last_pulses = 0;
      global.sensor.pressure = 0;
      global.sensor.last_pressure = 0;

      if (USING_TANK_) {
        open_tank_output();
        close_source_output();
      } else {
        close_tank_output();
        open_source_output();
      }

    }

    // Update state based on mode
    if ((USING_TANK_ && USING_SOURCE_ && !global.flag.resevoir_switch_flag) || (USING_TANK_ && !USING_SOURCE_)) {
      update_output_volume_sensor();
    } else if ((USING_SOURCE_ && USING_TANK_ && global.flag.resevoir_switch_flag) || (USING_SOURCE_ && !USING_TANK_)) {
      if (USING_SOURCE_FLOW) {
        update_output_volume_sensor();
      } else {
        update_output_volume_static();
      }
    }

    // Last time slice report and exit
    if (global.slice.total_output_volume >= global.target.target_output_volume) {

      dispense_report_summary();
      close_dispense_process();

    // Normal slice report
    } else if ((global.slice.total_output_volume - global.report.last_output_volume_report) >= global.services_config.data_resolution_l) {

      dispense_report_slice();

    }

    // Tank timeout
    if (USING_TANK_ && (global.slice.flow_rate < global.flow_sensor_config.min_flow_rate) && (global.slice.total_time_elapsed > global.tank_config.tank_timeout)) {
      if (USING_SOURCE_) {

        global.flag.resevoir_switch_flag = true;
        global.report.total_tank_output_volume = global.slice.total_output_volume;

        open_source_output();
        close_tank_output();

      } else {

        dispense_report_summary();
        close_dispense_process();

      }
    }

  }



}

void loop_drain(){

}

void loop_state(){

  // Update timestamps
  unsigned long int current_time = millis();
  global.slice.time_elapsed = current_time - global.time.last_timestamp;
  global.time.last_timestamp = current_time;
  if (global.time.process_begin_timestamp > 0) {
    global.slice.total_time_elapsed = current_time - global.time.process_begin_timestamp;
  }

  loop_deactivate();
  loop_dispensation();
  loop_drain();
}