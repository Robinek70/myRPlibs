#ifndef RPRELAY_h
#define RPRELAY_h

#include <Arduino.h>

class RpRelay : public RpSensor {
	public:
	  RpRelay(byte pin, bool invertActive = 0);
	  void setup();
	  void loop();
	  void receiveCReq(const MyMessage &message);
	  void receive(const MyMessage &message);
	  bool getState();
	private:
	  byte _pin;
	  byte _prev_door;
	  bool _invert;
};

#endif