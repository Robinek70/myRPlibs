#include "myRPlibs.h"
#include "myRPPIR.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

// LUX
static MyMessage pirMsg(RP_ID_PIR, V_TRIPPED);
static byte id_pir = RP_ID_PIR;
static int pirDelay = 10;

RpPir::RpPir(byte pin, bool enabled) 
	: RpSensor(enabled) {
	_pin = pin;
	Id = id_pir;
	_eeLength = 2;
	SensorType = S_MOTION;
	SensorData = V_TRIPPED;
	//Ping = 1;
	pinMode(_pin, INPUT_PULLUP);
	id_pir++;							// increace for next pir sensor
	Serial.println("PIR pin set");
	EEReadInt(eeOffset, &pirDelay);
}
void RpPir::receive(const MyMessage &message)
{
	char *p = (char *)message.data;
	if(*p=='M') {
		if(*(++p)=='D') {
			if(*(++p)!='\0') {
				int delay = atoi(p);
				setDelay(delay);
			}				
			report();
		}
	}
}
void RpPir::loop() {
	update();
}

void RpPir::loop_end() {
	update();
}

void RpPir::update() {
	byte trip;
	trip = digitalRead(_pin)>0?1:0;

	if(trip != _prev_pir) {
		if((trip == 1) || ((millis() - _lastSendPir) > (uint32_t)pirDelay*1000)) {			
			myresend(pirMsg.setSensor(Id).set(trip));
			_lastSendPir = millis();
			_prev_pir = trip;
		}
	} 
	else 
		_lastSendPir = millis();
}

void RpPir::report() {
	rp_addToBuffer("MotionDelay[s]: ");
	rp_addToBuffer(pirDelay);

	rp_reportBuffer();
}

void RpPir::help() {
	myresend(_msgv.set("MD{9s}"));
}

RpPir* RpPir::setDelay(int delay) {
	if(delay != pirDelay) {
		pirDelay = delay;
		EEPROMWriteInt(eeOffset, pirDelay);
	}
	return this;
}

