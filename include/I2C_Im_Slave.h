#ifndef _I2C_IM_SLAVE_H_
#define _I2C_IM_SLAVE_H_

#include "I2C_Device.h"

class I2C_Im_Slave: public I2C_Device
{
private:
    static int device_counter;

    bool synchronizing;
public:
    bool is_synchronizing    ( );
    bool check_synchronizing ( ); // TODO: Rename this

    static I2C_Im_Slave* begin_communication ( int address, void (*request)(void), void (*receive)(size_t) );

    void event_for_request ( );
    void event_for_receive ( int a );

    I2C_Im_Slave  (int device_address): I2C_Device(device_address) {}
    ~I2C_Im_Slave ();
};

#endif //_I2C_IM_SLAVE_H_