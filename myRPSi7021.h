#ifndef RPSI7021_h
#define RPSI7021_h

#include <Arduino.h>
#include "CircularBuffer.h"
#include "SparkFun_Si7021_Breakout_Library.h"

#define TBUFSIZE	1
#define HBUFSIZE	1

class RpDsSi7021 : public RpSensor {
	public:
	  RpDsSi7021();
	  //void receiveCommon(const MyMessage &message);
	  void receiveCReq(const MyMessage &message);
	  void loop_first();
	  void presentation();
	  void loop_1s_tick();
	  //void help();
	  //void report();
	private:  
	  float prevTemp, prevHum;
	  bool _isMetric;
	  Weather siSensor;
	  uint32_t _lastMeasureTime;
	  CircularBuffer* tBuffer;
	  CircularBuffer* hBuffer;	
	  void getSiWeather(float* humanity, float* temp);
	  void reportData(bool forceReport);
};

#endif