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

    // Handle sensor overload
    if (app::env.slice.flow_rate > app::env.flow_sensor_config.max_flow_rate) {

      char message[] = "Flow rate exceeded maximum";
      SLOG.println(message);
      srvc::publish_log(1, message);

    }

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

void update_tank_pressure() {

  if (!USING_PRESSURE_SENSOR) {return;}

  // Update tank pressure and averages
  app::env.sensor.pressure = pins::read_pressure();
  app::env.slice.current_avg_tank_pressure += app::env.sensor.pressure;
  app::env.slice.avg_tank_count++;

}

// End the dispensation process
void close_dispense_process() {

  //If the dispensing process is ended without switching to the source, set the tank volume to the volume
  if (USING_TANK_ && !app::env.flag.resevoir_switch_flag && app::env.flag.dispense_flag) {
    app::env.report.total_tank_output_volume = app::env.slice.total_output_volume;
  }

  if (USING_SOURCE_) {
      pins::close_source_output();
    }

  if (USING_TANK_) {
    pins::close_tank_output();
  }

  app::env.flag.dispense_flag = false;
  app::env.time.process_begin_timestamp = 0;

  char message[] = "Ending dispensation process";
  SLOG.println(message);
  srvc::publish_log(0, message);

}

void dispense_report_slice() {

  // Update flow rate based on whether the flow sensor is in use
  float avg_flow = 0;
  if ((USING_TANK_ && USING_SOURCE_ && !app::env.flag.resevoir_switch_flag) || (USING_TANK_ && !USING_SOURCE_)) {
      avg_flow = app::env.slice.current_avg_flow / app::env.slice.avg_flow_count;
      app::env.slice.avg_flow_count = 0;
      app::env.slice.current_avg_flow = 0;
  } else if ((USING_SOURCE_ && USING_TANK_ && app::env.flag.resevoir_switch_flag) || (USING_SOURCE_ && !USING_TANK_)) {
    if (USING_SOURCE_FLOW) {
      avg_flow = app::env.slice.current_avg_flow / app::env.slice.avg_flow_count;
      app::env.slice.avg_flow_count = 0;
      app::env.slice.current_avg_flow = 0;
    } else {
      avg_flow = app::env.source_config.static_flow_rate;
    }
  }

  // Update pressure based on whether pressure sensor is in use
  float avg_pressure = 0;
  if (USING_PRESSURE_SENSOR_) {
    avg_pressure = app::env.slice.current_avg_tank_pressure / app::env.slice.avg_tank_count;
    app::env.slice.current_avg_tank_pressure = 0;
    app::env.slice.avg_tank_count = 0;
  }

  srvc::publish_dispense_slice_report(app::env.slice.total_time_elapsed, app::env.slice.total_output_volume, avg_flow, avg_pressure);
  SLOG.println("Reported dispensation slice");

}

void dispense_report_summary() {

  float tank_volume = 0;
  unsigned long int tank_time = 0;
  if (USING_TANK_) {
    tank_volume = app::env.report.total_tank_output_volume;
    tank_time = app::env.time.resevoir_switch_timestamp - app::env.time.process_begin_timestamp;
  }

  srvc::publish_dispense_summary_report(app::env.slice.total_time_elapsed, app::env.slice.total_output_volume, tank_volume, tank_time);
  SLOG.println("Reported dispensation summary");

}

void loop_deactivate() {

  if (app::env.flag.deactivate_flag) {

    if (app::env.flag.dispense_flag) {
      close_dispense_process();
      dispense_report_slice();
      dispense_report_summary();
    }

    if (app::env.flag.drain_flag) {

    }

    app::env.flag.dispense_flag = false;
    app::env.flag.dispense_flag = false;
    app::env.flag.drain_flag = false;

    if (USING_SOURCE_) {
      pins::close_source_output();
    }

    if (USING_TANK_) {
      pins::close_tank_output();
    }

    if(USING_DRAIN_VALVE_) {
      pins::close_tank_drain();
    }

    app::env.flag.deactivate_flag = false;
    app::env.time.process_begin_timestamp = 0;

    char message[] = "Deactivation successful";
    SLOG.println(message);
    srvc::publish_log(0, message);

  }
  
}

void loop_dispensation(){

  if (!app::env.flag.dispense_flag) {return;}

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

    app::env.slice.total_output_volume = 0;
    app::env.slice.current_avg_flow = 0;
    app::env.slice.avg_flow_count = 0;
    app::env.slice.current_avg_tank_pressure = 0;
    app::env.slice.avg_tank_count = 0;

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

  if (USING_PRESSURE_SENSOR_) {
    update_tank_pressure();
  } 

  // Last time slice report and exit
  if (app::env.slice.total_output_volume >= app::env.target.target_output_volume) {

    close_dispense_process();
    dispense_report_slice();
    dispense_report_summary();

  // Normal slice report
  } else if ((app::env.slice.total_output_volume - app::env.report.last_output_volume_report) >= app::env.services_config.data_resolution_l) {

    dispense_report_slice();
    app::env.report.last_output_volume_report = app::env.slice.total_output_volume;

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
      dispense_report_slice();
      dispense_report_summary();

    }
  }

}

// End the drain process
void close_drain_process() {

  if (!USING_DRAIN_VALVE_) {return;}

  pins::close_tank_drain();
  
  app::env.flag.drain_flag = false;
  app::env.time.process_begin_timestamp = 0;

  char message[] = "Ending drain process";
  SLOG.println(message);
  srvc::publish_log(0, message);

}

void drain_report_summary() {

  float tank_volume = 0;
  unsigned long int tank_time = 0;
  if (USING_TANK_) {
    tank_volume = app::env.report.total_tank_output_volume;
    tank_time = app::env.time.resevoir_switch_timestamp - app::env.time.process_begin_timestamp;
  }

  // Total time elapsed, start pressure, start volume, end pressure, end volume
  float end_pressure = 0;
  float start_volume = 0;
  float end_volume = 0;
  if (USING_PRESSURE_SENSOR_) {
    end_pressure = app::env.sensor.pressure;
    start_volume = app::pressure_to_volume(app::env.report.drain_start_pressure);
    end_volume = app::pressure_to_volume(end_pressure);
  }

  srvc::publish_drain_summary_report(app::env.slice.total_time_elapsed, app::env.report.drain_start_pressure, end_pressure, start_volume, end_volume);
  SLOG.println("Reported drain summary");

}

void loop_drain(){

  if (!app::env.flag.drain_flag) {return;}

  // First time slice
  if (app::env.time.process_begin_timestamp == 0) {

    app::env.flag.dispense_flag = false;
    app::env.flag.deactivate_flag = false;

    app::env.time.process_begin_timestamp = millis();
    app::env.time.last_timestamp = app::env.time.process_begin_timestamp;
    
    app::env.report.drain_start_pressure = 0;
    if (USING_PRESSURE_SENSOR_) {
      app::env.report.drain_start_pressure = pins::read_pressure();
    }

    if (USING_SOURCE_) {
      pins::close_source_output();
    }
    if (USING_TANK_) {
      pins::close_tank_output();
    }

    pins::open_tank_drain();

  }

  // Update the pressure sensor if using
  if (USING_PRESSURE_SENSOR_) {
    update_tank_pressure();
  } 


  // Check end condition
  if (app::env.target.target_drain_time > 0) {

    if (app::env.slice.total_time_elapsed > app::env.target.target_drain_time) {
      close_drain_process();
      drain_report_summary();
    }

  } else if (app::env.target.target_drain_volume > 0) {

    float volume = app::pressure_to_volume(app::env.sensor.pressure);
    if (volume <= app::env.target.target_drain_volume) {
      close_drain_process();
      drain_report_summary();
    }

  } else if (app::env.target.target_drain_pressure > 0) {
    
    if (app::env.sensor.pressure <= app::env.target.target_drain_pressure) {
      close_drain_process();
      drain_report_summary();
    }

  }

}

void log_state() {

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

  if (DEBUG) {log_state();}

}

float pressure_to_volume (float pressure) {
  // Meters = (hPa / 10) / (kg/m^3 * m/s^2) = (kPa) / (kg/s^2*m^2) = (kPa) / (kPa / m)
  float height = (pressure / 10) / DENSITY_GRAVITY;

  switch (app::env.tank_config.shape_type) {
    case 1: // Rectangular prism: length, width, height
      return app::env.tank_config.dimension_1 * app::env.tank_config.dimension_2 * height;
    case 2: // Cylinder: radius, height, N/A
      return PI * app::env.tank_config.dimension_1 * app::env.tank_config.dimension_1 * height;
    default:
      return -1;
  }
}