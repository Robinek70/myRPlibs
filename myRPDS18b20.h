#ifndef RPDS_h
#define RPDS_h

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define MAX_ATTACHED_DS18B20	4

#define EE_TEMP_NAMES_LENGTH	15

#define EE_TEMP_NAMES_OFFSET		(MAX_ATTACHED_DS18B20)							//!< MAX_ATTACHED_DS18B20*EE_TEMP_NAMES_LENGTH bytes, names of sensors

class RpDs18b20 : public RpSensor {
	public:
	  RpDs18b20(byte pin);
	  void receiveCommon(const MyMessage &message);
	  void receiveCReq(const MyMessage &message);
	  void loop();
	  void presentation();
	  void loop_1s_tick();
	  void help();
	private:
	  byte _pin;
	  //byte _id;	  
	  bool _isMetric;
	  byte _numSensors;
	  uint32_t _lastMeasureTime;
	  DallasTemperature* _sensors;//(&oneWire);
	  byte _mapTempId[MAX_ATTACHED_DS18B20];

	  void report();	  
};

#endif