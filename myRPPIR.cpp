#include "myRPlibs.h"
#include "myRPPIR.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

// LUX
static MyMessage pirMsg(CHILD_ID_PIR, V_TRIPPED);
static byte id_pir = CHILD_ID_PIR;
static int pirDelay = 10;

void rp_pir_report() {
	rp_addToBuffer("MotionDelay[s]: ");
	rp_addToBuffer(pirDelay);

	rp_reportBuffer();
}

void rp_pir_receive(const MyMessage &message) {
	char *p = (char *)message.data;
	if(message.sensor == RP_ID_CUSTOM) {
		if(*p=='H') {
			myresend(_msgv.set("MD{9s}"));
		}
		if(*p=='M') {
			if(*(++p)=='D') {
				if(*(++p)!='\0') {
					pirDelay = atoi(p);
					EEPROMWriteInt(EE_MOTION_DELAY_OFFSET, pirDelay);
				}				
				rp_pir_report();
			}
		}
	}
}

RpPir::RpPir(byte pin) 
	: RpSensor() {
	_pin = pin;
	_id = id_pir;
	pinMode(_pin, INPUT_PULLUP);
	id_pir++;	// increace for next pir sensor
	Serial.println("PIR pin set");
	EEReadInt(EE_MOTION_DELAY_OFFSET, &pirDelay);
}
void RpPir::receive(const MyMessage &message){
	RpSensor::receive(message);
	rp_pir_receive(message);
}
void RpPir::loop() {
	byte trip;
	trip = digitalRead(_pin)>0?1:0;

	if(trip != _prev_pir) {
		if((trip == 1) || ((millis() - _lastSendPir) > (uint32_t)pirDelay*1000)) {			
			myresend(pirMsg.setSensor(_id).set(trip));
			_lastSendPir = millis();
			_prev_pir = trip;
		}
	} 
	else 
		_lastSendPir = millis();
}

void RpPir::loop_first() {
	rp_pir_report();
}

void RpPir::presentation() {
	present(_id, S_MOTION);
}

