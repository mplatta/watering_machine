#include <SFE_BMP180.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "I2C_Slave.h"
#include "sensors.h"

#define SLAVE_ADDR 9
#define TIMEOUT_TIME 2000

#define MY_SSID "ssid"
#define PASSWORD "pass"

ESP8266WebServer server(80);
SFE_BMP180 pressure;
I2C_Slave *sensors_slave;

sensors all_sensors;
double T;

/* ------------------------------- FUNCTION PROTORYPES --------------------------------------------- */

bool get_temperature (double *temperature_to_set);

void handle_sensor_settings ( );
void handle_main            ( );
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

		sensors_data = sensors_slave->get_int_array_from_response();
		
		if (sensors_data[0] != all_sensors.active_sensors) {
			sensors_slave->set_is_synchronized(false);
			refresh_sensors_list(sensors_data[0], &all_sensors);
		} else {
			Serial.println("----------------------------");
			if (sensors_data != NULL) {
				for (int i = 1; i < sensors_slave->get_recive_data_length(); i++) {
					all_sensors.sensor_list[i - 1].result = sensors_data[i];
				}
			}

		//for test only
		Serial.println("N: " + String(all_sensors.number_of_sensors));
		Serial.println("MASK: " + String(all_sensors.active_sensors));
		for (int i = 0; i < all_sensors.number_of_sensors; i++) {
			Serial.println("S" + String(all_sensors.sensor_list[i].pin) + ": " + String(all_sensors.sensor_list[i].result));
		}

		}
	} else {
		Serial.println("Synchronization....");
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