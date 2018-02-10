#include "myRPlibs.h"
#include "myRPPIRInt.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

// LUX
static MyMessage pirMsg(RP_ID_PIR, V_TRIPPED);
static byte id_pir = RP_ID_PIR;

RpPirInt::RpPirInt(byte pin, bool enabled) 
	: RpSensor(enabled) {
	_pin = pin;
	Id = id_pir;
	SensorType = S_MOTION;
	SensorData = V_TRIPPED;
	//Ping = 1;
	pinMode(_pin, INPUT_PULLUP);
	id_pir++;							// increace for next pir sensor
	Serial.println("PIR pin set");
}

void RpPirInt::setup() {
}

void RpPirInt::loop() {
	update();
}

void RpPirInt::loop_end() {
	update();
}

void RpPirInt::update() {
	byte trip;
	trip = digitalRead(_pin)>0?1:0;
	
	if(trip != _prev_pir) {		
		Serial.println(trip);
		myresend(pirMsg.setSensor(Id).set(trip));
		
		_prev_pir = trip;
	} 
}
