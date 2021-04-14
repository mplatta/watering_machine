#include "I2C_Im_Slave.h"

/* -------------------------- PRIVATE --------------------------------*/
int I2C_Im_Slave::device_counter = 0;

bool I2C_Im_Slave::check_synchronizing(int message) {
	this->synchronizing = (message == I2C_Device::SYNC_MESSAGE);
	
	return this->synchronizing;
}

/* -------------------------- PUBLIC --------------------------------*/
int I2C_Im_Slave::get_request_from_master() {
	return this->request_from_master;
}

void I2C_Im_Slave::set_request_from_master(int request) {
	this->request_from_master = request;
}

bool I2C_Im_Slave::is_synchronizing() {
	return this->synchronizing;
}

void I2C_Im_Slave::receive_message() {
	while (0 < Wire.available()) {
		int x = this->get_int_from_response();

		this->set_request_from_master(x);
		this->check_synchronizing(x);
	}
}

I2C_Im_Slave* I2C_Im_Slave::begin_communication(int address, void (*request)(void), void (*receive)(size_t)) {
	if (I2C_Im_Slave::device_counter == 0) {
		Wire.begin(address);
	}

	Wire.onRequest(request);
	Wire.onReceive(receive);

	I2C_Im_Slave::device_counter++;

	return new I2C_Im_Slave(address);
}

I2C_Im_Slave::~I2C_Im_Slave() {}