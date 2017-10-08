#include "myRPlibs.h"
#include "myRPDS18b20.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>


#define RP_DEBUG

static MyMessage tempMsg(RP_ID_TEMP, V_TEMP);
static byte id_ds = RP_ID_TEMP;

// Temp variables
static OneWire* oneWire;//(ONE_WIRE_BUS);

static float lastTemperature[MAX_ATTACHED_DS18B20];
static float avgTemp[MAX_ATTACHED_DS18B20];
static uint32_t lastTempSend[MAX_ATTACHED_DS18B20];
static byte prevSkippedTemp[MAX_ATTACHED_DS18B20];
static byte dtt = 4;

//const char DSPinSet[] PROGMEM  = {"DS pin set"};


RpDs18b20::RpDs18b20(byte pin) 
	: RpSensor() {
	_pin = pin;
	
	Serial.println("DS pin set");

	oneWire = new OneWire(_pin);
	_sensors = new DallasTemperature(oneWire);
	_sensors->begin();
	_sensors->setWaitForConversion(false);
	_numSensors = _sensors->getDeviceCount();
	_sensors->requestTemperatures();
	
	id_ds += _numSensors;	// increace for next pir sensor

	for (byte i=0; i<MAX_ATTACHED_DS18B20; i++) {      
		_mapTempId[i] = RP_ID_TEMP+i;
		byte id = loadState(EE_TEMP_MAP_OFFSET + i);
		if(id!=0 && id !=255) {
			_mapTempId[i]=id;
		}
	}
	_lastMeasureTime = 0;
}
void RpDs18b20::receive(const MyMessage &message){
	RpSensor::receive(message);

	char *p = (char *)message.data;
	if(message.sensor == RP_ID_CUSTOM) {
		if(*p=='H') {
			myresend(_msgv.set("T[{M|N}{no,id}"));
		}
	}
	if ((message.type==V_VAR2) || (message.sensor == RP_ID_CUSTOM)){

		if(*p=='T') {			
			char a = *(++p);
			if(*(p+1)!='\0'){
				byte sensor_no = atoi(p+1);
				byte i;
				byte ok;
				byte id;
				while(*(p++)!=',') {
					//p++;
				}
				//p++;
				if(a=='M') {
					id = atoi(p );
					//Serial.print(" ID: ");
					//Serial.print(id);
					_mapTempId[sensor_no] = id;
					saveState(EE_TEMP_MAP_OFFSET + sensor_no, id);
				
					//Serial.print(", No: ");
					//Serial.print(sensor_no);
				} 
				else if(a=='N') {
					//p++;
					i=0;
					ok = 1;
					//Serial.print(':');
					while(ok && i<EE_TEMP_NAMES_LENGTH) {
						saveState(EE_TEMP_NAMES_OFFSET + sensor_no*EE_TEMP_NAMES_LENGTH + i, *(p+i)); 
						if(*(p+i)) {
							//Serial.print( *(p+i));
							i++;
						} else {
							ok = 0;
						}					
					}
				}
			}

			report();
		}
	}
}
void RpDs18b20::loop() {
}

void RpDs18b20::loop_first() {
	report();
}

void RpDs18b20::presentation() {
	for (byte i=0; i<_numSensors && i<MAX_ATTACHED_DS18B20; i++) {   
		rp_addFromEeprom(EE_TEMP_NAMES_OFFSET + i*EE_TEMP_NAMES_LENGTH, EE_TEMP_NAMES_LENGTH);
		present(_mapTempId[i], S_TEMP, rp_buffer);
		*rp_buffer='\0';
	}
	_isMetric = getControllerConfig().isMetric;
}

void RpDs18b20::report() {
	rp_addToBuffer("Temps: ");
	rp_addToBuffer(_numSensors);
	rp_reportBuffer();
	for(byte i=0;i<MAX_ATTACHED_DS18B20;i++) {
				rp_addToBuffer(i);
				rp_addToBuffer(" -> ");
				rp_addToBuffer(_mapTempId[i]);
				rp_addToBuffer(" - ");

				rp_addFromEeprom(EE_TEMP_NAMES_OFFSET + i*EE_TEMP_NAMES_LENGTH, EE_TEMP_NAMES_LENGTH);

				rp_reportBuffer();
			}
}

void RpDs18b20::loop_1s_tick(){
	for (byte i=0; i<_numSensors && i<MAX_ATTACHED_DS18B20; i++) {
		bool tempOvertime = ((rp_now - lastTempSend[i]) > 60*1000UL*rp_force_time);
		// Fetch and round temperature to one decimal
		float temperature = static_cast<float>(static_cast<int>((_isMetric?_sensors->getTempCByIndex(i):_sensors->getTempFByIndex(i)) * 10.)) / 10.;		

		// Only send data if temperature has changed and no error
		if (temperature != -127.00 && temperature != 85.00) {

			if(avgTemp[i]==0) {
				avgTemp[i]=temperature;
			}
			avgTemp[i] = (avgTemp[i]*dtt + temperature)/(dtt + 1);

			if(round(avgTemp[i]*10) != round(lastTemperature[i]*10) || tempOvertime) {

				if(prevSkippedTemp[i]>2 || (abs(lastTemperature[i] - avgTemp[i])>0.19) || tempOvertime) {
					// Send in the new temperature
					myresend(tempMsg.setSensor(/*CHILD_ID_TEMP+i*/ _mapTempId[i]).set(avgTemp[i],1));
					//if(temp_mode == numSensors - 1) {
						// update send time after last sensor 
						lastTempSend[i] = rp_now;
					//}
					// Save new temperatures for next compare
					lastTemperature[i] = avgTemp[i];
					prevSkippedTemp[i]=0;
				} else {
					prevSkippedTemp[i]++;
				}
			}
		}
	}
	// Fetch temperatures from Dallas sensors
	_sensors->requestTemperatures();
	//temp_mode++;
}