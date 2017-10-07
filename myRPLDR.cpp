#include "myRPlibs.h"
#include "myRPLDR.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

#define L_a_	-1.6477
#define L_b_	7.873

// log10(1) = a*log10(60) + b, log10(60) => 1.7782
// log10(60) = A*log10(5k) + b, log10(5k) => 3.699

#define RP_LDR_V_REF	5.	// reference V
#define RP_LDR_VCC		5.	// Vcc
#define RP_LDR_R1		30*1000	// R1 - PullUp value

#define RP_LDR_R(_adc_)	(RP_LDR_R1/(RP_LDR_VCC/(adc *(RP_LDR_V_REF/1023.)) - 1))

inline int sensorLDRToLux(int adc) {
	//return (2500/((value*0.0048828125)-500))/10; 

	/*float a = -1.6477;	// wspolczyniki log10(lux)= a*log10(R)+b
	float b = 7.873;
	float v = 5;
	float v0 = adc *(5./1023.); // 0.0048828125;	
	float R1 = 30*1000;*/

	//float R = R1/(v/v0 - 1);
	//float R = R1/(v/v0 - 1);

	int lightLevel = pow(10,L_a_*log10(RP_LDR_R(adc))+L_b_);
	//int lightLevel = pow(10,a*log10(R)+b);
	//int lightLevel = 7.46449E7/pow(R, 1.6477);
	return lightLevel;
}

// LUX
static byte dtLux = 100;
static uint32_t luxValue = 0;
static int prevLuxValue;
static uint32_t ldrLastSend;
static MyMessage luxMsg1(5, V_LIGHT_LEVEL);
static byte pin_ldr;
static byte id_ldr;

void rp_ldr_config(byte pin, byte id) {
	pin_ldr = pin;
	id_ldr = id;
	digitalWrite(pin, HIGH);	// pull up analog pin
}

inline void reportLDR(int adcValue) {
	int vlux = sensorLDRToLux(adcValue);
	if((vlux != prevLuxValue) || ((rp_now - ldrLastSend) > 60*1000UL*rp_force_time)) {
		myresend(luxMsg1.set(vlux));
		prevLuxValue = vlux;	
		ldrLastSend = rp_now;
	}
}

void rp_ldr_presentation() {
	present(id_ldr, S_LIGHT_LEVEL);
}

void rp_ldr_loop() {
	int sensor = analogRead(pin_ldr);
	luxValue = (luxValue*dtLux + sensor*10) / (dtLux + 1);
}

void rp_ldr_loop_1s_tick(){
	reportLDR(luxValue/10);
}