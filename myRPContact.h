#ifndef RPCONTACT_h
#define RPCONTACT_h

#include <Arduino.h>

class RpContact : public RpSensor {
	public:
	  RpContact(byte pin);
	  void loop();

	  void receiveCReq(const MyMessage &message);
	private:
	  byte _pin;
	  uint32_t _luxValue;
	  int _prevLuxValue;
	  uint32_t _lastSend;
	  byte _prev_door;
};

#endif