#ifndef _FORMATTED_LOG_H_
#define _FORMATTED_LOG_H_

#include <Arduino.h>

#ifdef DEBUG
#define DNDEBUG false
#else
#define DNDEBUG true
#endif

static bool is_Serial_active = false;

inline void begin_log(const int frequency) 
{
	if (!is_Serial_active) Serial.begin(frequency);
	is_Serial_active = true;
}

#define formatted_log(msg) \
	do {if (!DNDEBUG) Serial.println(String(__FILE__) + " (" + String(__LINE__) + "): " + String(msg)); } \
	while(0)

#define formatted_info(msg) \
	do { Serial.println(String(msg)); } \
	while(0)

#endif