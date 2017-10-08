#ifndef RPCONTACT_h
#define RPCONTACT_h

#include <Arduino.h>

class RpContact : public RpSensor {
	public:
	  RpContact(byte pin);
	  //void receive(const MyMessage &message);
	  void loop();
	  void loop_1s_tick();
	  void presentation();
	private:
	  byte _pin;
	  byte _id;
	  uint32_t _luxValue;
	  int _prevLuxValue;
	  uint32_t _lastSend;
	  byte _prev_door;
};

#endif