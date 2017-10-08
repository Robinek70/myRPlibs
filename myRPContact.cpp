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
	_id = id_con;
	pinMode(_pin, INPUT_PULLUP); 
	id_con++;					// increace for next pir sensor
	Serial.println("CON pin set");

}
/*void RpLdr::receive(const MyMessage &message){
	RpSensor::receive(message);	
}*/
void RpContact::loop() {
}

void RpContact::loop_1s_tick(){
	byte trip = digitalRead(_pin)>0?1:0;

		if(trip != _prev_door) {
			myresend(doorMsg.setSensor(_id).set(trip));
			_prev_door = trip;		
		}
}

void RpContact::presentation() {
	present(_id, S_DOOR);
}

