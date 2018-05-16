#include "myRPlibs.h"
#include "myRPSi7021.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

//static MyMessage tempMsg(0, V_TEMP);
//static MyMessage humMsg(0+1, V_HUM);
static MyMessage rpMsg(0, V_HUM);

RpDsSi7021::RpDsSi7021() 
	: RpSensor() {
	Id = 0;

	tBuffer = new CircularBuffer(TBUFSIZE);
	hBuffer = new CircularBuffer(HBUFSIZE);
	Serial.println(F("Si starting..."));

	siSensor.begin();

	_lastMeasureTime = 0;
	MaxIds = 2;
	//prevHum = prevTemp = 0;
}

void RpDsSi7021::receiveCReq(const MyMessage &message){
	//myresend(tempMsg.setSensor(message.sensor).set(avgTemp[message.sensor-Id],1));
	reportData(true);
}

void RpDsSi7021::presentation() {
	present(Id, S_TEMP);
	present(Id+1, S_HUM);
	_isMetric = getControllerConfig().isMetric;
}
/*
void RpDsSi7021::help() {
	//myresend(_msgv.set("T[{M|N}{no,id}"));
}

void RpDsSi7021::report() {

}*/

void RpDsSi7021::loop_first() {
	reportData(true);
}

void RpDsSi7021::loop_1s_tick(){
	reportData(false);
}

void RpDsSi7021::reportData(bool forceReport) {
	float humidity, humidity1, temp, temp1;
	getSiWeather(&humidity1, &temp1);
	tBuffer->pushElement(temp1);
	hBuffer->pushElement(humidity1);

	temp = ((int)(tBuffer->averageLast(TBUFSIZE)*10))/10.;
	//humidity = ((int)(hBuffer->averageLast(HBUFSIZE)*10))/10.;
	humidity = (int)(hBuffer->averageLast(HBUFSIZE)+.5);

/*	Serial.print("Temp:");
  Serial.print(temp);
  Serial.print("C, ");

  Serial.print("Humidity:");
  Serial.print(humidity);
  Serial.println("%");

*/

	/*if(prevTemp != temp || forceReport) {
		myresend(tempMsg.set(temp,1));
		prevTemp = temp;
	}
	if(prevHum != humidity || forceReport) {
		myresend(humMsg.set(humidity,1));
		prevHum = humidity;
	}*/

	if(prevHum != humidity || prevTemp != temp || forceReport) {
		myresend(rpMsg.setSensor(Id).setType(V_TEMP).set(temp,1));
		prevTemp = temp;	
		//myresend(humMsg.set(humidity,0));
		myresend(rpMsg.setSensor(Id+1).setType(V_HUM).set(humidity,0));
		prevHum = humidity;
	}
}

void RpDsSi7021::getSiWeather(float* humidity, float* temp)
{

  // Measure Relative Humidity from the HTU21D or Si7021
  //*humidity = ((int)(siSensor.getRH()*10))/10.;
  *humidity = siSensor.getRH();

  // Measure Temperature from the HTU21D or Si7021
  //*temp = ((int)(siSensor.getTemp()*10))/10.;
  *temp = _isMetric?siSensor.getTemp():siSensor.getTempF();
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement with getTemp() instead with readTemp()
  //printInfo();
}