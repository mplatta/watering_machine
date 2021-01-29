#include "I2C_Im_Slave.h"

int I2C_Im_Slave::device_counter = 0;

bool I2C_Im_Slave::is_synchronizing() {
    return this->synchronizing;
}

bool I2C_Im_Slave::check_synchronizing() {
    while (0 < Wire.available()) {
	    int x = this->get_int_from_response();

        this->synchronizing = (x == I2C_Device::SYNC_MESSAGE);
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