#define DEBUG

#include "I2C_Im_Slave.h"
#include "formatted_log.h"
#include "timer.h"
// #include <ArduinoLowPower.h>


// Define Slave I2C Address
#define SLAVE_ADDR 9

// Define Slave answer size
#define HEADER_INFO_LENGTH      1  // 1 byte
#define NUMBER_OF_RESERVED_PINS 2  // two pins needed for i2c
#define MAX_ANALOG_PINS         8
#define MAX_NUMBER_OF_SENSORS MAX_ANALOG_PINS - NUMBER_OF_RESERVED_PINS
#define ANSWER_ARRAY_SIZE HEADER_INFO_LENGTH + MAX_NUMBER_OF_SENSORS

// Define kind of messages from master
#define MESSAGE_MASK           0b11000000
#define SEND_DATA_FROM_SENSORS 0b01000000
#define ACTIVE_PUMP            0b10000000

#define MAX_VALUE_FOR_INACTIVE_SENSOR 10

#define MAIN_LOOP_DELAY 500 // 50ms

const int RESERVED_PINS[NUMBER_OF_RESERVED_PINS] = { 4, 5 };

// Define wet/dry values
const int WET_VALUE = 261;
const int DRY_VALUE = 650;

/* ---------------------- GLOBAL VARIABLES ------------------- */
timer    g_main_loop_timer;

int     *g_answer        ;
int     *g_data_array    ;
int      g_answer_size   ;
int      g_active_sensors;

I2C_Im_Slave *me = NULL;

/* ----------------------- PROTOTYPES ------------------------ */
void receiveEvent         ( size_t a    );
void requestEvent         ( );
void reaction_for_request ( int request );

int  change_to_percentages                     ( int value );
int *get_data_from_sensors_and_check_if_active ( );
int  is_sensor_active                          ( int sensor_pin );

bool pin_is_in_reserved_pins ( int pin );

void active_pump ( int pump_id, int time );

/* ------------------------- MAIN ---------------------------- */
void setup() {
	g_main_loop_timer = new_timer();

	g_answer         = (int*)calloc(ANSWER_ARRAY_SIZE, sizeof(int));
	g_data_array     = (int*)calloc(MAX_NUMBER_OF_SENSORS, sizeof(int));
	g_answer_size    = ANSWER_ARRAY_SIZE;
	g_active_sensors = 0b00000000;

	me = I2C_Im_Slave::begin_communication(SLAVE_ADDR, requestEvent, receiveEvent);

	get_data_from_sensors_and_check_if_active();

	begin_log(9600);
	formatted_info("I2C Slave");
}

void loop() {
	start_timer(&g_main_loop_timer);
	if (check_if_time_elapsed(&g_main_loop_timer, MAIN_LOOP_DELAY)) {
		g_data_array = get_data_from_sensors_and_check_if_active();
		g_answer[0]  = g_active_sensors;

		for (int i = 0; i < ANSWER_ARRAY_SIZE; i++) {
			g_answer[i + HEADER_INFO_LENGTH] = change_to_percentages(g_data_array[i]);
		}

		// ONLY FOR TEST
		for (int i = 0; i < ANSWER_ARRAY_SIZE; i++) {
			formatted_log(String(i) + ":" + String(g_answer[i]) + " ");
		}
		formatted_log("");

		free(g_data_array);
	}
}

/* ---------------------- DEFINITIONS ------------------------ */
void receiveEvent(size_t a) {
	me->receive_message();
}

void requestEvent() {
	if(me->is_synchronizing()) {
		me->send_sync_data_for_u_int_array(g_answer_size);
	} else {
		reaction_for_request(me->get_request_from_master());
	}
}

void reaction_for_request(int request){
	int message_from_master = request & ~MESSAGE_MASK;
	int kind_of_message = request & MESSAGE_MASK;

	switch (kind_of_message)
	{
	case SEND_DATA_FROM_SENSORS:
		me->send_u_int_array(g_answer, g_answer_size);
		break;
	case ACTIVE_PUMP:
		me->send_u_int_array(g_answer, g_answer_size);
		active_pump(message_from_master >> 4, message_from_master & 0b00001111);
		break;
	default:
		formatted_info("Not supported");
		break;
	}
}

int change_to_percentages (int value) {
	if      (value < WET_VALUE) return 100;
	else if (value > DRY_VALUE) return 0;
	else                        return map(value, WET_VALUE, DRY_VALUE, 100, 0);
}

int *get_data_from_sensors_and_check_if_active () {
	int actual_answer_index = 0;
	int *result = (int*)calloc(MAX_NUMBER_OF_SENSORS, sizeof(int));

	g_active_sensors = 0;

	for (int i = 0; i < MAX_ANALOG_PINS; i++) {
		int data = 0;
		
		if (!pin_is_in_reserved_pins(i)) {
			if (data = is_sensor_active(i)) {
				g_active_sensors |= (1 << i);
				result[actual_answer_index] = data;
				actual_answer_index++;
			}
		}
	}

	return result;
}

int is_sensor_active (int sensor_pin) {
	int data = analogRead(sensor_pin);

	if (data <= MAX_VALUE_FOR_INACTIVE_SENSOR) return 0;
	else return data;
}

bool pin_is_in_reserved_pins(int pin) {
	bool result = false;

	for (int i = 0; i < NUMBER_OF_RESERVED_PINS; i++) {
		if (RESERVED_PINS[i] == pin) {
			result = true;
			break;
		}
	}

	return result;
}

void active_pump ( int pump_id, int time ) {
	formatted_log("pump_id...." + String(pump_id));
	digitalWrite(7, HIGH);
	formatted_log("Time...." + String(time));
	// delay(15000);
	digitalWrite(7, LOW);
}