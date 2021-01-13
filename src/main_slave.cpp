#include "I2C_Device.h"

// Define Slave I2C Address
#define SLAVE_ADDR 9

// Define Slave answer size
#define ANSWER_SIZE 3
#define HEADER_LENGTH 2

int answer[3] = {256, 6, 7};
I2C_Device *me = NULL;

void send_sensors_data() {  
	uint8_t* ans = (uint8_t*)answer;

	// Send response back to Master
	Wire.write(ans, ANSWER_SIZE * sizeof(int));
  
	// Print to Serial Monitor
	for (unsigned int i = 0; i < ANSWER_SIZE * sizeof(int); i++) {
		Serial.print(String(ans[i]) + " "); 
	}

	Serial.println("");
}

void requestEvent() {
	if(me->is_synchronizing()) {
		me->send_sync_data_for_int_array(ANSWER_SIZE);
	} else {
		me->send_int_array(answer, ANSWER_SIZE);
	}
}

void receiveEvent(size_t a) {
	me->check_synchronizing();
}

void setup() {

	me = I2C_Device::begin_communication_for_slave(SLAVE_ADDR, requestEvent, receiveEvent);

	Serial.begin(9600);
	Serial.println("I2C Slave");
}

void loop() {

	// Time delay in loop
	delay(50);
}