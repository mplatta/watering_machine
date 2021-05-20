#ifndef _SENSORS_H_
#define _SENSORS_H_

#include <Arduino.h>

typedef struct sensor 
{
	int pin;
	int result;
} sensor;

typedef struct sensors
{
	sensor *sensor_list;
	
	bool new_sensor_added = false;
	int  number_of_sensors       ;
	int  active_sensors          ;
} sensors;

/* ---------------------------- PROTOTYPES ----------------------------- */
bool refresh_sensors_list ( int active_sensors, sensors *s );
int  count_ones           ( int n );


/* --------------------------- DEFINITIONS ----------------------------- */
bool refresh_sensors_list(int active_sensors, sensors *s) {
	int new_length = count_ones(active_sensors);
	sensor *new_sensor_list = (sensor*)calloc(new_length, sizeof(sensor));
	
	int current_index = 0;
	int j = 0;
	for (int bit = 1; bit <= active_sensors; bit <<= 1, j++) {
		if (bit & active_sensors) {
			new_sensor_list[current_index].pin = j;
			new_sensor_list[current_index].result = 0;

			current_index++;
		}
	}

	free(s->sensor_list);

	s->number_of_sensors = new_length;
	s->active_sensors = active_sensors;
	s->sensor_list = new_sensor_list;

	return true;
}

int count_ones(int n) {
	int c = 0;
	while (n != 0)
	{
		n = n & (n - 1);
		c++;
	}
	
	return c;
}

#endif