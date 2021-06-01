#ifndef _I2C_IM_SLAVE_H_
#define _I2C_IM_SLAVE_H_

#include "I2C_Device.h"

class I2C_Im_Slave: public I2C_Device
{
private:
	static int device_counter;

	uint32_t request_from_master;
	bool     synchronizing;

	bool check_synchronizing ( uint32_t message ); // TODO: Rename this

public:
	uint32_t  get_request_from_master ( );
	void      set_request_from_master ( uint32_t request );

	bool is_synchronizing    ( );
	void receive_message     ( );

	static I2C_Im_Slave* begin_communication ( int address, void (*request)(void), void (*receive)(size_t) );

	void event_for_request ( );
	void event_for_receive ( int a );

	I2C_Im_Slave  (int device_address): I2C_Device(device_address) {}
	~I2C_Im_Slave ();
};

#endif //_I2C_IM_SLAVE_H_