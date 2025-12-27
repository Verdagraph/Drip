# Firmware Design

The firmware is currently being defined in header files, with most implementation code still blank.

The firmware uses a design with the following properties:

- It is centered around a finite state machine manager which runs in a main application task, with other tasks being used for background tasks such as filling queues of incoming data.
- Application logic is contained in Manager classes, with managers having only one-way interdepency between them.
- Manager classes may initialize Driver classes to further abstract low-level driver code.

The application code is contained in the `components` folder with the following components:

- `adc` contains drivers for the ADC.
- `config` contains the configuration data structures and persistence mechanism.
- `connection` is responsible for establishing a WiFi and MQTT connection.
- `errors` is for defining app errors.
- `flow` is responsible for reading data from the flow meter and executing the calibration process.
- `fsm` contains the state manager and all main application routine logic.
- `gpio` contains handles to the GPIO pins.
- `mqtt` is responsible for receiving and transmitting MQTT messages.
- `pressure` is responsible for reading data from the pressure sensor and executing the calibration process.
- `valves` is responsible for managing the dispense and drain processes.