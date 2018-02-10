#ifndef RPLDR_h
#define RPLDR_h

#include <Arduino.h>

class RpLdr : public RpSensor {
	public:
	  RpLdr(byte pin);
	  //void receive(const MyMessage &message);
	  void loop();
	  void loop_first();
	  void loop_1s_tick();
	  //void presentation();
	  void receiveCommon(const MyMessage &message);
	  void help();
	private:
	  byte _pin;
	  //byte _id;
	  uint32_t _luxValue;
	  int _prevLuxValue;
	  uint32_t _lastSend;
	  byte _luxMargin;		// difference to report [%]
	  void report();
};

#endif