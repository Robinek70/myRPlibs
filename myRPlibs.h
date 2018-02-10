/**
 * @file RPlibs.h
 *
 * RPlibs main interface (includes all necessary code for the library)
 */

// pressure measurement 
// https://cdn.sparkfun.com/assets/3/3/2/0/4/527d393b757b7f68548b456f.jpg
// https://learn.sparkfun.com/tutorials/bmp180-barometric-pressure-sensor-hookup-
/*
double SFE_BMP180::sealevel(double P, double A)
// Given a pressure P (mb) taken at a specific altitude (meters),
// return the equivalent pressure (mb) at sea level.
// This produces pressure readings that can be used for weather measurements.
{
	return(P/pow(1-(A/44330.0),5.255));
}

double SFE_BMP180::altitude(double P, double P0)
// Given a pressure measurement P (mb) and the pressure at a baseline P0 (mb),
// return altitude (meters) above baseline.
{
	return(44330.0*(1-pow(P/P0,1/5.255)));
}


*/
// CircularBuffer.h // avg
// E:\MojeDokumenty\Arduino\libraries\SparkFun_BME280_Arduino_Library\examples\I2C_DeltaAltitude



#ifndef RPlibs_h
#define RPlibs_h

#ifdef __cplusplus
#include <Arduino.h>
#endif

#define RP_DEBUG			1

#define RP_ID_TMPHUM		0	
#define RP_ID_HUM			1
#define RP_ID_PRESSURE		2
#define RP_ID_LIGHT_SENSOR	5
#define RP_ID_LIGHT_PCT		6
//#define RP_ID_DOOR			20
#define RP_ID_PIR			20
#define RP_ID_TEMP			30

#define RP_ID_RELAY			50
#define RP_ID_DOOR			80

#define RP_ID_POWER			100	// W,khW Counter 100,101

#define RP_ID_CUSTOM		199

//#define MAX_ATTACHED_DS18B20	4

//#define EE_TEMP_NAMES_LENGTH	15


#define EE_RP_OFFSET				0
#define EE_TEMP_MAP_OFFSET1			EE_RP_OFFSET														//!< MAX_ATTACHED_DS18B20 bytes, map
#define EE_TEMP_NAMES_OFFSET1		(EE_TEMP_MAP_OFFSET1+4)							//!< MAX_ATTACHED_DS18B20*EE_TEMP_NAMES_LENGTH bytes, names of sensors
#define EE_FORCE_REPORT_TIME_OFFSET	(EE_TEMP_NAMES_OFFSET1 + 4*15)	//!< 1 byte, max gap between reports from sensor [min]
#define EE_MOTION_DELAY_OFFSET1		(EE_FORCE_REPORT_TIME_OFFSET + 1)										//!< 2 byte, max gap between reports from sensor [s]
#define EE_LUX_MARGIN				(EE_MOTION_DELAY_OFFSET1 + 2)										// 1 byte lux margin [%] to report
#define EE_EMPTY					(EE_LUX_MARGIN + 1)							


#define EE_CFG1						EE_EMPTY
#define EE_CFG2						(EE_CFG1+1)

#define EE_PING_UNIT				(EE_CFG2 + 1)		// 1 byte
#define EE_PING_TIME				(EE_PING_UNIT + 1)	// 1 byte


#define EE_RP_SENSORS_OFFSET		50

#define EE_MAX_OFFSET				(EE_PING_TIME+1)

#define RP_FORCE_TIME_DEFAULT	(30)		// [min] max gap bettwen reports from sensor, 
#define RP_MAX_SENSORS			10

#define RP_SLEEP_MODE_NONE	0
#define RP_SLEEP_MODE_SLEEP	1
#define RP_SLEEP_MODE_WAIT	2

//#define MYF(x)	(myF(PSTR(x)))

//#define MY_NODE_ID	1
#include <../MySensors/core/MySensorsCore.h>
#include <../MySensors/hal/architecture/MyHwAVR.h>
//#include <core/MyCapabilities.h>
//#include <core/MyTransport.h>
//#include <core/Version.h>
#include <stdint.h>

extern uint32_t rp_now;
extern uint32_t rp_add_sleep_time;
extern byte rp_sleepMode;
extern byte rp_force_time;
extern char rp_buffer[25];
extern MyMessage _msgv;
extern bool rp_first_loop;
extern byte rp_sensors_count;
class RpSensor;
extern RpSensor* _rpsensors[RP_MAX_SENSORS];
//extern byte id_binary;


class RpSensor {
	public:
	  RpSensor(bool enabled = 1);
	  void processReceive(const MyMessage &message);
	  virtual void receive(const MyMessage &message) {};
	  virtual void receiveCReq(const MyMessage &message) {};
	  virtual void receiveCommon(const MyMessage &message) {};
	  virtual void receiveAll(const MyMessage &message) {};
	  virtual void before() {};
	  virtual void setup() {};
	  virtual void loop();
	  virtual void loop_first() {};
	  virtual void loop_1s_tick() {};
	  virtual void loop_end() {};
	  virtual void presentation();
	  virtual void help() {};
	  virtual void report() {};
	  byte SensorType;
	  byte SensorData;
	  byte Id;
	  byte Disabled;
	  byte Ping;
	  byte MaxIds;
	  byte eeOffset;
	  static void _onInterrupt_1();
	//private:
protected:
	  byte _eeLength;
		
};

void myresend(MyMessage &msg);

void EEPROMWriteInt(int p_address, int p_value);
unsigned int EEPROMReadInt(int p_address);
void EEReadInt(int pos, int* data);
void EEReadByte(int pos, byte* data);

void rp_presentation();
void rp_receive(const MyMessage &message);

void rp_before();
void rp_setup();
void rp_loop();
void rp_loop_end();
void rp_reset();
void rp_addToBuffer(const char* s) ;
char* rp_addPToBuffer(const char* s);
void rp_addToBuffer(int v);
void rp_addToBuffer(uint32_t v);
void rp_addToBuffer(int32_t v);
void rp_addToBuffer(float v, unsigned char decimals);
void rp_addToBuffer(char c);
void rp_addFromEeprom(int start, byte len);
char* myF(const char* s);
void rp_reportBuffer();

#define myF(x)	(rp_addPToBuffer(x))
byte mystrncmp(const char* flash, const char * s, byte count);

byte getFreeIdBinary() ;



#endif
