#include "I2C_Im_Slave.h"
// #include <ArduinoLowPower.h>

// Define Slave I2C Address
#define SLAVE_ADDR 9

// Define Slave answer size
#define HEADER_INFO_LENGTH 1
#define MAX_NUMBER_OF_SENSORS 8
#define ANSWER_ARRAY_SIZE HEADER_INFO_LENGTH + MAX_NUMBER_OF_SENSORS

// Define kind of messages from master
#define MESSAGE_MASK           0b11000000
#define SEND_DATA_FROM_SENSORS 0

#define MAX_VALUE_FOR_INACTIVE_SENSOR 10

// Define wet/dry values
const int WET_VALUE = 261;
const int DRY_VALUE = 650;


/* ---------------------- GLOBAL VARIABLES ------------------- */
int *g_data_array    = (int*)calloc(MAX_NUMBER_OF_SENSORS, sizeof(int));
int *g_answer        = (int*)calloc(ANSWER_ARRAY_SIZE, sizeof(int));
int g_answer_size    = 0;
int g_active_sensors = 0b00000000;

I2C_Im_Slave *me = NULL;

/* ----------------------- PROTOTYPES ------------------------ */
void receiveEvent         ( size_t a    );
void requestEvent         ( );
void reaction_for_request ( int request );

int  change_to_percentages                     ( int value );
int *get_data_from_sensors_and_check_if_active ( );
int  is_sensor_active                          ( int sensor_pin );

/* ------------------------- MAIN ---------------------------- */
void setup() {
	me = I2C_Im_Slave::begin_communication(SLAVE_ADDR, requestEvent, receiveEvent);

	get_data_from_sensors_and_check_if_active();

	Serial.begin(9600);
	Serial.println("I2C Slave");
}

void loop() {
	g_data_array = get_data_from_sensors_and_check_if_active();
	g_answer[0]  = g_active_sensors;

	for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++) {
		if (!g_data_array[i]) break;
		Serial.print(String(g_data_array[i]) + "-");
		g_answer[i + HEADER_INFO_LENGTH] = change_to_percentages(g_data_array[i]);
	}
	Serial.println("**\n");

	// ONLY FOR TEST
	for (int i = 0; i < ANSWER_ARRAY_SIZE; i++) {
		Serial.print(String(g_answer[i]) + " ");
	}
	Serial.println("");

	free(g_data_array);
	delay(50);
}

/* ---------------------- DEFINITIONS ------------------------ */
void receiveEvent(size_t a) {
	me->receive_message();
}

void requestEvent() {
	if(me->is_synchronizing()) {
		me->send_sync_data_for_int_array(g_answer_size);
	} else {
		reaction_for_request(me->get_request_from_master());
	}
}

void reaction_for_request(int request){
	int message_from_master = request ^ MESSAGE_MASK;
	int kind_of_message = request & MESSAGE_MASK;

	switch (kind_of_message)
	{
	case SEND_DATA_FROM_SENSORS:
		me->send_int_array(g_answer, g_answer_size);
		break;
	default:
		Serial.println("Not supported");
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

	g_answer_size = HEADER_INFO_LENGTH;
	g_active_sensors = 0;

	for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++) {
		int data = 0;
		
		if (data = is_sensor_active(i)) {
			g_answer_size++;
			g_active_sensors |= (1 << i);
			result[actual_answer_index] = data;
			actual_answer_index++;
		}
	}

	return result;
}

int is_sensor_active (int sensor_pin) {
	int data = analogRead(sensor_pin);

	if (data <= MAX_VALUE_FOR_INACTIVE_SENSOR) return 0;
	else return data;
}