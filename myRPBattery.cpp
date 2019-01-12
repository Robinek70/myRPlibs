#include "myRPlibs.h"
#include "myRPBattery.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>
#include <../MySensors/hal/architecture/MyHw.h>

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


//#define VCC     5.0       // voltage
//#define VCC     1.1       // voltage

#define RP_EE_SLEEP_UNIT	0
#define RP_EE_SLEEP_TIME	1




RpBattery::RpBattery(byte adcPin, uint8_t refType, uint16_t refV) 
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
	//Serial.println(F("BATTERY pin set"));
	rp_sleepMode = RP_SLEEP_MODE_SMART;
	_eeLength = 2;
	byte u = 'S';
	byte v = 10;
	EEReadByte(eeOffset + RP_EE_SLEEP_UNIT, &u);
	EEReadByte(eeOffset + RP_EE_SLEEP_TIME, &v);
	_sleepTime = calcTimestamp((char)u, v);
	_vccRef = refV;

	//analogReference(DEFAULT);
	//analogReference(INTERNAL);
	analogReference(refType);
	vBat = readBattery();//_present(SIGNAL_CHILD_ID, S_SOUND);
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
		Serial.print(F("Int1: "));	
		Serial.print(_pinI1);
		Serial.print(',');
		Serial.println(_modeI1);
	} else if(i==1) {
		_pinI2 = pinI;
		_modeI2 = mode;
		Serial.print(F("Int2: "));	
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
	
}

void RpBattery::presentation() {
	RpSensor::presentation();
	present(RP_ID_SIGNAL, S_SOUND);
}

void RpBattery::loop_end() {
	if(rp_sleepMode == RP_SLEEP_MODE_WAIT) {
		wait(_sleepTime);
	} else if(rp_sleepMode != RP_SLEEP_MODE_NONE) {
		//myresend(_msgv.set("battery_sleep"));
		//sendHeartbeat();
		//wait(MY_SMART_SLEEP_WAIT_DURATION_MS);
		//int8_t i = sleep(digitalPinToInterrupt(_pinI1), _modeI1,digitalPinToInterrupt(_pinI2), _modeI2, _sleepTime, false);
		//transportDisable();
		//transportReInitialise();
		int8_t i = sleep(digitalPinToInterrupt(_pinI1), _modeI1, digitalPinToInterrupt(_pinI2), _modeI2, _sleepTime, rp_sleepMode == RP_SLEEP_MODE_SMART);
		//hwSleep(_sleepTime);

		if(i == -1) {
			rp_add_sleep_time += _sleepTime;
		} else {
			//Serial.print("Interupt: ");
			//Serial.println(i);
		}
		//myresend(_msgv.set("battery_wakeup"));
	}
}

void RpBattery::loop_1s_tick() {
	if((rp_now - _lastBatReport) > rp_force_time*1000UL*60) {
		/*Serial.print(rp_now);
		Serial.print(',');
		Serial.print(_lastBatReport);
		Serial.print(',');
		Serial.println(rp_force_time*1000UL*60);*/
		updateBatteryLevel(true);
	}
	
	//signalReport();
}

void RpBattery::loop_first() {	
	updateBatteryLevel(true);
}

void RpBattery::receive(const MyMessage &message){
}

void RpBattery::receiveCReq(const MyMessage &message){
	
	//updateBatteryLevel();
}

void RpBattery::receiveCommon(const MyMessage &message){
	char *p = (char *)message.data;
	if(*p=='S') {
			char u = message.data[1];
			byte v = (byte)atoi(&(message.data[2]));
			//Serial.println(u);
			//Serial.println(',');
			//Serial.println(v);
			
			//uint32_t multiple = u=='S'?1:(u=='M'?60:(u=='H'?60UL*60:(u=='D'?24UL*60*60:1)));
			//_sleepTime = multiple * v * ((u=='U')?1:1000);
			saveState(eeOffset + RP_EE_SLEEP_UNIT, u);
			saveState(eeOffset + RP_EE_SLEEP_TIME, v);
			_sleepTime = calcTimestamp((char)u, v);
			//Serial.println(_sleepTime);
			// myresend(_msgv.set(sleep_time));
		}
}
static const char cHelp[] PROGMEM  =  {"S{U|S|M|H|D}{9} sleep time"};
static const char cSleepTime[] PROGMEM  =  {"Sleep time (S):"};

void RpBattery::help() {
	myresend(_msgv.set(myF(cHelp)));
}

void RpBattery::report() {
	byte u = 'S';
	byte v = 10;
	EEReadByte(eeOffset + RP_EE_SLEEP_UNIT, &u);
	EEReadByte(eeOffset + RP_EE_SLEEP_TIME, &v);
	rp_addPToBuffer(cSleepTime);
	rp_addToBuffer((char)u);
	rp_addToBuffer(v);
	rp_reportBuffer();
}

float RpBattery::readBattery() {
	int v = analogRead(_pin);
	float batteryV  = v * _vccRef*(_r1+_r2)/_r2/1023; //  * 0.0034408602150538;	
	//batteryV = constrain(batteryV, MIN_V_BAT, MAX_V_BAT);
	return batteryV;
}

void RpBattery::updateBatteryLevel(bool forceReport) {
	//if((rp_now - _lastBatReport) < rp_force_time*1000UL*60) {
	//	return;
	//}

	float batteryV = readBattery();
	vBat = batteryV;//(vBat*dtBat+batteryV)/(dtBat+1);
	byte v_prct = (vBat - _minBat)*100./(_maxBat - _minBat) + .5;
	
	v_prct=constrain(max(v_prct,0), 0, 100);
	byte proc = v_prct;
	if(_prevProcBat != proc || forceReport /*|| (rp_now - _lastBatReport) > rp_force_time*1000UL*60*/) {
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