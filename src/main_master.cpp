#include <SFE_BMP180.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "I2C_Slave.h"

#define SLAVE_ADDR 9
#define TIMEOUT_TIME 2000

#define MY_SSID "ssid"
#define PASSWORD "pass"

typedef struct sensor 
{
	int pin;
	int result;
} sensor;

typedef struct sensors
{
	sensor *sensor_list;
	
	bool new_sensor_added = false;
	int  number_of_sensors       ;
	int  active_sensors          ;
} sensors;

ESP8266WebServer server(80);
SFE_BMP180 pressure;
I2C_Slave *sensors_slave;

sensors all_sensors;
double T;

/* ------------------------------- FUNCTION PROTORYPES --------------------------------------------- */

bool get_temperature (double *temperature_to_set);

void handle_sensor_settings();
void handle_main           ();

bool refresh_sensors_list(int active_sensors);
bool add_sensor(int pin);

/* -------------------------------------------------------------------------------------------------- */

void setup() {
	sensors_slave = I2C_Slave::begin_communication(SLAVE_ADDR);

	Serial.begin(9600);
	Serial.println("I2C Master");

	all_sensors.sensor_list = (sensor*)calloc(3, sizeof(sensor));
	all_sensors.sensor_list[0] = { 0, 0 };
	all_sensors.sensor_list[1] = { 1, 0 };
	all_sensors.sensor_list[2] = { 2, 0 };
	all_sensors.number_of_sensors = 3;
	all_sensors.active_sensors = 0b00000111;

	if (pressure.begin())
		Serial.println("BMP180 init success");
	else
	{
		Serial.println("BMP180 init fail\n\n");
		// while(1); // Pause forever.
	}

	WiFi.begin(MY_SSID, PASSWORD);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected.");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
	
	server.on("/", HTTP_GET, handle_main);
	server.on("/set_sensors", HTTP_POST, handle_sensor_settings);

	server.begin();
}

void loop() {
	delay(5000);

	if (sensors_slave->get_is_synchronized()) {
		int *sensors_data = NULL;

		// for future
		// if (all_sensors.new_sensor_added) {
		// 	sensors_slave->send_request(SLAVE_ADDR, 1, 64); // TODO: change to more cleare
		// }

		sensors_data = sensors_slave->get_int_array_from_response();
		
		if (sensors_data[0] != all_sensors.active_sensors) {
			sensors_slave->set_is_synchronized(false);
			all_sensors.active_sensors = sensors_data[0];
			refresh_sensors_list(sensors_data[0]);
		} else {
			Serial.println("----------------------------");
			if (sensors_data != NULL) {
				for (int i = 1; i < sensors_slave->get_recive_data_length(); i++) {
					all_sensors.sensor_list[i - 1].result = sensors_data[i];
				}
			}

		//for test only
		Serial.println("N: " + String(all_sensors.number_of_sensors));
		for (int i = 0; i < all_sensors.number_of_sensors; i++) {
			Serial.println("S" + String(all_sensors.sensor_list[i].pin) + ": " + String(all_sensors.sensor_list[i].result));
		}

		}
	} else {
		sensors_slave->synchronize_data_format_with_slave();
	}

	if (get_temperature(&T)) {
		Serial.println("T: " + String(T));
	}

	server.handleClient();

	delay(1);
}


bool get_temperature(double *temperature_to_set) {
	char status = pressure.startTemperature();
	
	if (status != 0)
	{
		delay(status);
		
		status = pressure.getTemperature(*temperature_to_set);
		
		if (status == 0) {
			Serial.println("error retrieving temperature measurement\n");
			
			return false;
		}
	} else {
		Serial.println("error starting temperature measurement\n");
		
		return false;
	}

	return true;
}

void handle_sensor_settings() {
	if (!server.hasArg("pin") || !server.hasArg("id")) {
		server.send(400, "text/plain", "400: Invalid Request");

		return;
	}

	// if ( ! add_sensor(server.arg("id").toInt(), server.arg("pin").toInt())) {
	// 	server.send(400, "text/plain", "400: Can not add sensor");

	// 	return;
	// }

	server.send(200, "text/html", "Sensor added!");
}

void handle_main() {
	String main_page = "";

	main_page += "<!DOCTYPE HTML>";
	main_page += "<html>";

	for (int i = 0; i < all_sensors.number_of_sensors; i++) {
		main_page += "S" + String(i) + ": " + String(all_sensors.sensor_list[i].result) + " "; 
	}

	main_page += "<br />T: " + String(T) + "C";
	main_page += "</html>";

	server.send(200, "text/html", main_page);
}

bool refresh_sensors_list(int active_sensors) {
	int j = 0;
	for (int i = 1; i <= active_sensors; i <<= 1, j++) {
		bool match = false;
		for (int k = 0; k < all_sensors.number_of_sensors; k++) {
			if (all_sensors.sensor_list[k].pin == j) {
				match = true;

				break;
			}
		}

		if (i & active_sensors) {
			if (!match) add_sensor(j);
		} else {
			// if (match) remove_sensor(j);
			;
		}
	}

	return true;
}

bool add_sensor(int pin) {
	all_sensors.number_of_sensors++;
	all_sensors.sensor_list = (sensor*)realloc(all_sensors.sensor_list, sizeof(sensor) * all_sensors.number_of_sensors);
	all_sensors.sensor_list[all_sensors.number_of_sensors - 1].pin = pin;
	all_sensors.sensor_list[all_sensors.number_of_sensors - 1].result = 0;
	all_sensors.new_sensor_added = true;

	Serial.println("ADD: pin-" + String(pin) + " to-" + String(all_sensors.number_of_sensors - 1));

	if (all_sensors.sensor_list == NULL) {
		free(all_sensors.sensor_list);

		return false;
	}

	return true;
}