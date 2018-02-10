#ifndef RPPIR_h
#define RPPIR_h

#include <Arduino.h>

class RpPir : public RpSensor {
	public:
	  RpPir(byte pin, bool enabled = 1);
	  void receive(const MyMessage &message);
	  void loop();
	  void loop_end();	  
	  void help();
	  RpPir* setDelay(int delay);
	
	private:
	  byte _pin;  
	  byte _prev_pir;
	  uint32_t _lastSendPir;
	  void report();
	  void update();
};

#endif