#include "I2C_Device.h"

/* =================================== PRIVATE ===================================== */

void I2C_Device::zeroes_array(int target_array[], int size_of_array) {
	for (int i = 0; i < size_of_array; i++) {
		target_array[i] = 0;
	}
}

void I2C_Device::zeroes_array(uint8_t target_array[], int size_of_array) {
	for (int i = 0; i < size_of_array; i++) {
		target_array[i] = 0;
	}
}

void I2C_Device::send_sync_request(int address) {
	this->send_request(address, I2C_Device::HEADER_LENGTH, I2C_Device::SYNC_MESSAGE);
}

/* ==================================== PUBLIC ===================================== */
/* -------- Setters/Getters --------- */
int I2C_Device::get_address() {
	return this->address;
}

int I2C_Device::get_size_of_recive_type() {
	return this->size_of_recive_type;
}

int I2C_Device::get_recive_data_length() {
	return this->recive_data_length;
}

bool I2C_Device::get_is_synchronized() {
	return this->is_synchronized;
}

void I2C_Device::set_address(int device_address) {
	this->address = device_address;
}

void I2C_Device::set_size_of_recive_type(int size) {
	this->size_of_recive_type = size;
}

void I2C_Device::set_recive_data_length(int length) {
	this->recive_data_length = length;
}

void I2C_Device::set_is_synchronized(bool synchronized) {
	this->is_synchronized = synchronized;
}

/* -------------- Rest -------------- */

void I2C_Device::send_sync_data_for_u_int_array(int array_size) {
	uint8_t ans[2] = { 0, 0 };

	ans[0] = sizeof(int);
	ans[1] = array_size;
	
	Wire.write(ans, I2C_Device::HEADER_LENGTH);
}

void I2C_Device::send_u_int_array(int array[], int array_length) {
	uint8_t* ans = (uint8_t*)array;

	Wire.write(ans, array_length * sizeof(int));
}

void I2C_Device::send_request(int address, int expected_length, int request) {
	Wire.beginTransmission(address);
	Wire.write(request);
	Wire.endTransmission();
	Wire.requestFrom(address, expected_length); 
}

void I2C_Device::send_request(int request) {
	Wire.beginTransmission(this->address);
	Wire.write(request);
	Wire.endTransmission();
}

// uint32_t I2C_Device::get_u_int32_from_response() {
// 	byte raw_data = Wire.read();
// 	uint32_t int_buff = (uint32_t)raw_data;
	
// 	return int_buff;
// }

uint8_t I2C_Device::get_u_int_from_response() {
	byte raw_data = Wire.read();
	uint8_t int_buff = (uint8_t)raw_data;
	
	return int_buff;
}

uint8_t* I2C_Device::request_for_u_int8_array (const int message) {
	Wire.beginTransmission(this->address);
	Wire.write(message);
	Wire.endTransmission();
		
	// Read response
	Wire.requestFrom(this->address, this->recive_data_length * this->size_of_recive_type);

	uint8_t *result_array = new uint8_t[this->recive_data_length];
	zeroes_array(result_array, this->recive_data_length);
	
	int actual_byte = 0;
	int actual_index = 0;
	
	while (Wire.available()) {
		uint8_t tmp = get_u_int_from_response();

		result_array[actual_index] += tmp << 8 * actual_byte;
		actual_byte++;

		if (actual_byte == this->size_of_recive_type) {
			actual_index++;
			actual_byte = 0;
		}
	}

	return result_array;
}

I2C_Device::I2C_Device (int device_address) {
	this->address             = device_address;
	this->size_of_recive_type = -1;
	this->recive_data_length  = -1;
	this->is_synchronized     = false;
}