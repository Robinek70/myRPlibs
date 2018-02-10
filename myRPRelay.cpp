#include "myRPlibs.h"
#include "myRPRelay.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

// LUX
static MyMessage swMsg(RP_ID_RELAY, V_STATUS);
//static byte id_relay = RP_ID_RELAY;
#define RELAY_ON		1  
#define RELAY_OFF		0 


RpRelay::RpRelay(byte pin, bool invertActive) 
	: RpSensor() {
	_pin = pin;
	Id = getFreeIdBinary();
	_invert = invertActive;
	SensorType = S_BINARY;
	SensorData = V_STATUS;
	Ping = 1;
	pinMode(pin, OUTPUT);
    //digitalWrite(pin, loadState(sensor)?RELAY_ON:RELAY_OFF);
	//id_relay++;					// increace for next relay sensor
	Serial.println("RELAY pin set");

}

void RpRelay::setup() {
	Serial.println("Sending request...");
	request(Id, SensorData);
}

void RpRelay::loop() {
}

void RpRelay::receive(const MyMessage &message){
	//RpSensor::receive(message);
	//myresend(_msgv.set("RELAY"));
	
	if (message.type==V_STATUS) {
		// Change relay state
		bool currentState = (bool)digitalRead(_pin);

		//if(strlen(message.data)>0 ) {
			bool newState = message.getBool()?RELAY_ON:RELAY_OFF;
#if RP_DEBUG == 1
			/*rp_addToBuffer(message.sensor);
			rp_addToBuffer(" -> curr:");
			rp_addToBuffer(currentState);
			rp_addToBuffer(", new:");
			rp_addToBuffer(newState);
			rp_reportBuffer();*/
#endif
			if(currentState != newState) {
				digitalWrite(_pin, newState);
				myresend(swMsg.setSensor(Id).set(newState));
				// Store state in eeprom
				//saveState(message.sensor, message.getBool());
				// Write some debug info
				//Serial.print("Incoming change for sensor:");
				//Serial.print(message.sensor);
				//Serial.print(", New status: ");
				//Serial.println(message.getBool());
				//myresend(_msgv.set("CHANGING"));
			}
	}
}

void RpRelay::receiveCReq(const MyMessage &message){
	bool currentState = (bool)digitalRead(_pin);
	myresend(swMsg.setSensor(Id).set(currentState));
}

bool RpRelay::getState(){
	return (bool)digitalRead(_pin);
}