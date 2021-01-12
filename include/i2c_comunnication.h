#ifndef _I2C_COMMUNICATION_H_
#define _I2C_COMMUNICATION_H_

#ifndef MASTER
#define MASTER 1
#endif // MASTER 1

#include <Wire.h>
#include <Arduino.h>

#define HEADER_LENGTH        2
#define SYNC_MESSAGE         0
#define SEND_ME_DATA_MESSAGE 1

int DEVICE_COUNTER = 0;

typedef struct slave_device
{
	int address;
	int size_of_recive_type = -1;
	int recive_data_length = -1;
} Slave_device;

void (*receive_func);
void (*request_func);

/* ------------------------------- FUNCTION DEFINITIONS --------------------------------------------- */

// Function for slave and master device
Slave_device create_device_communication  ( int _slave_address );
// void         create_device_communication  ( int _slave_address, void (*_receive_func), void (*_request_func));
void         zeroes_array                 ( int target_array[], int size_of_array );

#if MASTER == 1 // funtions for MASTER device

bool         synchronized_with_slave      (Slave_device& slave);
bool         get_int_array_from_device    (Slave_device& slave, int destination_result[]);
int          read_int_from_i2c            ( );

#endif // if MASTER == 1

//funtions for SLAVE device
void         send_sync_data_for_int_array ( int array_size );
// void         receive_event                ( int how_many );
// void         request_event                ( );

/* -------------------------------------------------------------------------------------------------- */

Slave_device create_device_communication (int _slave_address) {
	Slave_device slave;
	slave.address = _slave_address;

	if (MASTER == 1) {
		if (DEVICE_COUNTER < 1) Wire.begin();

		DEVICE_COUNTER++;
	} else {
		Wire.begin(_slave_address);
		// Wire.onRequest(request_event);
		// Wire.onReceive(receive_event);
	}

	return slave;
}

// void create_device_communication (int _slave_address, void (*_receive_func), void (*_request_func)) {
// 	Wire.begin(_slave_address);
// 	Wire.onRequest(request_event);
// 	Wire.onReceive(receive_event);
// 	receive_func = _receive_func;
// 	request_func = _request_func;
// }


void zeroes_array(int target_array[], int size_of_array) {
    for (int i = 0; i < size_of_array; i++) {
        target_array[i] = 0;
    }
}

#if MASTER == 1 // functions for MASTER device

bool synchronized_with_slave (Slave_device& slave) {
	Wire.beginTransmission(slave.address);
	Wire.write(SYNC_MESSAGE);
	Wire.endTransmission();

	Wire.requestFrom(slave.address, HEADER_LENGTH);

	int tmp = read_int_from_i2c();
	slave.size_of_recive_type = tmp;

	tmp = read_int_from_i2c();
	slave.recive_data_length = tmp;

	if ((slave.size_of_recive_type < 0) || (slave.recive_data_length < 0)) return false;
	
	return true;
}

bool get_int_array_from_device (Slave_device& slave, int destination_result[]) {
    Wire.beginTransmission(slave.address);
    Wire.write(SEND_ME_DATA_MESSAGE);
    Wire.endTransmission();
        
    // Read response from Slave
    Wire.requestFrom(slave.address, slave.recive_data_length * slave.size_of_recive_type);

    zeroes_array(destination_result, slave.recive_data_length);
    
    int actual_byte = 0;
    int actual_index = 0;
    
    while (Wire.available()) {
        int tmp = read_int_from_i2c();

        destination_result[actual_index] += tmp << 8 * actual_byte;

        actual_byte++;
        if (actual_byte == slave.size_of_recive_type) {
            actual_index++;
            actual_byte = 0;
        }
    }

    return true;
}

int read_int_from_i2c() {
    byte raw_data = Wire.read();
    int int_buff = (int)raw_data;
    
    return int_buff;
}

#endif // if MASTER == 1

//funtions for SLAVE device
void send_sync_data_for_int_array (int array_size) {
	uint8_t ans[2] = { 0, 0 };

	ans[0] = sizeof(int);
	ans[1] = array_size;
	
	Wire.write(ans, HEADER_LENGTH);
}

// void receive_event ( int how_many ) {
// }

// void request_event ( ) {
// 	if (synchronizing) {
// 		send_sync_data();
// 	} else {
// 		send_sensors_data();
// 	}
// }


#endif // ifndef _I2C_COMMUNICATION_H_