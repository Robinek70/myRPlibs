#ifndef RPDS_h
#define RPDS_h

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS	A5

#define CHILD_ID_TEMP	30

//#define EE_MAP_OFFSET	0

//static bool Metric = true;
static MyMessage tempMsg(CHILD_ID_TEMP, V_TEMP);

//extern OneWire oneWire;

//void rp_ds_id(byte id);
//void rp_ds_pin(byte pin);
//void rp_ds_before(byte pin = ONE_WIRE_BUS, byte id  = CHILD_ID_TEMP);
//void rp_ds_presentation();
//void rp_ds_loop();
//void rp_ds_loop_1s_tick();
//void rp_ds_receive(const MyMessage &message);
//void rp_ds_report();

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