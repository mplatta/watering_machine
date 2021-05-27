#include <SFE_BMP180.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include "I2C_Slave.h"
#include "sensors.h"


// Const definition
#define MAIN_LOOP_DELAY 5000 // 5s

#define SLAVE_ADDR 9

#define MY_SSID  "ssid"
#define PASSWORD "pass"

#define WIFI_CONNECTION_DELAY   500   // 0.5s
#define WIFI_CONNECTION_TIMEOUT 20000 // 20s

// Global variable
ESP8266WebServer g_server(80);
SFE_BMP180       g_BMP180;
I2C_Slave       *g_sensors_slave;
sensors          g_all_sensors;

bool   g_is_BMP180_connected;
double g_T;
double g_preasure;

bool is_OnlineMode;

/* ------------------------------- FUNCTION PROTORYPES --------------------------------------------- */
bool check_BMP180_connection ( );

bool get_data_from_slave_check_sync ( int    *data );
bool get_temperature_and_preasure   ( double *temperature_to_set, double *preasure_to_set);

void handle_main            ( );
void handle_sensor_settings ( );
void handle_get_data        ( );

/* --------------------------------------- MAIN ---------------------------------------------------- */
void setup() {
	g_sensors_slave = I2C_Slave::begin_communication(SLAVE_ADDR);

	Serial.begin(9600);
	Serial.println("I2C Master");

	// Initial 3 default sensors
	g_all_sensors.sensor_list = (sensor*)calloc(3, sizeof(sensor));
	g_all_sensors.sensor_list[0] = { 0, 0 };
	g_all_sensors.sensor_list[1] = { 1, 0 };
	g_all_sensors.sensor_list[2] = { 2, 0 };
	g_all_sensors.number_of_sensors = 3;
	g_all_sensors.active_sensors = 0b00000111;

	check_BMP180_connection();

	// TODO: Move to function
	// ---------------------------- WIFI CONNECTION ----------------------
	WiFi.begin(MY_SSID, PASSWORD);

	int time_elapsed = 0;

	while ((WiFi.status() != WL_CONNECTED) && (time_elapsed < WIFI_CONNECTION_TIMEOUT)) {
		delay(WIFI_CONNECTION_DELAY);

		time_elapsed += WIFI_CONNECTION_DELAY;
		Serial.print(".");
		// TODO: Change it to more than one SSID
	}

	if (WiFi.status() != WL_CONNECTED) {
		is_OnlineMode = false;
	} else {
		is_OnlineMode = true;
	}

	if (is_OnlineMode) {
		Serial.println("");
		Serial.println("WiFi connected.");
		Serial.println("IP address: ");
		Serial.println(WiFi.localIP());
	
		// API
		g_server.on("/"           , HTTP_GET , handle_main           );
		g_server.on("/set_sensors", HTTP_POST, handle_sensor_settings);
		g_server.on("/get_data"   , HTTP_GET , handle_get_data       );

		g_server.begin();
	}
	// -----------------------------------------------------------------
}

void loop() {
	check_BMP180_connection();

	if (g_sensors_slave->get_is_synchronized()) {
		int *sensors_data = NULL;
		sensors_data = g_sensors_slave->get_u_int_array_from_response();
		
		get_data_from_slave_check_sync(sensors_data);
	} else {
		Serial.println("Synchronization....");
		g_sensors_slave->synchronize_data_format_with_slave();
	}

	if (get_temperature_and_preasure(&g_T, &g_preasure)) {
		Serial.println("T: " + String(g_T));
		Serial.println("P: " + String(g_preasure));
	}

	if (is_OnlineMode) g_server.handleClient();

	delay(MAIN_LOOP_DELAY);
}

/* ------------------------------------ DEFINITIONS ------------------------------------------------ */
bool check_BMP180_connection ( ) {
	if (g_is_BMP180_connected) {
		if (isnan(g_T) || isnan(g_preasure)) {
			g_is_BMP180_connected = false;
		}
	}

	if (!g_is_BMP180_connected) {
		if (g_BMP180.begin()) {
			g_is_BMP180_connected = true;
			Serial.println("BMP180 init success");
		} else {
			g_is_BMP180_connected = false;
			Serial.println("BMP180 init fail\n\n");
		}
	}
	
	return true;
}

bool get_data_from_slave_check_sync ( int *data ) {
	if (data[0] != g_all_sensors.active_sensors) {
		g_sensors_slave->set_is_synchronized(false);
		refresh_sensors_list(data[0], &g_all_sensors);
	} else {
		Serial.println("----------------------------");
		if (data != NULL) {
			for (int i = 1; i < g_sensors_slave->get_recive_data_length(); i++) {
				g_all_sensors.sensor_list[i - 1].result = data[i];
			}
		}

		//for test only
		Serial.println("N: " + String(g_all_sensors.number_of_sensors));
		Serial.println("MASK: " + String(g_all_sensors.active_sensors));
		for (int i = 0; i < g_all_sensors.number_of_sensors; i++) {
			Serial.println("S" + String(g_all_sensors.sensor_list[i].pin) + ": " + String(g_all_sensors.sensor_list[i].result));
		}

	}

	return true;
}

bool get_temperature_and_preasure( double *temperature_to_set, double *preasure_to_set ) {
	bool result = true;
	char status = g_BMP180.startTemperature();
	
	if (status) {
		delay(status);
		status = g_BMP180.getTemperature(*temperature_to_set);

		if (status) {
			status = g_BMP180.startPressure(3);

			if (status) {
				delay(status);
				status = g_BMP180.getPressure(*preasure_to_set, *temperature_to_set);
				
				if (!status) {
					g_is_BMP180_connected = false;
					result = false;
				}
			} else {
				g_is_BMP180_connected = false;
				result = false;
			}
		} else {
			g_is_BMP180_connected = false;
			result = false;		
		}
	} else {
		g_is_BMP180_connected = false;
		result = false;
	}
	
	return result;
}

void handle_main ( ) {
	String main_page = "";

	main_page += "<!DOCTYPE HTML>";
	main_page += "<html>";

	for (int i = 0; i < g_all_sensors.number_of_sensors; i++) {
		main_page += "S" + String(g_all_sensors.sensor_list[i].pin) + ": " + String(g_all_sensors.sensor_list[i].result) + " "; 
	}

	main_page += "<br />g_T: " + String(g_T) + "C";
	main_page += "</html>";

	g_server.send(200, "text/html", main_page);
}

void handle_sensor_settings ( ) {
	if (!g_server.hasArg("pin") || !g_server.hasArg("id")) {
		g_server.send(400, "text/plain", "400: Invalid Request");

		return;
	}

	// if ( ! add_sensor(g_server.arg("id").toInt(), g_server.arg("pin").toInt())) {
	// 	g_server.send(400, "text/plain", "400: Can not add sensor");

	// 	return;
	// }

	g_server.send(200, "text/html", "Sensor added!");
}

void handle_get_data ( ) {
	String raw_data;
	StaticJsonDocument<800> doc;
	
	doc["temperature"]    = g_T;
	doc["preasure"]       = g_preasure;
	doc["active_sensors"] = g_all_sensors.active_sensors;

	for (int i = 0; i < g_all_sensors.number_of_sensors; i++) {
		doc["sensors_data"][i]["pin"]      = g_all_sensors.sensor_list[i].pin;
		doc["sensors_data"][i]["moisture"] = g_all_sensors.sensor_list[i].result;
	}

	serializeJson(doc, raw_data);

	g_server.send(200, "application/json", raw_data);
	Serial.println(raw_data);
}