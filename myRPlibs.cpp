#include <Arduino.h> 
#include <myRPlibs.h>
#include <../MySensors/core/MySensorsCore.h>
//#include <../MySensors/hal/architecture/AVR/MyHwAVR.h>
#include <../MySensors/hal/architecture/MyHw.h>

#ifndef RP_MAX_REPEATS
	#define RP_MAX_REPEATS	5
#endif

MyMessage _msgv(RP_ID_CUSTOM, V_VAR2);
uint32_t rp_now;
uint32_t rp_add_sleep_time = 0;
uint32_t rp_1s_time;
byte rp_force_time = RP_FORCE_TIME_DEFAULT;
bool rp_first_loop = 1;
bool rp_1s_tick = 0;
byte rp_sleepMode = RP_SLEEP_MODE_NONE;
byte rp_sensors_count = 0;
RpSensor* _rpsensors[RP_MAX_SENSORS];
static uint32_t lastPing = 0;
static byte pingIntervalValue = 5;
static byte pingIntervalUnit = 'M';
static byte id_binary = RP_ID_RELAY;
static byte id_pir = RP_ID_PIR;


char rp_buffer[25];

byte getFreeIdBinary() {
	return id_binary++;
}
byte getFreeIdPir() {
	return id_pir++;
}

void EEPROMWriteInt(int p_address, int p_value)
     {
	int tmp = EEPROMReadInt(p_address);
	if(tmp == p_value) return;

     byte lowByte = ((p_value >> 0) & 0xFF);
     byte highByte = ((p_value >> 8) & 0xFF);

	 saveState(p_address, lowByte);
     saveState(p_address + 1, highByte);
#ifdef RP_DEBUG
	 Serial.println(F("EEPROM updated"));
#endif
     }

unsigned int EEPROMReadInt(int p_address)
     {
     byte lowByte = loadState(p_address);
     byte highByte = loadState(p_address + 1);

     return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
     }

void EEReadInt(int pos, int* data) {
	int tmp = EEPROMReadInt(pos);
	if(tmp != -1) *data = tmp;
}

void EEReadByte(int pos, byte* data) {
	byte tmp = loadState(pos);
	if(tmp != 255) *data = tmp;
}

bool myresend(MyMessage &msg)
{
	byte repeat = 1;
	boolean sendOK = false;


	#ifdef RP_DEBUG
      Serial.print(F("SEND D="));
      Serial.print(msg.destination);
      Serial.print(F(" I="));
      Serial.print(msg.sensor);
      Serial.print(F(" C="));
      Serial.print(msg.getCommand());
      Serial.print(F(" T="));
      Serial.print(msg.type);
	  Serial.print(F(" Data="));
      Serial.println(msg.getString(rp_buffer));
    #endif

	while ((sendOK == false) && (repeat < RP_MAX_REPEATS)) {
		sendOK = send(msg);
		if(!sendOK) {
			repeat++; 
		} else {
			break;
		}	
		wait(20*(1<<repeat));
	}
	return sendOK;
}

byte mystrncmp(const char* flash, const char * s, byte count) {
	byte i=0;
	for(char c; (c = pgm_read_byte(flash)) && i<count;flash++,s++,i++) {
		if(c!=*s) {
			return 0;
		}
	}
	return i==count;
}

char* rp_addPToBuffer(const char* s) {
	char *p = rp_buffer;
	for(char c; (c = pgm_read_byte(s));s++) *(p++)=c;
	*p = '\0';

	return rp_buffer;
}

void rp_addToBuffer(const char* s) {//4294967296
	byte len = strlen(rp_buffer);
	strcpy(rp_buffer+len, s);
}
void rp_addToBuffer(int v) {
	byte len = strlen(rp_buffer);
	itoa(v,rp_buffer+len,10);
}
void rp_addToBuffer(uint32_t v) {
	byte len = strlen(rp_buffer);
	ultoa(v,rp_buffer+len,10);
}
void rp_addToBuffer(int32_t v) {
	byte len = strlen(rp_buffer);
	ltoa(v,rp_buffer+len,10);
}
void rp_addToBuffer(float v, unsigned char decimals) {
	byte len = strlen(rp_buffer);
	dtostrf(v, 2, decimals, rp_buffer+len);
}
void rp_addToBuffer(char c) {
	byte len = strlen(rp_buffer);
	char *p= rp_buffer+len;
	*p=c; p++;*p='\0';
}

void rp_addFromEeprom(int start, byte len) {
	byte n=0;
	byte ok = 1;
	while(ok && n<len) {
		byte x = loadState(start + n);
		n++;
		if(x!=0 && x!= 255) {
			rp_addToBuffer((char)x);
		} else {
			ok = 0;
		}
	}
}

void rp_reportBuffer() {
	Serial.println(rp_buffer);				
	myresend(_msgv.set(rp_buffer));
	*rp_buffer='\0';
}
const char cForceReport[] PROGMEM  =  {"ForceReport(D)[m]: "};
const char cPing[] PROGMEM  =  {"PING:"};
const char cActive[] PROGMEM  =  {"Active(A):"};

void rp_report() {
	rp_addPToBuffer(cForceReport);
	rp_addToBuffer(rp_force_time);
	rp_reportBuffer();

	rp_addPToBuffer(cPing);
	rp_addToBuffer((char)pingIntervalUnit);
	rp_addToBuffer(pingIntervalValue);
	rp_reportBuffer();
	rp_addPToBuffer(cForceReport);
	rp_addToBuffer(rp_force_time);
	rp_reportBuffer();
}

enum MySensorAction {
	PRESENTATION = 1,
	LOOP = 2,
	LOOP_FIRST = 3,
	LOOP_1S = 4,
	HELP = 5,
	REPORT = 6,
	PING = 7,
	SETUP = 8,
	BEFORE = 9,
	LOOP_END = 10
};
void iterateSenors(MySensorAction action) {
	for(byte i=0;i<rp_sensors_count;i++) {
		RpSensor* s =  _rpsensors[i];

		if(s->Disabled) {
			continue;
		}
		switch (action ) {
		case MySensorAction::PRESENTATION:
			s->presentation();
			break;
		case MySensorAction::LOOP:
			s->loop();
			break;
		case MySensorAction::LOOP_FIRST:
			s->loop_first();
			break;		
		case MySensorAction::LOOP_1S:
			s->loop_1s_tick();
			break;
		case MySensorAction::HELP:
			s->help();
			break;
		case MySensorAction::REPORT:
			s->report();
			break;
		case MySensorAction::PING:
			if(s->Ping) {
				//Serial.print(s->Id);	
				//Serial.print(',');	
				//Serial.println(s->SensorData);	
				request(s->Id, s->SensorData);
			}
			break;
		case MySensorAction::SETUP:
			s->setup();
			break;
		case MySensorAction::BEFORE:
			s->before();
			break;
		case MySensorAction::LOOP_END:
			s->loop_end();
			break;
		}
		
	}
}

void iterateReceiveSenors(const MyMessage &msg) {
	for(byte i=0;i<rp_sensors_count;i++) {
		RpSensor* s =  _rpsensors[i];
		if(s->Disabled) {
			continue;
		}
	
		s->processReceive(msg);		
	}
}

void enableSensor(byte id, byte disable) {
	for(byte i=0;i<rp_sensors_count;i++) {
		RpSensor* s =  _rpsensors[i];
	
		if (s->Id <= id &&  id <= s->Id + s->MaxIds - 1 ){
			s->Disabled = disable;
			saveState(s->eeOffset - 1 + 0, disable);	// eeOffset wskazuje przeuniecie ustaiwen urzytkownika (nie wbudowane)
#ifdef RP_DEBUG
			rp_addToBuffer("Found");
			rp_reportBuffer();
#endif
			break;		
		}  
	}
}

void rp_before() {
	EEReadByte(EE_FORCE_REPORT_TIME_OFFSET, &rp_force_time);

	EEReadByte(EE_PING_UNIT, &pingIntervalUnit);
	EEReadByte(EE_PING_TIME, &pingIntervalValue);
	if(pingIntervalValue == 0) {
		pingIntervalValue=1;
		pingIntervalUnit='H';
	}
	iterateSenors(MySensorAction::BEFORE);
}

void rp_presentation() {
	present(RP_ID_CUSTOM, S_CUSTOM);
	iterateSenors(MySensorAction::PRESENTATION);
}

void rp_setup() {
	iterateSenors(MySensorAction::SETUP);
}

void rp_loop() {
	rp_now = millis() + rp_add_sleep_time;
	if(rp_first_loop) {
		rp_report();
		iterateSenors(MySensorAction::REPORT);
		iterateSenors(MySensorAction::LOOP_FIRST);
	}
	//Serial.print("Now: ");
	//Serial.print(rp_now);
	//Serial.print(", 1s: ");
	//Serial.println(rp_1s_time);
	if(rp_1s_time + 1000 < rp_now) {
		rp_1s_tick = 1;
		rp_1s_time = rp_now;
		iterateSenors(MySensorAction::LOOP_1S);	
	}

	iterateSenors(MySensorAction::LOOP);	
}

inline uint32_t calcTimestamp(char u, byte v) {
			uint32_t multiple = u=='S'?1:(u=='M'?60:(u=='H'?60UL*60:(u=='D'?24UL*60*60:1)));
			return multiple * v * ((u=='U')?1:1000);
}

void rp_loop_end() {

	if(rp_now > lastPing + calcTimestamp((char)pingIntervalUnit, pingIntervalValue)) {
		lastPing = rp_now;

		iterateSenors(MySensorAction::PING);	// PING
	}
	
	iterateSenors(MySensorAction::LOOP_END);

	rp_first_loop = 0;
	rp_1s_tick = 0;
}

const char cHello[] PROGMEM  =  {"HELLO"};
const char cHelp1[] PROGMEM  =  {"C,R,E,I{9},P{9},D{9m}"};
const char cHelp2[] PROGMEM  =  {"PING:{U|S|M|H|D}{9}"};
const char cHelp3[] PROGMEM  =  {"A{0|1},{id}"};

void rp_receive(const MyMessage &message) {
	/*
	Serial.print(F("SensorId:"));
	Serial.print(message.sensor);
	Serial.print(F(", VType:"));
	Serial.print(message.type);
	Serial.print(F(", Cmd:"));
	if(message.getCommand() == C_SET || message.getCommand() == C_REQ) {
		Serial.print(message.getCommand()==1?"C_SET ":"C_REQ ");
	}
	Serial.print(message.getCommand());
	Serial.print(F(", Data: "));
	Serial.println(message.data);
	*/
	//rp_addToBuffer("VType:");
	//rp_addToBuffer(message.type);
	//rp_reportBuffer();
	
	if(message.sensor == RP_ID_CUSTOM) {
		const char* data = message.data;
		char c = data[0];
		//byte b = atoi(&data[1]);
		if( mystrncmp(cPing, data, 5)) {
			pingIntervalUnit = data[5];
			pingIntervalValue = atoi(&data[6]);
			saveState(EE_PING_UNIT, pingIntervalUnit);
			saveState(EE_PING_TIME, pingIntervalValue);
			rp_report();
	  }	else if (c=='H') {
			myresend(_msgv.set(myF(cHello)));
			myresend(_msgv.set(myF(cHelp1)));
			myresend(_msgv.set(myF(cHelp2)));
			myresend(_msgv.set(myF(cHelp3)));
			iterateSenors(MySensorAction::HELP);
		}
		if (c=='C') {
			rp_report();
			iterateSenors(MySensorAction::REPORT);
		}
		else if (c=='R') {
		  rp_reset();
		}
		else if(c=='E') {
			for (int i = EE_RP_OFFSET ; i < EE_RP_OFFSET+200 /*EE_MAX_OFFSET*/ ; i++) {
				saveState(i, 255);
			}
			rp_reset();
		}
		else if(c=='I') {
			hwWriteConfig(EEPROM_NODE_ID_ADDRESS, atoi(&data[1]));
		}
		else if(c=='P') {
			hwWriteConfig(EEPROM_PARENT_NODE_ID_ADDRESS, atoi(&data[1]));
		}
		else if(c=='F') {
			myresend(_msgv.set(hwCPUFrequency()));
		}
		else if(c=='V') {
			myresend(_msgv.set(hwCPUVoltage()));
		}
		else if(c=='M') {
			myresend(_msgv.set(hwFreeMem()));
		}
		else if(c=='A') {
			// A[enable],[id]
			byte id = atoi(&data[3]); 
			byte enable = atoi(&data[1]);
			rp_addToBuffer("A:");
			rp_addToBuffer(enable);
			rp_addToBuffer(",");
			rp_addToBuffer(id);
			rp_reportBuffer();
			enableSensor(id, !enable);
		}
		else if(c=='D') {
			if(data[1]!='\0') {
				rp_force_time = atoi(&data[1]);
				saveState(EE_FORCE_REPORT_TIME_OFFSET, rp_force_time);
			}
			rp_report();
		} 
	}
	//rp_addToBuffer(message.data);
	//rp_reportBuffer();

	iterateReceiveSenors(message);
}

void rp_reset() {
	// Software reboot with watchdog timer.
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	// Reset enable
	WDTCSR= (1<<WDE);
	//WDTCSR  = (1<<WDE) | 1<<WDIE | (0<<WDP3)|(1<<WDP2) | (1<<WDP1);	//1s
	//WDTCSR  = (1<<WDE) | 1<<WDIE | (1<<WDP3)|(0<<WDP2) | (0<<WDP1) | (0<<WDP0);	//4s
	//Watchdog reset after 16 ms
	while(true){}
}

RpSensor::RpSensor(bool enabled) {
			Disabled = !enabled;
			Ping = 0;
			eeOffset = EE_RP_SENSORS_OFFSET;
			if(rp_sensors_count > 0) {
				eeOffset += _rpsensors[rp_sensors_count - 1]->_eeLength;
			}

			EEReadByte(eeOffset + 0, &Disabled);
			eeOffset++;

			_rpsensors[rp_sensors_count] = this;
			rp_sensors_count++;
			MaxIds = 1;
			_eeLength = 0;

			/*strcpy(rp_buffer, __PRETTY_FUNCTION__);

			char* p = rp_buffer;//[] = {__PRETTY_FUNCTION__};//const char DSPinSet[] PROGMEM  = {"DS pin set"};

			while(*(++p)!=':') {}; *p='\0';
			Serial.println(rp_buffer);*/
}

void RpSensor::loop() {}

void RpSensor::processReceive(const MyMessage &message) {

	receiveAll(message);

	if ((message.type==V_VAR2) || (message.sensor == RP_ID_CUSTOM)){
		receiveCommon(message);
	}

	if (message.sensor >= Id && message.sensor <= Id + MaxIds - 1){

		if(strlen(message.data)>0 ) {
			receive(message);
		}  else {
			if(message.getCommand() == C_REQ) {
				receiveCReq(message);
			}
		}
	}
}

void RpSensor::presentation() {
	present(Id, SensorType);
}

void RpSensor::report() {
	rp_addPToBuffer(cActive);
	rp_addToBuffer(!Disabled);
	rp_addToBuffer(',');
	rp_addToBuffer(Id);
	rp_reportBuffer();
}

/*void RpSensor::_onInterrupt_1() {

      Serial.print(F("INT P="));
      Serial.println(digitalRead(2)>0?1:0);
}*/

