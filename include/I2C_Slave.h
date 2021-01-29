#ifndef _I2C_SLAVE_H_
#define _I2C_SLAVE_H_

#include "I2C_Device.h"

class I2C_Slave: public I2C_Device
{
private:
    static int device_counter;

public:
    static I2C_Slave* begin_communication ( int address );

    bool synchronize_data_format_with_slave ( );

    I2C_Slave(int device_address): I2C_Device(device_address) {}
    ~I2C_Slave();
};

#endif //_I2C_SLAVE_H_