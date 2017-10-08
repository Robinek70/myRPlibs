#ifndef RPPIR_h
#define RPPIR_h

#include <Arduino.h>

class RpPir : public RpSensor {
	public:
	  RpPir(byte pin);
	  void receive(const MyMessage &message);
	  void loop();
	  void loop_first();
	  void presentation();
	private:
	  byte _pin;
	  byte _id;
	  byte _prev_pir;
	  uint32_t _lastSendPir;
};

#endif