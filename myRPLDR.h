#ifndef RPLDR_h
#define RPLDR_h

#include <Arduino.h>

void rp_ldr_config(byte pin, byte id);
inline int sensorLDRToLux(int adc);
void reportLDR(int adcValue);
void rp_ldr_presentation();
void rp_ldr_loop() ;
void rp_ldr_loop_1s_tick();

#endif