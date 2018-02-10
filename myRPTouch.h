#ifndef RPTOUCH_h
#define RPTOUCH_h

#include <Arduino.h>
#include <CapacitiveSensor.h>

class RpTouch : public RpSensor {
	public:
	  RpTouch(byte pin, byte commonPin);
	  void setup();
	  void loop();
	  void receiveCReq(const MyMessage &message);
	  void receive(const MyMessage &message);
	  bool getState();
	  void report();
	  void help();
	private:
	  byte _pin;
	  byte _commonPin;
	  CapacitiveSensor*   ts; 
	  byte _prevState;
	  byte firstOn;
	  uint32_t start1;
	  int _threshold;
	  long _avg;
};

#endif