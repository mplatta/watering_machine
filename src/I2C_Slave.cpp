#include "I2C_Slave.h"

int I2C_Slave::device_counter = 0;

I2C_Slave* I2C_Slave::begin_communication(int address) {
	if (I2C_Slave::device_counter == 0) {
		Wire.begin();
	}

	I2C_Slave::device_counter++;

	return new I2C_Slave(address);
}

bool I2C_Slave::synchronize_data_format_with_slave() {
	this->send_sync_request(this->get_address());

	this->set_size_of_recive_type( this->get_u_int_from_response() ); // read first byte
	this->set_recive_data_length ( this->get_u_int_from_response() ); // read second byte

	if ((this->get_size_of_recive_type() < 0) || (this->get_recive_data_length() < 0)) 
		return false;

	this->set_is_synchronized(true);

	return true;
}

I2C_Slave::~I2C_Slave() { }
