typedef enum FsmStates = {
    STATE_MIN,
    STATE_BOOT,
    STATE_FATAL_ERROR,
    STATE_CONNECT,
    STATE_ACCESS_POINT,
    STATE_RESTART,
    STATE_LISTEN,
    STATE_SLEEP,
    STATE_CONFIG,
    STATE_DISPENSE_START,
    STATE_DISPENSE_SOURCE,
    STATE_DISPENSE_TANK,
    STATE_FLOW_CALIBRATE_DISPENSE,
    STATE_FLOW_CALIBRATE_MEASURE,
    STATE_PRESSURE_CALIBRATE,
    STATE_PRESSURE_POLL,
    STATE_DRAIN,
    STATE_MAX
};

class StateMachine {
    FsmStates state = STATE_MIN;

    void start() {}

    void loop() {
        switch (state) {
            case STATE_BOOT:
                boot();
                break;
            case STATE_FATAL_ERROR:
                fatalError();
                break;
            case STATE_CONNECT:
                connect();
                break;
            case STATE_ACCESS_POINT:
                accessPoint();
                break;
            case STATE_RESTART:
                restart();
                break;
            case STATE_LISTEN:
                listen();
                break;
            case STATE_SLEEP:
                sleep();
                break;
            case STATE_CONFIG:
                config();
                break;
            case STATE_DISPENSE_START:
                dispense_start();
                break;
            case STATE_DISPENSE_SOURCE:
                dispense_source();
                break;
            case STATE_DISPENSE_TANK:
                dispense_tank();
                break;
            case STATE_FLOW_CALIBRATE_DISPENSE:
                flow_calibrate_dispense();
                break;
            case STATE_FLOW_CALIBRATE_MEASURE:
                flow_calibrate_measure();
                break;
            case STATE_PRESSURE_CALIBRATE:
                pressure_calibrate();
                break;
            case STATE_PRESSURE_POLL:
                pressure_poll();
                break;
            case STATE_DRAIN:
                drain();
                break;
            default:
                break;
        }
    }


    void boot() {
        // Placeholder for boot state logic
    }

    void fatalError() {
        // Placeholder for fatal error state logic
    }

    void connect() {
        // Placeholder for connect state logic
    }

    void accessPoint() {
        // Placeholder for access point state logic
    }

    void restart() {
        // Placeholder for restart state logic
    }

    void listen() {
        // Placeholder for listen state logic
    }

    void sleep() {
        // Placeholder for sleep state logic
    }

    void config() {
        // Placeholder for config state logic
    }

    void dispense_start() {
        // Placeholder for dispense start state logic
    }

    void dispense_source() {
        // Placeholder for dispense source state logic
    }

    void dispense_tank() {
        // Placeholder for dispense tank state logic
    }

    void flow_calibrate_dispense() {
        // Placeholder for flow calibrate dispense state logic
    }

    void flow_calibrate_measure() {
        // Placeholder for flow calibrate measure state logic
    }

    void pressure_calibrate() {
        // Placeholder for pressure calibrate state logic
    }

    void pressure_poll() {
        // Placeholder for pressure poll state logic
    }

    void drain() {
        // Placeholder for drain state logic
    }
}