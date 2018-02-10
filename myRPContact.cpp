#include "myRPlibs.h"
#include "myRPContact.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

// LUX
static MyMessage doorMsg(RP_ID_DOOR, V_ARMED);
static byte id_con = RP_ID_DOOR;

//#define __CLASS_NAME__ className(__PRETTY_FUNCTION__)
//const char className[]  = {__PRETTY_FUNCTION__};
//const char className[]  = {"TEST"};

RpContact::RpContact(byte pin) 
	: RpSensor() {
	_pin = pin;
	Id = id_con;
	SensorType = S_DOOR;
	SensorData = V_ARMED;
	//Ping = 1;
	pinMode(_pin, INPUT_PULLUP); 
	id_con++;					// increace for next pir sensor
	Serial.println("CON pin set");

}

void RpContact::loop() {
	//Serial.println("RPContact Loop");
	byte trip = digitalRead(_pin)>0?1:0;

		if(trip != _prev_door) {
			Serial.println(trip);
			myresend(doorMsg.setSensor(Id).set(trip));
			_prev_door = trip;	
			
		}
}

void RpContact::receiveCReq(const MyMessage &message){
	bool currentState = (bool)digitalRead(_pin);
	myresend(doorMsg.setSensor(Id).set(currentState));
}