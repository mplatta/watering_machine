#ifndef _PUMPS_H_
#define _PUMPS_H_

#include <Arduino.h>

#include "timer.h"

typedef enum pumps_state {
	PUMP_IDLE,
	PUMP_ACTIVE,
	PUMP_STOPPED
} pumps_state;

typedef struct pumps
{
	int pin;
	unsigned long timeout_time;
	pumps_state state;
	timer pump_timer;
} pumps;

/* ---------------------------- PROTOTYPES ----------------------------- */
pumps new_pump ( int pin );

void start_pump            ( pumps *pump, unsigned long ms);
void stop_pump             ( pumps *pump );
void reset_pump            ( pumps *pump );

void stop_pump_after_timeout ( pumps *pump, unsigned long ms );

/* --------------------------- DEFINITIONS ----------------------------- */
pumps new_pump (int pin) {
	pumps p;

	p.pin = pin;
	p.state = PUMP_IDLE;
	p.timeout_time = 0;
	p.pump_timer = new_timer();

	digitalWrite(pin, LOW);

	return p;
}

void start_pump (pumps *pump, unsigned long ms) {
	switch (pump->state)
	{
	case PUMP_IDLE:
	case PUMP_STOPPED:
		pump->state = PUMP_ACTIVE;
		pump->timeout_time = ms;
		start_timer(&(pump->pump_timer));
		digitalWrite(pump->pin, HIGH);
		break;
	case PUMP_ACTIVE:
		break;
	default:
		// TODO: ...
		break;
	}
}

void stop_pump (pumps *pump) {
	switch (pump->state)
	{
	case PUMP_IDLE:
	case PUMP_STOPPED:
		break;
	case PUMP_ACTIVE:
		pump->state = PUMP_STOPPED;
		stop_timer(&(pump->pump_timer));
		digitalWrite(pump->pin, LOW);
		break;
	default:
		// TODO: ...
		break;
	}
}

void reset_pump (pumps *pump) {
	switch (pump->state)
	{
	case PUMP_IDLE:
		break;
	case PUMP_STOPPED:
	case PUMP_ACTIVE:
		pump->state = PUMP_IDLE;
		pump->timeout_time = 0;
		reset_timer(&(pump->pump_timer));
		digitalWrite(pump->pin, LOW);
		break;
	default:
		// TODO: ...
		break;
	}
}

void stop_pump_after_timeout ( pumps *pump ) {
	if (pump->state == PUMP_ACTIVE) {
		if (wait(&(pump->pump_timer), pump->timeout_time)) {
			stop_pump(pump);
		}
	}
}

#endif