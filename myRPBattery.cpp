#include "myRPlibs.h"
#include "myRPBattery.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

#define RP_ID_SIGNAL			197
#define RP_ID_VOLTAGE			198
MyMessage vMsg(RP_ID_VOLTAGE, V_VOLTAGE);

float dtBat = 3;
float vBat;

//#define R1	0
//#define R1	1e6
//#define R2	1
//#define R2	330e3
//#define MAX_V_BAT     4.990      // voltage
//#define MIN_V_BAT     3.500       // voltage
#define VCC     5.0       // voltage


RpBattery::RpBattery(byte adcPin) 
	: RpSensor() {
	_pin = adcPin;
	_pinI1 = INTERRUPT_NOT_DEFINED;
	_pinI2 = INTERRUPT_NOT_DEFINED;
	_modeI1 = MODE_NOT_DEFINED;
	_modeI2 = MODE_NOT_DEFINED;
	Id = RP_ID_VOLTAGE;
	setDivider(1e6, 330e3);
	// digitalPinToInterrupt(INTERRUPT_PIN_2)
	SensorType = S_MULTIMETER;
	SensorData = V_VOLTAGE;
	Serial.println("BATTERY pin set");
	rp_sleepMode = RP_SLEEP_MODE_SLEEP;
}

RpBattery* RpBattery::setDivider(float r1, float r2) {
	_r1 = r1;
	_r2 = r2;
	return this;
}
RpBattery* RpBattery::setBattery(float minBat, float maxBat) {
	_minBat = minBat;
	_maxBat = maxBat;
	return this;
}

RpBattery* RpBattery::wakeUpPin(byte pinI, byte mode) {
	byte i = digitalPinToInterrupt(pinI);
	if(i==0) {
		_pinI1 = pinI;
		_modeI1 = mode;
		Serial.print("Int1: ");	
		Serial.print(_pinI1);
		Serial.print(',');
		Serial.println(_modeI1);
	} else if(i==1) {
		_pinI2 = pinI;
		_modeI2 = mode;
		Serial.print("Int2: ");	
		Serial.print(_pinI2);
		Serial.print(',');
		Serial.println(_modeI2);
	}
	return this;
}

RpBattery* RpBattery::sleepTime(uint32_t sleepTime) {
	_sleepTime = sleepTime;
	return this;
}

void RpBattery::before() {
	analogReference(DEFAULT);
	vBat = readBattery();//_present(SIGNAL_CHILD_ID, S_SOUND);
}

void RpBattery::presentation() {
	RpSensor::presentation();
	present(RP_ID_SIGNAL, S_SOUND);
}

void RpBattery::loop_end() {
	if(rp_sleepMode == RP_SLEEP_MODE_SLEEP) {
		sendHeartbeat();
		wait(MY_SMART_SLEEP_WAIT_DURATION_MS);
		int8_t i = sleep(digitalPinToInterrupt(_pinI1), _modeI1,digitalPinToInterrupt(_pinI2), _modeI2, _sleepTime, false);
		if(i == -1) {
			rp_add_sleep_time += _sleepTime;
		} else {
			//Serial.print("Interupt: ");
			//Serial.println(i);
		}
	} else if(rp_sleepMode == RP_SLEEP_MODE_WAIT) {
		wait(_sleepTime);
	}
}

void RpBattery::loop_1s_tick() {

	updateBatteryLevel();
	signalReport();
}

void RpBattery::receive(const MyMessage &message){
}

void RpBattery::receiveCReq(const MyMessage &message){
	
	updateBatteryLevel();
}

float RpBattery::readBattery() {
	int v = analogRead(_pin);
	float batteryV  = v * VCC*(_r1+_r2)/_r2/1023; //  * 0.0034408602150538;	
	//batteryV = constrain(batteryV, MIN_V_BAT, MAX_V_BAT);
	return batteryV;
}

void RpBattery::updateBatteryLevel() {
	//int v = analogRead(BATT_PIN);
	
	//Serial.print(v);
	//float batteryV  = v * 0.0034408602150538;	// 1.1*(330e3+150e3)/150e3/1023;
	//batteryV = constrain(batteryV, MIN_V_BAT, MAX_V_BAT);
	float batteryV = readBattery();
	vBat = batteryV;//(vBat*dtBat+batteryV)/(dtBat+1);
	byte v_prct = (vBat - _minBat)*100./(_maxBat - _minBat) + .5;
	
	v_prct=constrain(max(v_prct,0), 0, 100);
	byte proc = v_prct;
	if(_prevProcBat != proc || (rp_now - _lastBatReport) > rp_force_time*1000UL*60) {
		
		_prevProcBat = proc;
		_lastBatReport = rp_now;

#if RP_DEBUG==1
		Serial.print(F("Vcc [mv]:") );
		Serial.print(batteryV);
		Serial.print(F(", ") );
		Serial.print(vBat );
		Serial.print(F(",[%]:") );
		Serial.println(proc);
#endif

		sendBatteryLevel(proc);
		myresend(vMsg.set(vBat, 2)); 
	}
}

void RpBattery::signalReport() {
	static int16_t prevLevel = 0;
    int16_t value = transportGetSignalReport(SR_TX_RSSI);
    // report RF signal level
	if(prevLevel!=value) {
		MyMessage signal_msg(RP_ID_SIGNAL, V_LEVEL);
		myresend(signal_msg.set(value));
		prevLevel = value;
	}
  }