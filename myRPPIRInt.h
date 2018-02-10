#ifndef RPPIRINT_h
#define RPPIRINT_h

#include <Arduino.h>

class RpPirInt : public RpSensor {
	public:
	  RpPirInt(byte pin, bool enabled = 1);

	  void setup();
	  void loop();
	  void loop_end();
	
	private:
	  byte _pin;  
	  byte _prev_pir;
	  void update();
};

#endif