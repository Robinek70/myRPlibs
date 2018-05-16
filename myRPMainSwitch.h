#ifndef RPMAINSWITCH_h
#define RPMAINSWITCH_h

#include <Arduino.h>

#define TIMER2_START TCCR2B |= (1<<CS21);// | (1<<CS20); 
#define TIMER2_STOP TCCR2B &= ~((1<<CS22) |(1<<CS21) | (1<<CS20)); 

#define INT_NO	1

//F_CPU == 16000000
// 8MHz
#define COUNTDOWN_TIMER  255-100+1

// 16MHz
//#define COUNTDOWN_TIMER 255 - 200 + 1

#define SWITCH_OFF	0
#define SWITCH_ON	1

#define MIN_VALUE   0
#define MAX_VALUE   100

#define EE_SWITCHES		0
#define EE_MINWAVES		1
#define EE_TRIACK_MODE	2
#define EE_SW_INVERT	3	// 2  bytes, for 2 lights
#define EE_PIR_MODE		5	// PIR zamiast switch on input

#define isLIGHTFULL		1
//#define isLIGHTFULL		(!(cfg & CFG_DIMMER_ON))



class RpMainSwitch : public RpSensor {
	public:
	  RpMainSwitch();
	  void setup();
	  void loop();
	  void loop_1s_tick();
	  void receiveCReq(const MyMessage &message);
	  void receive(const MyMessage &message);
	  void receiveCommon(const MyMessage &message);
	  bool getState(byte idx);
	  void report();
	  void presentation();
	private:
	  byte IdPir;
	  void checkSwitch();
	  byte switchPosition(struct LIGHTHW* light);
	  boolean switchTo(byte idx, byte onOff);
	  void storeSwitchPosition(byte idx);
};

#endif