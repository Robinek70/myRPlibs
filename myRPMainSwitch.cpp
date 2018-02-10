#include "myRPlibs.h"
#include "myRPMainSwitch.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

static MyMessage swMsg(RP_ID_RELAY, V_LIGHT);


//#define LIGHT_PORT	PORTD
static volatile byte count = 0;
//static volatile int countint = 0;

static void myint0();

static byte minWaves = 10;	// EE
static byte MaxLights = 1;	// EE
static byte RevTriack = 1;	// EE

struct LIGHTHW {
	volatile uint8_t* rejestr;
	volatile uint8_t* port;
	byte  pin;

	//volatile uint8_t* line_rejestr;
	volatile uint8_t* line_port;
	byte line_pin;

	byte savedLevel;
	byte invertSwitch;	// EE x2

	byte current;
	byte targetLevel;

	byte lineState;
	byte prevLineState;	

	byte switchPosition;	// 1 - on; 0 - off
	byte switchStatus;	
	

	byte prevImpState;
	byte prevImpStateCounter;
	byte currentImpState;

	//SAVEDDATA data;
};

LIGHTHW /*PROGMEM*/ lights_hw[] = {
  {&DDRD, &PORTD, (1<<PD4), /*&DDRD,*/ &PIND, (1<<PD7) /*D7*/, 100, 0, 100, 100},
  {&DDRD, &PORTD, (1<<PD5), /*&DDRD,*/ &PINB, (1<<PB0) /*D8*/, 100, 0, 100, 100},
};

#define MAXLIGHTS (sizeof(lights_hw)/sizeof(LIGHTHW))


RpMainSwitch::RpMainSwitch() 
	: RpSensor() {
	//_pin = pin;
	Id = getFreeIdBinary();
	SensorType = S_BINARY;
	SensorData = V_STATUS;
	Ping = 1;
	pinMode(INT_NO + 2, INPUT); 

	_eeLength = 5;

	EEReadByte(eeOffset + EE_SWITCHES, &MaxLights);
	EEReadByte(eeOffset + EE_TRIACK_MODE, &RevTriack);
	EEReadByte(eeOffset + EE_MINWAVES, &minWaves);
	for(byte i = 0;i<MaxLights;i++) {		
		EEReadByte(eeOffset + EE_SW_INVERT + i, &(lights_hw[i].invertSwitch));
	}

	MaxIds = MaxLights;

	TIMER2_STOP;

	  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));  // normal overflow
	  TCCR2A &= ~((1<<COM2B0) | (1<<COM2B1));

	  TIMER2_START;
	  TCNT2 = COUNTDOWN_TIMER;

	  TIMSK2 |= (1 << TOIE2); // overflow timer2 interupt enable

	  attachInterrupt (INT_NO, myint0, CHANGE);
	  sei();
	Serial.println("MainSwitch pin set");

}

static void myint0()
{
	TIMER2_STOP;
	TCNT2 = COUNTDOWN_TIMER;
	TIMER2_START;
	count=0;
	//countint++;
	for(byte i = 0;i<MaxLights;i++) {
		byte pin = lights_hw[i].pin;

		lights_hw[i].current = lights_hw[i].targetLevel;
		
		byte newValue = lights_hw[i].current;
		
		if(RevTriack) {  // TRIACKS
			if(newValue == MIN_VALUE) {
				*lights_hw[i].port &= ~(pin); // Low - On
			} else {  // Turn On immediately if 100%
				*lights_hw[i].port |= (pin); // High - Off
			}
		} else {	// RELAY
			if(newValue == MIN_VALUE) {
				*lights_hw[i].port |= (pin); // High - On
			} else {  // Turn On immediately if 100%				
				*lights_hw[i].port &= ~(pin); // Low - Off
			}
		}
	}
}

ISR(TIMER2_OVF_vect)          // timer compare interrupt service routine
{
	TCNT2 = COUNTDOWN_TIMER;//255-200+1;
	count++;
	//countint++;

	for(byte i=0; i<MaxLights;i++){ 
		/*byte pin = lights_hw[i].pin;
		if(count > 40) { // don't off, required for Low Power LEDs, impuls should be longer than one tick
			LIGHT_PORT |= pin;		// High - Off
		}
		if(count > 92) // proper values are 0 - 96
			return;

		if(count == lights_hw[i].current) {  // wlacz	
			LIGHT_PORT &= ~pin;	// Low - On
		}*/
		
		// line detection
		if(count == 50) {
			if(!(PIND & (1<<PD3))) {  // first half wave int1
				lights_hw[i].currentImpState = *lights_hw[i].line_port & lights_hw[i].line_pin; 

				if(lights_hw[i].prevImpState == lights_hw[i].currentImpState) {
					if(lights_hw[i].prevImpStateCounter < minWaves) {
						lights_hw[i].prevImpStateCounter++;
					} else {
						lights_hw[i].lineState = lights_hw[i].currentImpState;
					}
				} else {
					lights_hw[i].prevImpStateCounter = 0;
					lights_hw[i].prevImpState = lights_hw[i].currentImpState;
				}
			}
		}
	}
}

void RpMainSwitch::setup() {
	Serial.println("Sending request...");
	for(byte i=0;i<MaxLights;i++) {
		request(Id + i, SensorData);
		wait(200);
	}
}

void RpMainSwitch::loop() {
	//if(count==0) {
		//myresend( _msgv.set("Tick count0") );
	//}
	//myresend( _msgv.set(count) );
	checkSwitch();
}

void RpMainSwitch::loop_1s_tick() {
	//myresend( _msgv.set(countint) );
	//countint=0;
	/**rp_buffer='\0';
	
	rp_addToBuffer("pin:");
	//rp_addToBuffer(PIND & (1<<PD7));
	rp_addToBuffer(*lights_hw[0].line_port & lights_hw[0].line_pin);
	rp_addToBuffer(", ls:");
	rp_addToBuffer(lights_hw[0].lineState);
	rp_addToBuffer(", cls:");
	rp_addToBuffer(lights_hw[0].currentImpState);
	rp_addToBuffer(" ,W:");
	rp_addToBuffer(PIND & (1<<PD3));
	rp_reportBuffer();*/
}

const char cMinWaves[] PROGMEM  =  {"MinWaves (w):"};
const char cLights[] PROGMEM  =  {"Switches (s):"};
const char cTriackMode[] PROGMEM  =  {"Triack (t):"};

void RpMainSwitch::report() {
	rp_addPToBuffer(cLights);
	rp_addToBuffer(MaxLights);
	rp_reportBuffer();

	rp_addPToBuffer(cMinWaves);
	rp_addToBuffer(minWaves);
	rp_reportBuffer();

	rp_addPToBuffer(cTriackMode);
	rp_addToBuffer(RevTriack);
	rp_reportBuffer();
}

void RpMainSwitch::receiveCommon(const MyMessage &message){
	const char* s = &message.data[0];
	if( *s== 'w') {
		minWaves = atoi( &s[1] );
		saveState(eeOffset + EE_MINWAVES, minWaves);
	}else if( *s== 's') {
		MaxLights = MIN(atoi( &s[1] ), 2);
		saveState(eeOffset + EE_SWITCHES, MaxLights);
		rp_reset();
	} else if( *s== 't') {
		RevTriack = atoi( &s[1] );
		saveState(eeOffset + EE_TRIACK_MODE, RevTriack);
		rp_reset();
	}
}

void RpMainSwitch::receive(const MyMessage &message){

	byte idx  = message.sensor - Id;
	if (message.type==V_STATUS) {
		int onOff = atoi( message.data );
		myresend( _msgv.setSensor(message.sensor).set(onOff?"light ON":"light OFF") );
		if(!switchTo(idx, onOff)){
			return;
		}
	}
}

void RpMainSwitch::receiveCReq(const MyMessage &message){
	myresend(swMsg.setSensor(Id).set(getState(message.sensor - Id)));
}

bool RpMainSwitch::getState(byte idx){
	return (bool)lights_hw[idx].switchPosition;
}

byte RpMainSwitch::switchPosition(struct LIGHTHW* light) {
	if(light->invertSwitch) {
		return light->lineState?0:1;
	}

	return light->lineState?1:0;	  
}

void RpMainSwitch::storeSwitchPosition(byte idx) {
	saveState(eeOffset + EE_SW_INVERT + idx,lights_hw[idx].invertSwitch);
	
}

boolean RpMainSwitch::switchTo(byte idx, byte onOff) {
	/* *rp_buffer='\0';
	rp_addToBuffer("SwPos:");
	rp_addToBuffer(switchPosition(&lights_hw[idx]));
	rp_addToBuffer(", inv:");
	rp_addToBuffer(lights_hw[idx].invertSwitch);
	rp_addToBuffer(", ls:");
	rp_addToBuffer(lights_hw[idx].lineState);
	rp_reportBuffer();*/
	  if(onOff == lights_hw[idx].switchPosition){
		return false;
	  }

	  lights_hw[idx].invertSwitch = !lights_hw[idx].invertSwitch;

	  /*rp_addToBuffer("changed to:");
		rp_addToBuffer(onOff);
		rp_reportBuffer();*/

	  storeSwitchPosition(idx);

	  return true;
}


void RpMainSwitch::checkSwitch() {
	for(byte i=0;i<MaxLights;i++) {
		LIGHTHW* light = &lights_hw[i];
		byte switch_status = light->switchStatus;

		// final light switch position; 1 - on; 0 - off
		light->switchPosition = switchPosition(light);

		if(light->prevLineState != light->lineState) {
			light->prevLineState = light->lineState;

			myresend( _msgv.set(light->lineState?"line ON":"line OFF") );
		}

		if(light->switchPosition) {
			// switch PRESSED/ACTIVE
			if(switch_status == SWITCH_OFF) {
				switch_status = SWITCH_ON;

				if((isLIGHTFULL) || (light->targetLevel == MAX_VALUE)) {
					light->targetLevel = MIN_VALUE;	// 100% light value
				}
				else {
					light->targetLevel = light->savedLevel;
				}
				myresend(swMsg.setSensor(Id + i).set(1));
				myresend( _msgv.set("SET ON") );
			} 
		} else {
			// switch NOT pressed/active
			if(switch_status == SWITCH_ON) {
				switch_status = SWITCH_OFF;
				
				light->savedLevel = light->targetLevel; 
				// zapisaæ w eeprom jesli wartosc sie zmienila
				//if(loadState(EE_SW_DATA + EE_SW_SAVED + i*EE_SW_DATA_SIZE) != light->targetLevel /*&& !setByRemote*/){
				  
				  //saveState(EE_SW_DATA + EE_SW_SAVED + i*EE_SW_DATA_SIZE,light->savedLevel);
			    //}

				light->targetLevel = MAX_VALUE;
				myresend(swMsg.setSensor(Id + i).set(0));
				myresend( _msgv.set("SET OFF") );
			}
		}

		light->switchStatus = switch_status;
	}	
}