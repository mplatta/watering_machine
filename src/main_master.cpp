#include <SFE_BMP180.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#define DEBUG

#include "I2C_Slave.h"
#include "sensors.h"
#include "formatted_log.h"
#include "timer.h"

// Const definition
#define MAIN_LOOP_DELAY 5000 // 5s

#define SLAVE_ADDR 9

#define SEND_ME_DATA_MESSAGE 0b01000000

#define MY_SSID  "ssid"
#define PASSWORD "pass"

#define WIFI_CONNECTION_DELAY   500     // 0.5s
#define WIFI_CONNECTION_TIMEOUT 20000   // 20s
#define WIFI_TRY_AGAIN_DELAY    3600000 // 1h

// Global variable
ESP8266WebServer g_server(80);
SFE_BMP180       g_BMP180;
I2C_Slave       *g_sensors_slave;
sensors          g_all_sensors;

bool   g_is_BMP180_connected;
double g_T;
double g_preasure;

bool g_is_OnlineMode;

timer g_main_loop_timer;
timer g_wifi_retry_timer;

/* ------------------------------- FUNCTION PROTORYPES --------------------------------------------- */
bool connect_BMP180_if_necessary ( );
bool connect_wifi                ( );
void set_server_API              ( );

bool get_data_from_slave_check_sync ( );
bool get_temperature_and_preasure   ( double  *temperature_to_set, double *preasure_to_set );

void handle_main            ( );
void handle_sensor_settings ( );
void handle_get_data        ( );
void handle_active_pump     ( );

/* --------------------------------------- MAIN ---------------------------------------------------- */
void setup() {
	g_main_loop_timer  = new_timer();
	g_wifi_retry_timer = new_timer();

	g_sensors_slave = I2C_Slave::begin_communication(SLAVE_ADDR);

	begin_log(9600);
	formatted_info("I2C Master");

	// Initial 3 default sensors
	g_all_sensors.sensor_list = (sensor*)calloc(3, sizeof(sensor));
	g_all_sensors.sensor_list[0] = { 0, 0 };
	g_all_sensors.sensor_list[1] = { 1, 0 };
	g_all_sensors.sensor_list[2] = { 2, 0 };
	g_all_sensors.number_of_sensors = 3;
	g_all_sensors.active_sensors = 0b00000111;

	connect_BMP180_if_necessary();
	g_is_OnlineMode = connect_wifi();
}

void loop() {
	start_timer(&g_main_loop_timer);
	if (wait_periodical(&g_main_loop_timer, MAIN_LOOP_DELAY)) {
		connect_BMP180_if_necessary();

		if (g_sensors_slave->get_is_synchronized()) {
			get_data_from_slave_check_sync();
		} else {
			formatted_info("I2C Synchronization....");
			
			g_sensors_slave->synchronize_data_format_with_slave();
			
			formatted_log(String(g_sensors_slave->get_recive_data_length())); 
			formatted_log(String(g_sensors_slave->get_size_of_recive_type()));
		}

		if (get_temperature_and_preasure(&g_T, &g_preasure)) {
			formatted_log("T: " + String(g_T));
			formatted_log("P: " + String(g_preasure));
		}
	}

	if (g_is_OnlineMode) g_server.handleClient();
	else {
		start_timer(&g_wifi_retry_timer);
		if (wait_periodical(&g_wifi_retry_timer, WIFI_TRY_AGAIN_DELAY)) {
			g_is_OnlineMode = connect_wifi();
		}
	}
}

/* ------------------------------------ DEFINITIONS ------------------------------------------------ */
bool connect_BMP180_if_necessary ( ) {
	if (g_is_BMP180_connected) {
		if (isnan(g_T) || isnan(g_preasure)) {
			g_is_BMP180_connected = false;
		}
	}

	if (!g_is_BMP180_connected) {
		if (g_BMP180.begin()) {
			g_is_BMP180_connected = true;
			formatted_info("BMP180 init success");
		} else {
			g_is_BMP180_connected = false;
			formatted_info("BMP180 init fail\n\n");
		}
	}
	
	return true;
}

bool connect_wifi ( ) {
	bool result = true;
	int time_elapsed = 0;

	WiFi.begin(MY_SSID, PASSWORD);

	while ((WiFi.status() != WL_CONNECTED) && (time_elapsed < WIFI_CONNECTION_TIMEOUT)) {
		delay(WIFI_CONNECTION_DELAY);

		time_elapsed += WIFI_CONNECTION_DELAY;
		formatted_info(".");
		// TODO: Change it to more than one SSID
	}

	if (WiFi.status() != WL_CONNECTED) {
		result = false;
	} else {
		result = true;
	}

	if (result) {
		formatted_info("");
		formatted_info("WiFi connected.");
		formatted_info("IP address: ");
		formatted_info(WiFi.localIP().toString());

		set_server_API();

		g_server.begin();
	}

	return result;
}

void set_server_API ( ) {
	g_server.on("/"           , HTTP_GET , handle_main           );
	g_server.on("/set_sensors", HTTP_POST, handle_sensor_settings);
	g_server.on("/get_data"   , HTTP_GET , handle_get_data       );
	g_server.on("/active_pump", HTTP_POST, handle_active_pump    );
}

bool get_data_from_slave_check_sync ( ) {
	uint8_t *data = NULL;
	data = g_sensors_slave->request_for_u_int8_array(SEND_ME_DATA_MESSAGE);

	if (data[0] != g_all_sensors.active_sensors) {
		g_sensors_slave->set_is_synchronized(false);
		refresh_sensors_list(data[0], &g_all_sensors);
	} else {
		formatted_log("----------------------------");
		if (data != NULL) {
			for (int i = 1; i < g_all_sensors.number_of_sensors; i++) {
				g_all_sensors.sensor_list[i].result = data[i + 1];
			}
		}

		//for test only
		formatted_log("N: " + String(g_all_sensors.number_of_sensors));
		formatted_log("MASK: " + String(g_all_sensors.active_sensors));
		for (int i = 0; i < g_all_sensors.number_of_sensors; i++) {
			formatted_log("S" + String(g_all_sensors.sensor_list[i].pin) + ": " + String(g_all_sensors.sensor_list[i].result));
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
	formatted_log(raw_data);
}

void handle_active_pump ( ) {
	if (!g_server.hasArg("pump_id") || !g_server.hasArg("time")) {
		g_server.send(400, "text/plain", "400: Invalid Request");

		return;
	}
	
	formatted_log("Pumpming....");
	g_sensors_slave->request_for_u_int8_array(0b10011111);
	formatted_log("Pumpming....");
	g_server.send(200, "application/json", "{success: 1}");
}