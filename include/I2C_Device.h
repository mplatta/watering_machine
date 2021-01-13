#ifndef _I2C_DEVICE_H_
#define _I2C_DEVICE_H_

#include <Wire.h>
#include <Arduino.h>

class I2C_Device {
private:
    static int device_counter;

    int address;
    int size_of_recive_type;
    int recive_data_length;

    void (*request_event);
    void (*receive_event);

    bool is_synchronized;
    bool synchronizing;

    void zeroes_array(int target_array[], int size_of_array);

    void send_sync_request ( int address );

public:
    static const int HEADER_LENGTH        = 2;
    static const int SYNC_MESSAGE         = 0;
    static const int SEND_ME_DATA_MESSAGE = 1;

    int  get_size_of_recive_type ();
    int  get_recive_data_length  ();
    bool get_is_synchronized     ();
    bool is_synchronizing        ();

    static I2C_Device* begin_communication ( int address ); //TODO: change function name to more properly
    static I2C_Device* begin_communication_for_slave ( int address, void (*request)(void), void (*receive)(size_t) );
    
    bool synchronize_data_format_with_slave ( );
    bool check_synchronizing                ( );
    // void synchronize_if_necessary           ( );
    
    int  get_int_from_response       ();
    int* get_int_array_from_response ();

    void send_sync_data_for_int_array ( int array_size );
    void send_int_array               ( int array[], int array_length );

    void event_for_request ( );
    void event_for_receive ( int a );

    I2C_Device ( int address );
    // I2C_Device ( int device_address, void (*request)(void), void (*receive)(size_t) );
    ~I2C_Device() {}
};

#endif // _I2C_DEVICE_H_
