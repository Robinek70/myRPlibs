#ifndef RPEMONI_h
#define RPEMONI_h

#include <Arduino.h>
//#include "CircularBuffer.h"
#include "EmonLib.h"

class RpEmonI : public RpSensor {
	public:
	  RpEmonI(byte pin);
	  void setup();
	  void presentation();
	  void loop_1s_tick();
	  void receiveCReq(const MyMessage &message);
	  void receive(const MyMessage &message);

	  RpEmonI* setVoltage(int v);
	  RpEmonI* setCurrentCalibration(float iCal, float iOffset);
	  RpEmonI* setNumSamples(int numSamples);
	  void setTimer(int8_t delay);
	private:
	  byte _pin;
	  EnergyMonitor _emon1;
	  int _numSamples;
	  float _iCal;
	  float _iOffset;
	  int _voltage;
	  int _prevP;
	  uint32_t _lastKwhTime;
	  uint32_t _kwhCount;
	  uint32_t _localKwhCount;
	  byte _idKwh;
	  bool _ready;
	  //CircularBuffer* myBuffer;
	  int8_t _countdownTimer;
	  uint32_t _sum;
	  int8_t _sumCount;
};

#endif