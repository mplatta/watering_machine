#ifndef _I2C_DEVICE_H_
#define _I2C_DEVICE_H_

#include <Wire.h>
#include <Arduino.h>

class I2C_Device {
private:
	int address;
	int size_of_recive_type;
	int recive_data_length;
	
	bool is_synchronized;

protected:
	void zeroes_array(int target_array[], int size_of_array);
	void send_sync_request ( int address );

public:
	static const int HEADER_LENGTH        = 2;
	static const int SYNC_MESSAGE         = 0;
	static const int SEND_ME_DATA_MESSAGE = 1;

	int  get_address             ();
	int  get_size_of_recive_type ();
	int  get_recive_data_length  ();
	bool get_is_synchronized     ();

	void set_address             (int  device_address );
	void set_size_of_recive_type (int  size           );
	void set_recive_data_length  (int  length         );
	void set_is_synchronized     (bool synchronized   );

	void send_request                 ( int address, int length, int request );
	void send_sync_data_for_int_array ( int array_size );
	void send_int_array               ( int array[], int array_length );

	int  get_int_from_response       ();
	int* get_int_array_from_response ();

	I2C_Device  () {};
	I2C_Device  ( int address );
  
	~I2C_Device () {}
};

#endif // _I2C_DEVICE_H_
