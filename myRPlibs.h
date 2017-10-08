/**
 * @file RPlibs.h
 *
 * RPlibs main interface (includes all necessary code for the library)
 */

#ifndef RPlibs_h
#define RPlibs_h

#ifdef __cplusplus
#include <Arduino.h>
#endif


#define RP_ID_LIGHT_SENSOR	5
#define RP_ID_DOOR			20
#define RP_ID_PIR			21
#define RP_ID_TEMP			30
#define RP_ID_CUSTOM		199

#define MAX_ATTACHED_DS18B20	4

#define EE_TEMP_NAMES_LENGTH	15


#define EE_RP_OFFSET				0
#define EE_TEMP_MAP_OFFSET			EE_RP_OFFSET														//!< MAX_ATTACHED_DS18B20 bytes, map
#define EE_TEMP_NAMES_OFFSET		(EE_TEMP_MAP_OFFSET+MAX_ATTACHED_DS18B20)							//!< MAX_ATTACHED_DS18B20*EE_TEMP_NAMES_LENGTH bytes, names of sensors
#define EE_FORCE_REPORT_TIME_OFFSET	(EE_TEMP_NAMES_OFFSET + MAX_ATTACHED_DS18B20*EE_TEMP_NAMES_LENGTH)	//!< 1 byte, max gap between reports from sensor [min]
#define EE_MOTION_DELAY_OFFSET		(EE_FORCE_REPORT_TIME_OFFSET + 1)										//!< 2 byte, max gap between reports from sensor [s]
#define EE_EMPTY					(EE_MOTION_DELAY_OFFSET + 2)							

#define EE_MAX_OFFSET				EE_EMPTY

#define RP_FORCE_TIME_DEFAULT	(30)		// [min] max gap bettwen reports from sensor, 
#define RP_MAX_SENSORS			10

//#define MYF(x)	(myF(PSTR(x)))

//#define MY_NODE_ID	1
#include <../MySensors/core/MySensorsCore.h>
#include <../MySensors/hal/architecture/MyHwAVR.h>
//#include <core/MyCapabilities.h>
//#include <core/MyTransport.h>
//#include <core/Version.h>
#include <stdint.h>

extern uint32_t rp_now;
extern byte rp_force_time;
extern char rp_buffer[25];
extern MyMessage _msgv;
extern bool rp_first_loop;
extern byte rp_sensors_count;
class RpSensor;
extern RpSensor* _rpsensors[RP_MAX_SENSORS];


class RpSensor {
	public:
	  RpSensor();
	  virtual void receive(const MyMessage &message);
	  virtual void loop();
	  virtual void loop_first() {};
	  virtual void loop_1s_tick() {};
	  virtual void presentation() {};
	//private:
};

void myresend(MyMessage &msg);

void EEPROMWriteInt(int p_address, int p_value);
unsigned int EEPROMReadInt(int p_address);
void EEReadInt(int pos, int* data);
void EEReadByte(int pos, byte* data);

void rp_presentation();
void rp_receive(const MyMessage &message);

void rp_before();
void rp_loop();
void rp_loop_end();
void rp_reset();
void rp_addToBuffer(const char *s) ;
void rp_addToBuffer(int v);
void rp_addToBuffer(char c);
void rp_addFromEeprom(int start, byte len);
char* myF(const char* s);
void rp_reportBuffer();



#endif
