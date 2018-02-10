#include "myRPlibs.h"
#include "myRPTouch.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

// LUX
static MyMessage swMsg(RP_ID_RELAY, V_STATUS);
//static byte id_relay = RP_ID_RELAY;


RpTouch::RpTouch(byte pin, byte commonPin) 
	: RpSensor() {
	_pin = pin;
	_commonPin = commonPin;
	Id = getFreeIdBinary() ;
	SensorType = S_BINARY;
	SensorData = V_STATUS;
	Ping = 1;
	ts = new CapacitiveSensor(_commonPin, _pin); 
	_prevState = 0;
	start1 = 0;
	firstOn = 0;
	_threshold = 35;
	_eeLength = 3;
	EEReadInt(eeOffset, &_threshold);
	//pinMode(pin, INPUT_PULLUP);
    //digitalWrite(pin, loadState(sensor)?RELAY_ON:RELAY_OFF);
	//id_relay++;					// increace for next relay sensor
	Serial.println("Touch pin set");

}

void RpTouch::setup() {
	//Serial.println("Sending request...");
}
//long avg;
int dt = 4;
void RpTouch::loop() {
	long total1 =  ts->capacitiveSensor(20);
	_avg = total1;
		//(_avg*dt+total1)/(dt + 1);
	
	*rp_buffer='\0';
	byte current1 = _prevState;

	if ( _avg > _threshold )
      current1 = 1;

     if(_avg < _threshold - 5)
       current1 = 0;

	 if(_prevState != current1) {
		 if(_prevState) {

		 } else {
			 
			 if(firstOn == 0) {
				start1 = rp_now;
				firstOn = 1;
				rp_addToBuffer(", total:");
				rp_addToBuffer(total1);
				rp_reportBuffer();
			 } else {
				 if(firstOn==1) {
					 if((rp_now - start1) < 500UL) {
						 firstOn = 2;
					 }
				 }
			 }
		 }
		 _prevState = current1;
		 //myresend(swMsg.setSensor(Id).set(current1));
		 //Serial.println(current1);
	 }

	 if(firstOn == 1 && current1 == 1) {
		 if((rp_now - start1) > 2000UL) {
			// long press
			rp_addToBuffer("LongPress");
			rp_addToBuffer(", total:");
			rp_addToBuffer(total1);
			rp_reportBuffer();
			firstOn = 0;
		 }
	 }
	 if(firstOn == 1 && current1 == 0) {
		 if((rp_now - start1) > 1000UL) {
			// short press
			rp_addToBuffer("ShortPress");
			rp_addToBuffer(", total:");
			rp_addToBuffer(total1);
			rp_reportBuffer();
			firstOn = 0;
		 }
	 }
	 if(firstOn == 2) {
		rp_addToBuffer("DoublePress");
		rp_addToBuffer(", total:");
		rp_addToBuffer(total1);

		rp_reportBuffer();
		firstOn = 0;
	}

	  if(firstOn == 0 && current1 == 1 && (rp_now - start1) > 20000UL) {
		  _threshold = _avg + _avg/5;
		  report();
	  }
	 /*if(firstOn != 0) {
	 Serial.print("First:");
	 Serial.print(firstOn);
	 Serial.print(", curr:");
	 Serial.print(current1);
	 Serial.print(", avg:");
	 Serial.print(_avg);
	 Serial.print(", total:");
	 Serial.println(total1);
	 }*/
	

	/**rp_buffer='\0';
	rp_addToBuffer("avg:");
	rp_addToBuffer(_avg);
	rp_addToBuffer(", total:");
	rp_addToBuffer(total1);
	rp_reportBuffer();
	*rp_buffer='\0';*/
	//wait(300);
}

const char cAvg[] PROGMEM  =  {"AVG:"};

void RpTouch::receiveCReq(const MyMessage &message){
	//bool currentState = (bool)digitalRead(_pin);
	//myresend(swMsg.setSensor(Id).set(currentState));
}

void RpTouch::receive(const MyMessage &message){
	if(message.type==V_VAR1) {
		const char* data = message.data;
		if( mystrncmp(cAvg, data, 4)) {
			int t = atoi(&data[4]);
			if(t > 0) {
				_threshold = t;
				EEPROMWriteInt(eeOffset, _threshold);
			}
			report();
		}
	}
}

bool RpTouch::getState(){
	//return (bool)digitalRead(_pin);
	return 0;
}

void RpTouch::report() {
	rp_addPToBuffer(cAvg);
	rp_addToBuffer(_threshold);
	rp_addToBuffer(',');
	rp_addToBuffer(_avg);
	rp_reportBuffer();
}

void RpTouch::help() {
	rp_addPToBuffer(cAvg);
	rp_addToBuffer("[999]");	
	rp_reportBuffer();
}