#include <Arduino.h> 
#include <myRPlibs.h>
#include <../MySensors/core/MySensorsCore.h>

#ifndef RP_MAX_REPEATS
	#define RP_MAX_REPEATS	5
#endif

MyMessage _msgv(RP_ID_CUSTOM, V_VAR2);
uint32_t rp_now;
uint32_t rp_1s_time;
byte rp_force_time = RP_FORCE_TIME_DEFAULT;
bool rp_first_loop = 1;
bool rp_1s_tick = 0;
byte rp_sensors_count = 0;
RpSensor* _rpsensors[RP_MAX_SENSORS];


char rp_buffer[25];

void EEPROMWriteInt(int p_address, int p_value)
     {
     byte lowByte = ((p_value >> 0) & 0xFF);
     byte highByte = ((p_value >> 8) & 0xFF);

	 saveState(p_address, lowByte);
     saveState(p_address + 1, highByte);
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

void myresend(MyMessage &msg)
{
	byte repeat = 1;
	boolean sendOK = false;

	while ((sendOK == false) && (repeat < RP_MAX_REPEATS)) {
		sendOK = send(msg);
		if(!sendOK) {
			repeat++; 
		} else {
			break;
		}	
		wait(20*(1<<repeat));
	}	
}

void rp_addToBuffer(const char *s) {
	byte len = strlen(rp_buffer);
	strcpy(rp_buffer+len, s);
}
void rp_addToBuffer(int v) {
	byte len = strlen(rp_buffer);
	itoa(v,rp_buffer+len,10);
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

char* myF(const char* s) {
  byte len;
  /*if(!clear) {
	len = strlen(rp_buffer);
  }*/
  char *p= rp_buffer;//+len;
  
  len = strlen_P(s);
  byte k;
  for (k = 0; k < len; k++)
  {
    char myChar =  pgm_read_byte_near(s + k);
	p[k]=myChar;
  }
  p[k]='\0';
  return rp_buffer;
}

void rp_reportBuffer() {
	Serial.println(rp_buffer);				
	myresend(_msgv.set(rp_buffer));
	*rp_buffer='\0';
}

void rp_report() {
	rp_addToBuffer("ForceReportTime[m]: ");
	rp_addToBuffer(rp_force_time);

	rp_reportBuffer();
}

enum MySensorAction {
	PRESENTATION = 1,
	LOOP = 2,
	LOOP_FIRST = 3,
	LOOP_1S = 4
};
void iterateSenors(MySensorAction action) {
	for(byte i=0;i<rp_sensors_count;i++) {
		RpSensor* s =  _rpsensors[i];
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
		}
	}
}

void iterateReceiveSenors(const MyMessage &message) {
	for(byte i=0;i<rp_sensors_count;i++) {
		RpSensor* s =  _rpsensors[i];
		s->receive(message);
	}
}

void rp_before() {
	EEReadByte(EE_FORCE_REPORT_TIME_OFFSET, &rp_force_time);
	//Serial.print("ForceTime: ");
	//Serial.println(rp_force_time);	
}

void rp_presentation() {
	present(RP_ID_CUSTOM, S_CUSTOM);
	iterateSenors(MySensorAction::PRESENTATION);
}

void rp_loop() {
	rp_now = millis();
	if(rp_first_loop) {
		rp_report();
		iterateSenors(MySensorAction::LOOP_FIRST);
	}
	if(rp_1s_time + 1000 < rp_now) {
		rp_1s_tick = 1;
		rp_1s_time = rp_now;
		iterateSenors(MySensorAction::LOOP_1S);	
	}

	iterateSenors(MySensorAction::LOOP);	
}
void rp_loop_end() {
	rp_first_loop = 0;
	rp_1s_tick = 0;
}

void rp_receive(const MyMessage &message) {
	
	if(message.sensor == RP_ID_CUSTOM) {
		const char* data = message.data;
		char c = data[0];
		//byte b = atoi(&data[1]);
		if (c=='H') {
			myresend(_msgv.set("HELLO"));
			myresend(_msgv.set("R,E,I{9},P{9},D{9m}"));
		}
		else if (c=='R') {
		  //myresend(_msgv.set(data));
		  
		  rp_reset();
		}
		else if(c=='E') {
			for (int i = EE_RP_OFFSET ; i < EE_MAX_OFFSET ; i++) {
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
		else if(c=='D') {
			if(data[1]!='\0') {
				rp_force_time = atoi(&data[1]);
				saveState(EE_FORCE_REPORT_TIME_OFFSET, rp_force_time);
			}
			rp_report();
		}		
	}

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

RpSensor::RpSensor() {
			_rpsensors[rp_sensors_count] = this;
			rp_sensors_count++;
			Serial.print("RpSensor: ");
			Serial.println(rp_sensors_count);

			/*strcpy(rp_buffer, __PRETTY_FUNCTION__);

			char* p = rp_buffer;//[] = {__PRETTY_FUNCTION__};//const char DSPinSet[] PROGMEM  = {"DS pin set"};

			while(*(++p)!=':') {}; *p='\0';
			Serial.println(rp_buffer);*/
}
void RpSensor::receive(const MyMessage &message) {
		  //Serial.println("RpSensor receive");
}

void RpSensor::loop() {}
