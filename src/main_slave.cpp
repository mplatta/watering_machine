#include "I2C_Im_Slave.h"
// #include <ArduinoLowPower.h>

// Define Slave I2C Address
#define SLAVE_ADDR 9

// Define Slave answer size
#define ANSWER_SIZE   3
#define HEADER_LENGTH 2

// Define kind of messages from master
#define MESSAGE_MASK           0b11000000
#define SEND_DATA_FROM_SENSORS 0
#define REGISTER_NEW_SENSOR    64

// Define wet/dry values
const int WET_VALUE = 261;
const int DRY_VALUE = 650;

// Sensors PINs
const int SENSOR_0_PIN = 19; // A0
const int SENSOR_1_PIN = 20; // A1
const int SENSOR_2_PIN = 21; // A2

/* ---------------------- GLOBAL VARIABLES ------------------- */
int answer[3] = {0, 0, 0};
I2C_Im_Slave *me = NULL;

/* ----------------------- PROTOTYPES ------------------------ */
void receiveEvent         ( size_t a    );
void requestEvent         ( );
void reaction_for_request ( int request );

int  read_humidity_in_percentages (int sensor_pin);

/* ------------------------- MAIN ---------------------------- */
void setup() {
	me = I2C_Im_Slave::begin_communication(SLAVE_ADDR, requestEvent, receiveEvent);

	Serial.begin(9600);
	Serial.println("I2C Slave");
}

void loop() {
	int sensor_0_percent = read_humidity_in_percentages(SENSOR_0_PIN);
	int sensor_1_percent = read_humidity_in_percentages(SENSOR_1_PIN);
	int sensor_2_percent = read_humidity_in_percentages(SENSOR_2_PIN);

	answer[0] = sensor_0_percent;
	answer[1] = sensor_1_percent;
	answer[2] = sensor_2_percent;

	delay(50);
}

/* ---------------------- DEFINITIONS ------------------------ */
void receiveEvent(size_t a) {
	me->receive_message();
}

void requestEvent() {
	if(me->is_synchronizing()) {
		me->send_sync_data_for_int_array(ANSWER_SIZE);
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
		me->send_int_array(answer, ANSWER_SIZE);
		break;
	case REGISTER_NEW_SENSOR:
		Serial.println("TODO");
		break;
	default:
		Serial.println("Not supported");
		break;
	}
}

int read_humidity_in_percentages(int sensor_pin) {
	int raw_value = analogRead(sensor_pin);
	int percent = map(raw_value, WET_VALUE, DRY_VALUE, 100, 0);

	return percent;
}