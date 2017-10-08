#ifndef RPDS_h
#define RPDS_h

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

class RpDs18b20 : public RpSensor {
	public:
	  RpDs18b20(byte pin);
	  void receive(const MyMessage &message);
	  void loop();
	  void loop_first();
	  void presentation();
	  void loop_1s_tick();
	private:
	  byte _pin;
	  byte _id;	  
	  bool _isMetric;
	  byte _numSensors;
	  uint32_t _lastMeasureTime;
	  DallasTemperature* _sensors;//(&oneWire);
	  byte _mapTempId[MAX_ATTACHED_DS18B20];

	  void report();	  
};

#endif