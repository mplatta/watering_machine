#include <Arduino.h>
#include <Wire.h>

// Define Slave I2C Address
#define SLAVE_ADDR 9

// Define Slave answer size
#define ANSWER_SIZE 3
#define HEADER_LENGTH 2

int answer[3] = {256, 6, 7};
bool synchronizing = false;

void send_sync_data() {
	uint8_t ans[2] = { 0, 0 };

	ans[0] = sizeof(int);
	ans[1] = ANSWER_SIZE;

	Wire.write(ans, HEADER_LENGTH);
  
	Serial.print(String(ans[0]) + " " + String(ans[1])); 
}

void send_sensors_data() {  
	uint8_t* ans = (uint8_t*)answer;

	// Send response back to Master
	Wire.write(ans, ANSWER_SIZE * sizeof(int));
  
	// Print to Serial Monitor
	for (unsigned int i = 0; i < ANSWER_SIZE * sizeof(int); i++) {
		Serial.print(String(ans[i]) + " "); 
	}

	Serial.println("");
}

void requestEvent() {
	if (synchronizing) {
		send_sync_data();
	} else {
		send_sensors_data();
	}
}

void receiveEvent(int a) {
	// Read while data received
	while (0 < Wire.available()) {
		byte x = Wire.read();
   
		if (x == 0) synchronizing = true;
		else synchronizing = false;
	
	}
  
	// Print to Serial Monitor
	Serial.println("Receive event");
}

void setup() {

	// Initialize I2C communications as Slave
	Wire.begin(SLAVE_ADDR);
  
	// Function to run when data requested from master
	Wire.onRequest(requestEvent); 
  
	// Function to run when data received from master
	Wire.onReceive(receiveEvent);

	// Setup Serial Monitor 
	Serial.begin(9600);
	Serial.println("I2C Slave");
}

void loop() {

	// Time delay in loop
	delay(50);
}