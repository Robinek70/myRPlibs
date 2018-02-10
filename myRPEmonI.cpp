#include "myRPlibs.h"
#include "myRPEmonI.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

// POWER
static MyMessage wattMsg(RP_ID_POWER,V_WATT);
static MyMessage kwhMsg(RP_ID_POWER+1,V_KWH);
static MyMessage var2Msg(RP_ID_POWER,V_VAR2);
static byte id_pwr = RP_ID_POWER;
static int short_interval = 60;


RpEmonI::RpEmonI(byte pin) 
	: RpSensor() {
	_pin = pin;
	Id = id_pwr;
	_idKwh = Id+1;
	SensorType = S_POWER;
	SensorData = V_WATT;
	_numSamples = 1250;
	_voltage = 230;
	_iCal = 1000;
	_iOffset = 0;
	_kwhCount = 0;
	_localKwhCount = 0;
	_prevP = 0;
	MaxIds = 2;
	_lastKwhTime = 0;//4294957296;	// -15s
	_ready = 0;
	//myBuffer = new CircularBuffer(5);
	_countdownTimer = 5;

	//pinMode(pin, OUTPUT);
    //digitalWrite(pin, loadState(sensor)?RELAY_ON:RELAY_OFF);
	id_pwr += 2;					// increace for next relay sensor
	Serial.println("EmonI pin set");
	_sum=0;
	_sumCount=0;
}

RpEmonI* RpEmonI::setVoltage(int v) {
	_voltage = v;
	return this;
}
RpEmonI* RpEmonI::setCurrentCalibration(float iCal, float iOffset) {
	_iCal = iCal;
	_iOffset = iOffset;
	return this;
}
RpEmonI* RpEmonI::setNumSamples(int numSamples) {
	_numSamples = numSamples;
	return this;
}

void RpEmonI::setTimer(int8_t delay) {
	_countdownTimer = delay;
}

void RpEmonI::presentation() {
	present(Id, SensorType);
	present(_idKwh, SensorType);
}

void RpEmonI::setup() {
	Serial.println("Sending request...");
	//request(Id + 1, V_KWH);
	request(_idKwh, V_VAR2);

	_emon1.current(_pin, _iCal); 
}

void RpEmonI::loop_1s_tick() {
	if(!_ready) {
		request(_idKwh, V_VAR2);
		return;
	}

	float Irms = _emon1.calcIrms(_numSamples) - _iOffset;  // Calculate Irms only	

	if(_countdownTimer) {
		_countdownTimer--;
		myresend(_msgv.set(_countdownTimer));
		return;
	}

	int P = Irms*_voltage+.5;
	P = max(0,P);
	_sum+=P;
	_sumCount++;

	//myBuffer->pushElement(P);

	if( abs(P-_prevP)>0) {
		myresend(wattMsg.setSensor(Id).set(P));
		_prevP = P;
	}

	if(rp_now - _lastKwhTime > 1000UL * short_interval) {
		uint32_t avgP = 100UL*_sum/_sumCount;	// avg power multipled by 100
		rp_addToBuffer("Avg: ");
		rp_addToBuffer(avgP);
		rp_addToBuffer(", c: ");
		rp_addToBuffer(_sumCount);
		rp_reportBuffer();
		rp_addToBuffer("sum: ");
		rp_addToBuffer(_sum);
		rp_reportBuffer();
		_sum=0;
		_sumCount=0;
			//myBuffer->averageLast(5);
		uint32_t kwh = (avgP) * (rp_now - _lastKwhTime)*10/1000/60/60;	// ms*10, kWh=/10000/100
		rp_addToBuffer("part kwh: ");
		rp_addToBuffer(kwh);
		rp_reportBuffer();
		
		_localKwhCount += kwh;
		uint32_t localDiv100 = _localKwhCount/100;
		if(_localKwhCount>1000000) {
			_kwhCount += localDiv100;
			_localKwhCount = 0;
		}
		//_kwhCount += kwh;
		myresend(kwhMsg.setSensor(_idKwh).set((_kwhCount+localDiv100)/10000.,4));
		myresend(var2Msg.setSensor(_idKwh).set(_kwhCount+localDiv100));
		_lastKwhTime = rp_now;
	}
}

void RpEmonI::receive(const MyMessage &message){
	if(!_ready && message.type==V_VAR2) {
		_kwhCount+=message.getULong();
		rp_addToBuffer("Counter: ");
		rp_addToBuffer(_kwhCount);
		rp_reportBuffer();
		_ready = 1;
	}
}

void RpEmonI::receiveCReq(const MyMessage &message){
	if(message.sensor == Id) {
		float Irms = _emon1.calcIrms(_numSamples) - _iOffset;  // Calculate Irms only		
		int P = Irms*_voltage+.5;
		P = max(0,P);
			myresend(wattMsg.setSensor(Id).set(P));
		}
}