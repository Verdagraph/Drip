#ifndef SERVICES_H
#define SERVICES_H

// Activate the device to dispense water
void dispense_activate(byte* payload, unsigned int len);

// Deactivate all current dispense or drain processes
void deactivate();

// Update config file
void config_change(byte* payload, unsigned int len);

// Activate the device to drain water from the exhaustible resevoir
void drain_activate(byte* payload, unsigned int len);

#endif
