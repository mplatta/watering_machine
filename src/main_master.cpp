#include <SFE_BMP180.h>
#include <ESP8266WiFi.h>

#include "I2C_Slave.h"

// #define MASTER 1  // tmp solution
// #include "i2c_comunnication.h"

#define SLAVE_ADDR 9

#define TIMEOUT_TIME 2000

#define MY_SSID "ssid"
#define PASSWORD "pass"

WiFiServer server(80);

SFE_BMP180 pressure;
I2C_Slave *sensors_slave;

bool is_synchronized = false;

/* ------------------------------- FUNCTION DEFINITIONS --------------------------------------------- */

bool get_temperature (double *temperature_to_set);

/* -------------------------------------------------------------------------------------------------- */

void setup() {
	sensors_slave = I2C_Slave::begin_communication(SLAVE_ADDR);

	Serial.begin(9600);
	Serial.println("I2C Master");

	WiFi.begin(MY_SSID, PASSWORD);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected.");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
	
	server.begin();

	if (pressure.begin())
		Serial.println("BMP180 init success");
	else
	{
		Serial.println("BMP180 init fail\n\n");
		while(1); // Pause forever.
	}
}

void loop() {
	double T;

	int s1 = 0;
	int s2 = 0;
	int s3 = 0;

	delay(5000);

	if (sensors_slave->get_is_synchronized()) {
		int *sensors_data = NULL;

		sensors_data = sensors_slave->get_int_array_from_response();

		Serial.println("----------------------------");
		if (sensors_data != NULL) {
			for (int i = 0; i < sensors_slave->get_recive_data_length(); i++) {
				Serial.println(String(sensors_data[i]));
			}

			s1 = sensors_data[0];
			s2 = sensors_data[1];
			s3 = sensors_data[2];
		}
	} else {
		sensors_slave->synchronize_data_format_with_slave();
	}

	if (get_temperature(&T)) {
		Serial.println("T: " + String(T));
	}
	
	WiFiClient client = server.available();

	if (client) { 
		Serial.println("New Client.");

		while(!client.available()) {
		  delay(1);
		}

		String request = client.readStringUntil('\r');
		Serial.println(request);
		client.flush();

		client.println("HTTP/1.1 200 OK");
		client.println("Content-Type: text/html");
		client.println("");
		client.println("<!DOCTYPE HTML>");
		client.println("<html>");
		client.println("S1: " + String(s1) + "  S2: " + String(s2) + "  S3: " + String(s3));
		client.println("T: " + String(T) + "C");
		client.println("</html>");

		delay(1);
	}
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
