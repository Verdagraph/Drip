// app.cpp

#include <Arduino.h>

#include "config.h"
#include "file.h"
#include "network.h"
#include "services.h"
#include "pins.h"

#include "app.h"

app::DeviceState app::env;

void app::init_app() {
  file::read_config(&app::env);
}

// Update the state variables with the new flow sensor data
void update_output_volume_sensor(){

  if (!USING_FLOW_SENSOR_) {return;}

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
      srvc::warning(message);

    }

  }
}

// Update state variables using static flow rate 
void update_output_volume_static(){

  if (!USING_SOURCE_ || (USING_SOURCE_ && USING_SOURCE_FLOW)) {return;}

  if (app::env.slice.time_elapsed > 0) {

    // Update volume delta
    app::env.slice.output_volume_elapsed = (app::env.slice.time_elapsed / (1000 * 60)) * app::env.source_config.static_flow_rate;
    app::env.slice.total_output_volume += app::env.slice.output_volume_elapsed;

  }
}

// If the flow sensor is currently being used for updates
bool currently_updating_flow_sensor() {
  return 
  (USING_TANK_ && !USING_SOURCE_)  || //If usi
  (USING_SOURCE_ && USING_SOURCE_FLOW) ||
  (USING_TANK_ && USING_SOURCE_ && !app::env.flag.resevoir_switch_flag);
}

void update_tank_pressure() {

  if (!USING_PRESSURE_SENSOR) {return;}

  // Update tank pressure and averages
  app::env.sensor.pressure = pins::read_pressure();
  app::env.slice.current_avg_tank_pressure += app::env.sensor.pressure;
  app::env.slice.avg_tank_count++;

}

void app::open_dispense_process(float target_output_volume) {

  app::env.target.target_output_volume = target_output_volume;
  app::env.flag.dispense_flag = true;
  app::env.flag.drain_flag = false;
  app::env.flag.calibration_flag = false;
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
    pins::close_source_output();
  } else {
    pins::open_source_output();
  }
}

// End the dispensation process
void close_dispense_process() {

  if (!app::env.flag.dispense_flag) {return;}

  //If the dispensing process is ended without switching to the source, set the total tank output volume to the volume
  bool tank_only_output = 
  USING_TANK_ &&
  !app::env.flag.resevoir_switch_flag;
  if (tank_only_output) {
    app::env.report.total_tank_output_volume = app::env.slice.total_output_volume;
  }

  pins::close_source_output();
  pins::close_tank_output();

  app::env.flag.dispense_flag = false;
  app::env.time.process_begin_timestamp = 0;

  char message[] = "Ending dispensation process";
  srvc::info(message);

}

void dispense_report_slice() {

  if (!app::env.flag.dispense_flag) {return;}

  // Update flow rate based on whether the flow sensor is in use
  float avg_flow = 0;

  if (currently_updating_flow_sensor()) {
    avg_flow = app::env.slice.current_avg_flow / app::env.slice.avg_flow_count;
    app::env.slice.avg_flow_count = 0;
    app::env.slice.current_avg_flow = 0;
  } else {
    avg_flow = app::env.source_config.static_flow_rate;
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

  if (!app::env.flag.dispense_flag) {return;}

  float tank_volume = 0;
  unsigned long int tank_time = 0;
  if (USING_TANK_) {
    tank_volume = app::env.report.total_tank_output_volume;
    tank_time = app::env.time.resevoir_switch_timestamp - app::env.time.process_begin_timestamp;
  }

  srvc::publish_dispense_summary_report(app::env.slice.total_time_elapsed, app::env.slice.total_output_volume, tank_volume, tank_time);
  SLOG.println("Reported dispensation summary");

}


void loop_dispensation(){

  if (!app::env.flag.dispense_flag) {return;}

  // Update state based on mode
  if (currently_updating_flow_sensor()) {
    update_output_volume_sensor();
  } else {
    update_output_volume_static();
  }
  update_tank_pressure();

  // Last time slice report and exit
  bool end_condition = app::env.slice.total_output_volume >= app::env.target.target_output_volume;
  // Send dispense slice report at volume intervals
  bool slice_report_condition = 
  (app::env.slice.total_output_volume - app::env.report.last_output_volume_report) >= 
  app::env.services_config.data_resolution_l;
  if (end_condition) {

    close_dispense_process();
    dispense_report_slice();
    dispense_report_summary();

  // Normal slice report
  } else if (slice_report_condition) {

    dispense_report_slice();
    app::env.report.last_output_volume_report = app::env.slice.total_output_volume;

  }

  // Tank timeout
  bool tank_timeout = 
  USING_TANK_ && 
  (app::env.slice.flow_rate < app::env.flow_sensor_config.min_flow_rate) && 
  (app::env.slice.total_time_elapsed > (app::env.tank_config.tank_timeout * 1000));
  if (tank_timeout) {
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

void app::open_flow_calibration_process(int id) {

  if (!USING_FLOW_SENSOR_) {return;}

  app::env.report.calibration_id = id;
  app::env.flag.calibration_flag = true;
  app::env.flag.dispense_flag = false;
  app::env.flag.drain_flag = false;

  app::env.time.process_begin_timestamp = millis();
  app::env.time.last_timestamp = app::env.time.process_begin_timestamp;
  app::env.time.last_calibration_action = app::env.time.process_begin_timestamp;

  app::env.report.calibration_state = 1;
  app::env.slice.current_avg_pulses_per_l = 0;
  app::env.slice.avg_calibration_count = 0;

}

void app::close_flow_calibration_process(bool save_calibration) {

  if (!USING_FLOW_SENSOR_) {return;}

  pins::close_source_output();
  pins::close_tank_output();

  app::env.flow_sensor_config.pulses_per_l = app::env.slice.current_avg_pulses_per_l / app::env.slice.avg_calibration_count;

  if (save_calibration) {
    file::save_config(&app::env);
  }

  char message[150];
  snprintf(message, 150, "Ending calibration process with average pulses per liter: %f. %s", app::env.flow_sensor_config.pulses_per_l, (save_calibration ? "Saving to file." : "Not saving to file."));
  srvc::info(message);

  app::env.flag.calibration_flag = false;
  app::env.time.process_begin_timestamp = 0;

}

void app::begin_calibration_dispense(float target_volume) {

  if (!USING_FLOW_SENSOR_) {return;}

  app::env.target.target_calibration_volume = target_volume;
  app::env.slice.total_output_volume = 0;
  app::env.report.calibration_state = 2;
  app::env.time.last_calibration_action = millis();

  app::env.sensor.pulses = 0;
  app::env.sensor.last_pulses = 0;
  app::env.sensor.pressure = 0;

  app::env.slice.total_output_volume = 0;
  app::env.slice.current_avg_flow = 0;
  app::env.slice.avg_flow_count = 0;
  app::env.slice.current_avg_tank_pressure = 0;
  app::env.slice.avg_tank_count = 0;
  app::env.slice.current_avg_pulses_per_l = 0;
  app::env.slice.avg_calibration_count = 0;

  if (USING_SOURCE_ && USING_SOURCE_FLOW) {
    pins::open_source_output();
  } else if(USING_TANK_) {
    pins::open_tank_output();
  }

}

void end_calibration_dispense() {

  if (!USING_FLOW_SENSOR_) {return;}

  pins::close_source_output();
  pins::close_tank_output();
  app::env.report.calibration_state = 3;
  app::env.time.last_calibration_action = millis();
}

void app::take_calibration_measurement(float measured_volume) {

  if (!USING_FLOW_SENSOR_) {return;}

  app::env.slice.current_avg_pulses_per_l += app::env.sensor.pulses / measured_volume;
  app::env.slice.avg_calibration_count++;
  app::env.sensor.pulses = 0;
  app::env.sensor.last_pulses = 0;

  app::env.report.calibration_state = 1;
  app::env.time.last_calibration_action = millis();
}

void loop_flow_calibration_process() {

  if (!USING_FLOW_SENSOR_ || !app::env.flag.calibration_flag) {return;}
  
  // Last calibration time slice timeout
  bool end_condition = 
  millis() - app::env.time.last_calibration_action > 
  (app::env.flow_sensor_config.calibration_timeout * 1000);

  if (app::env.report.calibration_state == 2) {

    update_output_volume_sensor();

    // Last dispense time slice save and exit
    bool end_dispense_condition = 
    (app::env.slice.total_output_volume >= app::env.target.target_calibration_volume) || 
    ((app::env.slice.flow_rate < app::env.flow_sensor_config.min_flow_rate) && 
    (millis() - app::env.time.last_calibration_action > (app::env.tank_config.tank_timeout * 1000)));

    if (end_dispense_condition) {
      update_tank_pressure();
      end_calibration_dispense();
      srvc::publish_pressure_report();
    } 

  } else if (end_condition) {

    app::close_flow_calibration_process(true);

  }

}

void app::open_drain_process(int target_time, float target_pressure, float target_volume) {

  if (!USING_DRAIN_VALVE_) {return;}

  app::env.target.target_drain_time = target_time;
  app::env.target.target_drain_volume = target_pressure;
  app::env.target.target_drain_pressure = target_volume;

  app::env.flag.drain_flag = true;
  app::env.flag.dispense_flag = false;
  app::env.flag.calibration_flag = false;

  app::env.time.process_begin_timestamp = millis();
  app::env.time.last_timestamp = app::env.time.process_begin_timestamp;
  
  app::env.report.drain_start_pressure = 0;
  if (USING_PRESSURE_SENSOR_) {
    app::env.report.drain_start_pressure = pins::read_pressure();
  }

  pins::close_source_output();
  pins::close_tank_output();

  pins::open_tank_drain();
}

// End the drain process
void close_drain_process() {

  if (!USING_DRAIN_VALVE_) {return;}

  pins::close_tank_drain();
  
  app::env.flag.drain_flag = false;
  app::env.time.process_begin_timestamp = 0;

  char message[] = "Ending drain process";
  srvc::info(message);

}

void drain_report_summary() {

  if (!USING_DRAIN_VALVE_) {return;}

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

  if (!USING_DRAIN_VALVE_ || !app::env.flag.drain_flag) {return;}

  // Update the pressure sensor if using
  update_tank_pressure();

  // Check end condition
  if (app::env.target.target_drain_time > 0) {

    bool end_condition = app::env.slice.total_time_elapsed >= app::env.target.target_drain_time;
    if (end_condition) {
      close_drain_process();
      drain_report_summary();
    }

  } else if (app::env.target.target_drain_volume > 0) {

    float volume = app::pressure_to_volume(app::env.sensor.pressure);
    bool end_condition = volume <= app::env.target.target_drain_volume;
    if (end_condition) {
      close_drain_process();
      drain_report_summary();
    }

  } else if (app::env.target.target_drain_pressure > 0) {

    bool end_condition = app::env.sensor.pressure <= app::env.target.target_drain_pressure;
    if (end_condition) {
      close_drain_process();
      drain_report_summary();
    }

  }

}

void app::deactivate() {

  if (app::env.flag.dispense_flag) {
    close_dispense_process();
    dispense_report_slice();
    dispense_report_summary();
    app::env.flag.dispense_flag = false;
  }

  if (app::env.flag.calibration_flag) {
    close_flow_calibration_process(false);
    app::env.flag.calibration_flag = false;
  }

  if (app::env.flag.drain_flag) {
    close_drain_process();
    drain_report_summary();
    app::env.flag.drain_flag = false;
  }

  pins::close_source_output();
  pins::close_tank_output();
  pins::close_tank_drain();

  app::env.time.process_begin_timestamp = 0;

  char message[] = "Deactivation successful";
  srvc::info(message);  

}


void log_state() {

  if (!DEBUG) {return;}

  SLOG.println("*****************");
  SLOG.println("Device state log:");
  SLOG.print("dispense_flag: ");
  SLOG.print(app::env.flag.dispense_flag);
  SLOG.print("calibration_flag: ");
  SLOG.print(app::env.flag.calibration_flag);
  SLOG.print(" drain_flag: ");
  SLOG.print(app::env.flag.drain_flag);
  SLOG.print(" resevoir_switch_flag: ");
  SLOG.print(app::env.flag.resevoir_switch_flag);
  SLOG.print(" mqtt_connected_flag: ");
  SLOG.println(app::env.flag.mqtt_connected_flag);

  SLOG.print("process_begin_timestamp: ");
  SLOG.print(app::env.time.process_begin_timestamp);
  SLOG.print(" last_timestamp: ");
  SLOG.print(app::env.time.last_timestamp);
  SLOG.print(" resevoir_switch_timestamp: ");
  SLOG.println(app::env.time.resevoir_switch_timestamp);
  SLOG.print(" last_calibration_action: ");
  SLOG.println(app::env.time.last_calibration_action);

  SLOG.print("target_output_volume: ");
  SLOG.print(app::env.target.target_output_volume);
  SLOG.print("target_calibration_volume: ");
  SLOG.print(app::env.target.target_calibration_volume);
  SLOG.print(" target_drain_time: ");
  SLOG.print(app::env.target.target_drain_time);
  SLOG.print(" target_drain_volume: ");
  SLOG.print(app::env.target.target_drain_volume);
  SLOG.print(" target_drain_pressure: ");
  SLOG.println(app::env.target.target_drain_pressure);

  SLOG.print("pulses: ");
  SLOG.print(app::env.sensor.pulses);
  SLOG.print(" last_pulses: ");
  SLOG.print(app::env.sensor.last_pulses);
  SLOG.print(" pressure: ");
  SLOG.println(app::env.sensor.pressure);

  SLOG.print("time_elapsed: ");
  SLOG.print(app::env.slice.time_elapsed);
  SLOG.print(" pulses_elapsed: ");
  SLOG.print(app::env.slice.pulses_elapsed);
  SLOG.print(" output_volume_elapsed: ");
  SLOG.print(app::env.slice.output_volume_elapsed);
  SLOG.print(" flow_rate: ");
  SLOG.print(app::env.slice.flow_rate);
  SLOG.print(" total_time_elapsed: ");
  SLOG.print(app::env.slice.total_time_elapsed);
  SLOG.print(" total_output_volume: ");
  SLOG.print(app::env.slice.total_output_volume);
  SLOG.print(" current_avg_flow: ");
  SLOG.print(app::env.slice.current_avg_flow);
  SLOG.print(" avg_flow_count: ");
  SLOG.print(app::env.slice.avg_flow_count);
  SLOG.print(" current_avg_tank_pressure: ");
  SLOG.print(app::env.slice.current_avg_tank_pressure);
  SLOG.print(" avg_tank_count: ");
  SLOG.println(app::env.slice.avg_tank_count);
  SLOG.print(" current_average_pulses_per_l: ");
  SLOG.println(app::env.slice.current_avg_pulses_per_l);
  SLOG.print(" avg_calibration_count: ");
  SLOG.println(app::env.slice.avg_calibration_count);
  
  SLOG.print("last_output_volume_report: ");
  SLOG.print(app::env.report.last_output_volume_report);
  SLOG.print(" total_tank_output_volume: ");
  SLOG.print(app::env.report.total_tank_output_volume);
  SLOG.print(" calibration_id: ");
  SLOG.print(app::env.report.calibration_id);
  SLOG.print(" calibration_state: ");  
  SLOG.print(app::env.report.calibration_state);
  SLOG.print(" drain_start_pressure: ");
  SLOG.println(app::env.report.drain_start_pressure);
  SLOG.println("*****************");

}

void app::loop_app(){

  // Update timestamps
  unsigned long int current_time = millis();
  app::env.slice.time_elapsed = current_time - app::env.time.last_timestamp;
  app::env.time.last_timestamp = current_time;
  if (app::env.time.process_begin_timestamp > 0) {
    app::env.slice.total_time_elapsed = current_time - app::env.time.process_begin_timestamp;
  }

  loop_dispensation();

  // Try to reconnect to the MQTT broker if there isn't any process going on
  if (!app::env.flag.dispense_flag && !app::env.flag.drain_flag && !app::env.flag.mqtt_connected_flag) {
    net::reconnect_mqtt();
  }

  loop_flow_calibration_process();
  loop_drain();

  log_state();

}

void app::restart(){

  if (app::env.flag.dispense_flag){
    close_dispense_process();
    dispense_report_slice();
    dispense_report_summary();
  }

  if (app::env.flag.drain_flag){
    close_drain_process();
    drain_report_summary();
  }

  ESP.restart();
}


// Returns the volume of fluid in the tank given its pressure and geometry
float app::pressure_to_volume (float pressure) {
  // Meters = (hPa / 10) / (kg/m^3 * m/s^2) = (kPa) / (kg/s^2*m^2) = (kPa) / (kPa / m)
  float height = (pressure / 10) / DENSITY_GRAVITY;

  switch (app::env.tank_config.shape_type) {
    case 1: // Rectangular prism: length, width, height
      return app::env.tank_config.dimension_1 * app::env.tank_config.dimension_2 * height;
    case 2: // Cylinder: diameter, height, N/A
      return PI * (app::env.tank_config.dimension_1 / 2) * (app::env.tank_config.dimension_1 / 2) * height;
    default:
      return -1;
  }
}

// Returns the total volume of the tank given its geometry
float total_tank_volume () {

  switch (app::env.tank_config.shape_type) {
    case 1: // Rectangular prism: length, width, height
      return app::env.tank_config.dimension_1 * app::env.tank_config.dimension_2 * app::env.tank_config.dimension_3;
    case 2: // Cylinder: diameter, height, N/A
      return PI * (app::env.tank_config.dimension_1 / 2) * (app::env.tank_config.dimension_1 / 2) * app::env.tank_config.dimension_2;
    default:
      return -1;
  }
}