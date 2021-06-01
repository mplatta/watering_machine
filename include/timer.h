#ifndef _TIMER_H_
#define _TIMER_H_

#include <Arduino.h>
#include <limits.h>

#include "formatted_log.h"

typedef enum timer_states {
    TIMER_IDLE,
    TIMER_ACTIVE,
    TIMER_PAUSED,
	TIMER_UNPAUSED,
	TIMER_STOPPED
} timer_state;

typedef struct timer
{	
	timer_state state;

	unsigned long start_time;
	unsigned long pause_time;
	unsigned long pause_duration;
} timer;

/* ---------------------------- PROTOTYPES ----------------------------- */
timer new_timer     ( );

void  start_timer   ( timer *t );
void  pause_timer   ( timer *t );
void  stop_timer    ( timer *t );
void  reset_timer   ( timer *t );

bool check_if_time_elapsed ( timer *t, unsigned long ms );

static unsigned long time_duration_to_now ( unsigned long a );

/* --------------------------- DEFINITIONS ----------------------------- */
timer new_timer ( ) {
	timer t;

	t.state          = TIMER_IDLE;
	t.start_time     = 0 ;
	t.pause_time     = 0 ;
	t.pause_duration = 0 ;

	return t;
};

void start_timer ( timer *t ) {
	switch (t->state)
	{
	case TIMER_IDLE:
	case TIMER_STOPPED:
		t->start_time     = millis();
		t->state          = TIMER_ACTIVE;
		t->pause_time     = 0;
		t->pause_duration = 0;
		break;
	case TIMER_PAUSED:
		t->pause_duration = time_duration_to_now(t->pause_time);
		t->state          = TIMER_ACTIVE;
		break;
	case TIMER_ACTIVE:
	case TIMER_UNPAUSED:
		break;
	default:
		// TODO
		break;
	}
}

void pause_timer ( timer *t ) {
	switch (t->state)
	{
	case TIMER_IDLE:
	case TIMER_PAUSED:
	case TIMER_STOPPED:
		break;
	case TIMER_ACTIVE:
		t->state          =  TIMER_PAUSED;
		t->pause_time     =  millis();
		t->pause_duration += time_duration_to_now(t->start_time);
		break;
	case TIMER_UNPAUSED:
		t->state          =  TIMER_PAUSED;
		t->pause_duration += time_duration_to_now(t->pause_time);
		t->pause_time     =  millis();
		break;
	default:
		// TODO
		break;
	}
}

void stop_timer ( timer *t ) {
	switch (t->state)
	{
	case TIMER_IDLE:
	case TIMER_STOPPED:
		break;
	case TIMER_ACTIVE:
	case TIMER_PAUSED:
	case TIMER_UNPAUSED:
		t->state          = TIMER_STOPPED;
		t->start_time     = 0;
		t->pause_time     = 0;
		t->pause_duration = 0;
		break;
	default:
		// TODO
		break;
	}
}

void reset_timer ( timer *t ) {
	switch (t->state)
	{
	case TIMER_IDLE:
		break;
	case TIMER_ACTIVE:
	case TIMER_PAUSED:
	case TIMER_UNPAUSED:
	case TIMER_STOPPED:
		t->state          = TIMER_IDLE;
		t->start_time     = 0;
		t->pause_time     = 0;
		t->pause_duration = 0;
		break;
	default:
		// TODO
		break;
	}
}

bool check_if_time_elapsed( timer *t, unsigned long ms ) {
	bool result = false;
	unsigned long duration_time = 0; // only for log

	switch (t->state)
	{
	case TIMER_ACTIVE:
		duration_time = time_duration_to_now(t->start_time); // only for log
		result = ms <= duration_time;
		break;
	case TIMER_UNPAUSED:
		duration_time = (time_duration_to_now(t->pause_time) + t->pause_duration);
		result = ms <= duration_time;
	case TIMER_IDLE:
	case TIMER_PAUSED:
	case TIMER_STOPPED:
		break;
		// TODO
	default:
		break;
	}

	if (result) {
		reset_timer(t);
		start_timer(t);

		begin_log(9600);
		formatted_log("Duration time:" + String(duration_time));
	}

	return result;
}

static unsigned long time_duration_to_now ( unsigned long a ) {
	unsigned long result   = 0;
	unsigned long now      = millis();
	unsigned long interval = max(a, now) - min(a, now);

	if (a < now) result = interval;
	else {
		unsigned long max_time    = ULONG_MAX;
		unsigned long time_to_max = max_time - a;

		result = time_to_max + interval;
	}

	return result;
}

#endif