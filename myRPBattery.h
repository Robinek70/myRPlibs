#ifndef RPBATTERY_h
#define RPBATTERY_h

#include <Arduino.h>
#include <../MySensors/core/MyTransport.h>

class RpBattery : public RpSensor {
	public:
	  RpBattery(byte adcPin, uint8_t refType, uint16_t refV);

	  void before();
	  void loop_1s_tick();
	  void loop_end();
	  void loop_first();
	  void receiveCReq(const MyMessage &message);
	  void receive(const MyMessage &message);
	  void receiveCommon(const MyMessage &message);
	  RpBattery* setDivider(float r1, float r2);
	  RpBattery* setBattery(float minBat, float maxBat);
	  RpBattery* wakeUpPin(byte pinI, byte mode);
	  RpBattery* sleepTime(uint32_t sleepTime);
	  void presentation();
	  void help();
	  void report();
	  
	private:
	  byte _pin, _pinI1, _pinI2;
	  byte _modeI1, _modeI2;
	  float _r1;
	  float _r2;
	  float _minBat, _maxBat;
	  byte _prevProcBat;
	  uint16_t _vccRef;
	  uint32_t _sleepTime;
	  uint32_t _lastBatReport;

	  float readBattery();
	  void updateBatteryLevel(bool forceReport);
	  void signalReport();
};

#endif