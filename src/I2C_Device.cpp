#include "I2C_Device.h"

/* =================================== PRIVATE ===================================== */

int I2C_Device::device_counter = 0;

void I2C_Device::zeroes_array(int target_array[], int size_of_array) {
    for (int i = 0; i < size_of_array; i++) {
        target_array[i] = 0;
    }
}

void I2C_Device::send_sync_request(int address) {
    Wire.beginTransmission(address);
	Wire.write(I2C_Device::SYNC_MESSAGE);
	Wire.endTransmission();
    Wire.requestFrom(address, I2C_Device::HEADER_LENGTH);
}

/* ==================================== PUBLIC ===================================== */
/* -------- Setters/Getters --------- */
int I2C_Device::get_size_of_recive_type() {
    return this->size_of_recive_type;
}

int I2C_Device::get_recive_data_length() {
    return this->recive_data_length;
}

bool I2C_Device::get_is_synchronized() {
    return this->is_synchronized;
}

bool I2C_Device::is_synchronizing() {
    return this->synchronizing;
}

/* -------------- Rest -------------- */
I2C_Device* I2C_Device::begin_communication(int address) {
    if (I2C_Device::device_counter == 0) {
        Wire.begin(address);
    }

    I2C_Device::device_counter++;

    return new I2C_Device(address);
}

I2C_Device* I2C_Device::begin_communication_for_slave(int address, void (*request)(void), void (*receive)(size_t)) {
    if (I2C_Device::device_counter == 0) {
        Wire.begin(address);
    }

    Wire.onRequest(request);
    Wire.onReceive(receive);

    I2C_Device::device_counter++;

    return new I2C_Device(address);
}

bool I2C_Device::synchronize_data_format_with_slave() {
    this->send_sync_request(this->address);

    this->size_of_recive_type = get_int_from_response(); // read first byte
    this->recive_data_length  = get_int_from_response(); // read second byte

    if ((this->size_of_recive_type < 0) || (this->recive_data_length < 0)) 
        return false;

    this->is_synchronized = true;

    return true;
}

bool I2C_Device::check_synchronizing() {
    while (0 < Wire.available()) {
	    int x = get_int_from_response();

        this->synchronizing = (x == I2C_Device::SYNC_MESSAGE);
    }
}

// void I2C_Device::synchronize_if_necessary() {
//     if (this->is_synchronizing()) {

//     }
// }

int I2C_Device::get_int_from_response() {
    byte raw_data = Wire.read();
    int int_buff = (int)raw_data;
    
    return int_buff;
}

int* I2C_Device::get_int_array_from_response () {
    Wire.beginTransmission(this->address);
    Wire.write(I2C_Device::SEND_ME_DATA_MESSAGE);
    Wire.endTransmission();
        
    // Read response
    Wire.requestFrom(this->address, this->recive_data_length * this->size_of_recive_type);

    int *result_array = new int[this->recive_data_length];
    zeroes_array(result_array, this->recive_data_length);
    
    int actual_byte = 0;
    int actual_index = 0;
    
    while (Wire.available()) {
        int tmp = get_int_from_response();

        result_array[actual_index] += tmp << 8 * actual_byte;
        actual_byte++;

        if (actual_byte == this->size_of_recive_type) {
            actual_index++;
            actual_byte = 0;
        }
    }

    return result_array;
}

void I2C_Device::send_sync_data_for_int_array(int array_size) {
	uint8_t ans[2] = { 0, 0 };

	ans[0] = sizeof(int);
	ans[1] = array_size;
	
	Wire.write(ans, I2C_Device::HEADER_LENGTH);
}

void I2C_Device::send_int_array(int array[], int array_length) {
    uint8_t* ans = (uint8_t*)array;

    Wire.write(ans, array_length * sizeof(int));
}

I2C_Device::I2C_Device (int device_address) {
    this->address             = device_address;
    this->size_of_recive_type = -1;
    this->recive_data_length  = -1;
    this->is_synchronized     = false;
}

// I2C_Device::I2C_Device (int device_address, void (*request)(void), void (*receive)(size_t)) {
//     this->address             = device_address;
//     this->size_of_recive_type = -1;
//     this->recive_data_length  = -1;
//     this->request_event       = request;
//     this->receive_event       = receive;
//     this->is_synchronized     = false;
// }