/*****************************************
* This is a library for the ADXL345 accelerometer.
*
* You'll find an example which should enable you to use the library. 
*
* You are free to use it, change it or build on it. In case you like 
* it, it would be cool if you give it a star.
* 
* If you find bugs, please inform me!
* 
* Written by Wolfgang (Wolle) Ewald
* https://wolles-elektronikkiste.de
*
*******************************************/

#include "ADXL345_WE.h"

/************  Constructors ************/

ADXL345_WE::ADXL345_WE(int addr){
	i2cAddress = addr;
}

ADXL345_WE::ADXL345_WE(){
	i2cAddress = 0x53;
}

/************ Basic settings ************/
	
bool ADXL345_WE::init(){	
	writeRegister(ADXL345_POWER_CTL,0);
	if(!setMeasureMode(true)){
		return false;
	}
	corrFact.x = 1.0;
	corrFact.y = 1.0;
	corrFact.z = 1.0;
	offsetVal.x = 0.0;
	offsetVal.y = 0.0;
	offsetVal.z = 0.0;
	setRange(ADXL345_RANGE_2G);
	writeRegister(ADXL345_DATA_FORMAT,0);
	setFullRes(true);
	writeRegister(ADXL345_INT_ENABLE, 0);
	writeRegister(ADXL345_INT_MAP,0);
	writeRegister(ADXL345_TIME_INACT, 0);
	writeRegister(ADXL345_THRESH_INACT,0);
	writeRegister(ADXL345_ACT_INACT_CTL, 0);
	writeRegister(ADXL345_DUR,0);
	writeRegister(ADXL345_LATENT,0);
	writeRegister(ADXL345_THRESH_TAP,0);
	writeRegister(ADXL345_TAP_AXES,0);
	writeRegister(ADXL345_WINDOW, 0);
	readAndClearInterrupts();
	writeRegister(ADXL345_FIFO_CTL,0);
	writeRegister(ADXL345_FIFO_STATUS,0);
	
	return true;
}

void ADXL345_WE::setCorrFactors(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax){
	corrFact.x = UNITS_PER_G / (0.5 * (xMax - xMin));
	corrFact.y = UNITS_PER_G / (0.5 * (yMax - yMin));
	corrFact.z = UNITS_PER_G / (0.5 * (zMax - zMin));
	offsetVal.x = (xMax + xMin) * 0.5;
	offsetVal.y = (yMax + yMin) * 0.5;
	offsetVal.z = (zMax + zMin) * 0.5;
}

void ADXL345_WE::setDataRate(adxl345_dataRate rate){
	regVal |= readRegister8(ADXL345_BW_RATE);
	regVal &= 0xF0;
	regVal |= rate;
	writeRegister(ADXL345_BW_RATE, regVal);
}
	
adxl345_dataRate ADXL345_WE::getDataRate(){
	return (adxl345_dataRate)(readRegister8(ADXL345_BW_RATE) & 0x0F);
}

String ADXL345_WE::getDataRateAsString(){
	adxl345_dataRate dataRate = (adxl345_dataRate)(readRegister8(ADXL345_BW_RATE) & 0x0F);
	String returnString = "";
	
	switch(dataRate) {
		case ADXL345_DATA_RATE_3200: returnString = "3200 Hz"; break;
		case ADXL345_DATA_RATE_1600: returnString = "1600 Hz"; break;
		case ADXL345_DATA_RATE_800:  returnString = "800 Hz";  break;
		case ADXL345_DATA_RATE_400:  returnString = "400 Hz";  break;
		case ADXL345_DATA_RATE_200:  returnString = "200 Hz";  break;
		case ADXL345_DATA_RATE_100:  returnString = "100 Hz";  break;
		case ADXL345_DATA_RATE_50:   returnString = "50 Hz";   break;
		case ADXL345_DATA_RATE_25:   returnString = "25 Hz";   break;
		case ADXL345_DATA_RATE_12_5: returnString = "12.5 Hz"; break;
		case ADXL345_DATA_RATE_6_25: returnString = "6.25 Hz"; break;
		case ADXL345_DATA_RATE_3_13: returnString = "3.13 Hz"; break;
		case ADXL345_DATA_RATE_1_56: returnString = "1.56 Hz"; break;
		case ADXL345_DATA_RATE_0_78: returnString = "0.78 Hz"; break;
		case ADXL345_DATA_RATE_0_39: returnString = "0.39 Hz"; break;
		case ADXL345_DATA_RATE_0_20: returnString = "0.20 Hz"; break;
		case ADXL345_DATA_RATE_0_10: returnString = "0.10 Hz"; break;
	}
	
	return returnString;
}

uint8_t ADXL345_WE::getPowerCtlReg(){
	return readRegister8(ADXL345_POWER_CTL);
}

void ADXL345_WE::setRange(adxl345_range range){
	uint8_t regVal = readRegister8(ADXL345_DATA_FORMAT);
	if(adxl345_lowRes){
		switch(range){
			case ADXL345_RANGE_2G: 	rangeFactor = 1.0; 	break;
			case ADXL345_RANGE_4G: 	rangeFactor = 2.0; 	break;
			case ADXL345_RANGE_8G: 	rangeFactor = 4.0; 	break;
			case ADXL345_RANGE_16G: rangeFactor = 8.0; 	break;	
		}
	}
	else{
		rangeFactor = 1.0;
	}
	regVal &= 0b11111100;
	regVal |= range;
	writeRegister(ADXL345_DATA_FORMAT, regVal);
}

adxl345_range ADXL345_WE::getRange(){
	regVal = readRegister8(ADXL345_DATA_FORMAT);
	regVal &= 0x03; 
	return adxl345_range(regVal);
}

void ADXL345_WE::setFullRes(boolean full){
	regVal = readRegister8(ADXL345_DATA_FORMAT);
	if(full){
		adxl345_lowRes = false;
		rangeFactor = 1.0;
		regVal |= (1<<ADXL345_FULL_RES);
	}
	else{
		adxl345_lowRes = true;
		regVal &= ~(1<<ADXL345_FULL_RES);
		setRange(getRange());
	}
	writeRegister(ADXL345_DATA_FORMAT, regVal);
}

String ADXL345_WE::getRangeAsString(){
	String rangeAsString = "";
	adxl345_range range = getRange();
	switch(range){
		case ADXL345_RANGE_2G: 	rangeAsString = "2g"; 	break;
		case ADXL345_RANGE_4G:	rangeAsString = "4g"; 	break;
		case ADXL345_RANGE_8G: 	rangeAsString = "8g"; 	break;
		case ADXL345_RANGE_16G: rangeAsString = "16g"; 	break;
		
	}
	return rangeAsString;
}

/************ x,y,z results ************/

xyzFloat ADXL345_WE::getRawValues(){
	rawVal.x = readRegister16(ADXL345_DATAX0);
	rawVal.y = readRegister16(ADXL345_DATAY0);
	rawVal.z = readRegister16(ADXL345_DATAZ0);
	return rawVal;
}

xyzFloat ADXL345_WE::getCorrectedRawValues(){
	rawVal.x = readRegister16(ADXL345_DATAX0) - (offsetVal.x / rangeFactor);
	rawVal.y = readRegister16(ADXL345_DATAY0) - (offsetVal.y / rangeFactor);
	rawVal.z = readRegister16(ADXL345_DATAZ0) - (offsetVal.z / rangeFactor);
	return rawVal;
}

xyzFloat ADXL345_WE::getGValues(){
	getCorrectedRawValues();
	gVal.x = rawVal.x * MILLI_G_PER_LSB * rangeFactor * corrFact.x / 1000.0;
	gVal.y = rawVal.y * MILLI_G_PER_LSB * rangeFactor * corrFact.y / 1000.0;
	gVal.z = rawVal.z * MILLI_G_PER_LSB * rangeFactor * corrFact.z / 1000.0;
	return gVal;
}

xyzFloat ADXL345_WE::getAngles(){
	getGValues();
	if(gVal.x > 1){
		gVal.x = 1;
	}
	else if(gVal.x < -1){
		gVal.x = -1;
	}
	angleVal.x = (asin(gVal.x)) * 57.296;
	
	if(gVal.y > 1){
		gVal.y = 1;
	}
	else if(gVal.y < -1){
		gVal.y = -1;
	}
	angleVal.y = (asin(gVal.y)) * 57.296;
	
	if(gVal.z > 1){
		gVal.z = 1;
	}
	else if(gVal.z < -1){
		gVal.z = -1;
	}
	angleVal.z = (asin(gVal.z)) * 57.296;
	
	return angleVal;
}

xyzFloat ADXL345_WE::getTilts(){
	getAngles();
	xyzFloat tiltVal;
	tiltVal.x = angleVal.x - angleOffsetVal.x;
	tiltVal.y = angleVal.y - angleOffsetVal.y;
	tiltVal.z = angleVal.z - angleOffsetVal.z;
		
	return tiltVal;
}

/************ Angles and Orientation ************/ 

void ADXL345_WE::measureAngleOffsets(){
	getAngles();
	angleOffsetVal.x = angleVal.x; 
	angleOffsetVal.y = angleVal.y;
	angleOffsetVal.z = angleVal.z;
}

adxl345_orientation ADXL345_WE::getOrientation(){
	adxl345_orientation orientation = FLAT;
	getAngles();
	if(abs(angleVal.x) < 45){      // |x| < 45
    if(abs(angleVal.y) < 45){      // |y| < 45
      if(angleVal.z > 0){          //  z  > 0
        orientation = FLAT;
      }
      else{                        //  z  < 0
        orientation = FLAT_1;
      }
    }
    else{                         // |y| > 45 
      if(angleVal.y > 0){         //  y  > 0
        orientation = XY;
      }
      else{                       //  y  < 0
        orientation = XY_1;   
      }
    }
  }
  else{                           // |x| >= 45
    if(angleVal.x > 0){           //  x  >  0
      orientation = YX;       
      }
      else{                       //  x  <  0
        orientation = YX_1;
      }
  }
	return orientation;
}

String ADXL345_WE::getOrientationAsString(){
	adxl345_orientation orientation = getOrientation();
	String orientationAsString = "";
	switch(orientation){
		case FLAT: 		orientationAsString = "z up"; 	break;
		case FLAT_1:	orientationAsString = "z down";	break;
		case XY:	 	orientationAsString = "y up"; 	break;
		case XY_1: 		orientationAsString = "y down"; 	break;
		case YX:		orientationAsString = "x up"; 	break;
		case YX_1:		orientationAsString = "x down"; 	break;
	}
	return orientationAsString;
}

/************ Power, Sleep, Standby ************/ 

bool ADXL345_WE::setMeasureMode(boolean measure){
	regVal = readRegister8(ADXL345_POWER_CTL);
	if(measure){
		regVal |= (1<<ADXL345_MEASURE);
	}
	else{
		regVal &= ~(1<<ADXL345_MEASURE);
	}
	uint8_t ack = writeRegister(ADXL345_POWER_CTL, regVal);
	
	return (ack == 0);
}

void ADXL345_WE::setSleep(bool sleep, adxl345_wUpFreq freq){
	regVal = readRegister8(ADXL345_POWER_CTL);
	regVal &= 0b11111100;
	regVal |= freq;
	if(sleep){
		regVal |= (1<<ADXL345_SLEEP);
	}
	else{
		setMeasureMode(false);  // it is recommended to enter Stand Mode when clearing the Sleep Bit!
		regVal &= ~(1<<ADXL345_SLEEP);
		regVal &= ~(1<<ADXL345_MEASURE);
	}
	writeRegister(ADXL345_POWER_CTL, regVal);
	if(!sleep){
		setMeasureMode(true);
	}
}

void ADXL345_WE::setSleep(bool sleep){
	regVal = readRegister8(ADXL345_POWER_CTL);
	if(sleep){
		regVal |= (1<<ADXL345_SLEEP);
	}
	else{
		setMeasureMode(false);  // it is recommended to enter Stand Mode when clearing the Sleep Bit!
		regVal &= ~(1<<ADXL345_SLEEP);
		regVal &= ~(1<<ADXL345_MEASURE);
	}
	writeRegister(ADXL345_POWER_CTL, regVal);
	if(!sleep){
		setMeasureMode(true);
	}
}
	

void ADXL345_WE::setAutoSleep(bool autoSleep){
	if(autoSleep){
		setLinkBit(true);
		regVal = readRegister8(ADXL345_POWER_CTL);
		regVal |= (1<<ADXL345_AUTO_SLEEP);
		writeRegister(ADXL345_POWER_CTL, regVal);
	}
	else{
		regVal = readRegister8(ADXL345_POWER_CTL);
		regVal &= ~(1<<ADXL345_AUTO_SLEEP);
		writeRegister(ADXL345_POWER_CTL, regVal);
	}
		
}

bool ADXL345_WE::isAsleep(){
	return readRegister8(ADXL345_ACT_TAP_STATUS) & (1<<ADXL345_ASLEEP);
}

void ADXL345_WE::setLowPower(bool lowpwr){
	regVal = readRegister8(ADXL345_BW_RATE);
	if(lowpwr){
		regVal |= (1<<ADXL345_LOW_POWER);
	}
	else{
		regVal &= ~(1<<ADXL345_LOW_POWER);
	}
	writeRegister(ADXL345_BW_RATE, regVal);
}
			
/************ Interrupts ************/


void ADXL345_WE::setInterrupt(adxl345_int type, uint8_t pin){
	regVal = readRegister8(ADXL345_INT_ENABLE);
	regVal |= (1<<type);
	writeRegister(ADXL345_INT_ENABLE, regVal);
	regVal = readRegister8(ADXL345_INT_MAP);
	if(pin == INT_PIN_1){
		regVal &= ~(1<<type);
	}
	else {
		regVal |= (1<<type);
	}
	writeRegister(ADXL345_INT_MAP, regVal);
}

void ADXL345_WE::setInterruptPolarity(uint8_t pol){
	regVal = readRegister8(ADXL345_DATA_FORMAT);
	if(pol == ADXL345_ACT_HIGH){
		regVal &= ~(0b00100000);
	}
	else if(pol == ADXL345_ACT_LOW){
		regVal |= 0b00100000;
	}
};

void ADXL345_WE::deleteInterrupt(adxl345_int type){
	regVal = readRegister8(ADXL345_INT_ENABLE);
	regVal &= ~(1<<type);
	writeRegister(ADXL345_INT_ENABLE, regVal);	
}

uint8_t ADXL345_WE::readAndClearInterrupts(){
	regVal = readRegister8(ADXL345_INT_SOURCE);
	return regVal;
}

bool ADXL345_WE::checkInterrupt(uint8_t source, adxl345_int type){
	source &= (1<<type);
	return source;
}
void ADXL345_WE::setLinkBit(bool link){
	regVal = readRegister8(ADXL345_POWER_CTL);
	if(link){
		regVal |= (1<<ADXL345_LINK);
	}
	else{
		regVal &= ~(1<<ADXL345_LINK);
	}
	writeRegister(ADXL345_POWER_CTL, regVal);
}

void ADXL345_WE::setFreeFallThresholds(float ffg, float fft){
	regVal = (uint8_t)(round(ffg / 0.0625));
	if(regVal<1){
		regVal = 1;
	}
	writeRegister(ADXL345_THRESH_FF, regVal);
	regVal = (uint8_t)(round(fft / 5));
	if(regVal<1){
		regVal = 1;
	}
	writeRegister(ADXL345_TIME_FF, regVal);
}

void ADXL345_WE::setActivityParameters(adxl345_dcAcMode mode, adxl345_actTapSet axes, float threshold){
	regVal = (uint8_t)(round(threshold / 0.0625));
	if(regVal<1){
		regVal = 1;
	}
	
	writeRegister(ADXL345_THRESH_ACT, regVal);

	regVal = readRegister8(ADXL345_ACT_INACT_CTL);
	regVal &= 0x0F;
	regVal |= ((uint8_t)mode + uint8_t(axes))<<4;
	writeRegister(ADXL345_ACT_INACT_CTL, regVal);
}

void ADXL345_WE::setInactivityParameters(adxl345_dcAcMode mode, adxl345_actTapSet axes, float threshold, uint8_t inactTime){
	regVal = (uint8_t)(round(threshold / 0.0625));
	if(regVal<1){
		regVal = 1;
	}
	writeRegister(ADXL345_THRESH_INACT, regVal);

	regVal = readRegister8(ADXL345_ACT_INACT_CTL);
	regVal &= 0xF0;
	regVal |= (uint8_t)mode + uint8_t(axes);
	writeRegister(ADXL345_ACT_INACT_CTL, regVal);

	writeRegister(ADXL345_TIME_INACT, inactTime);
}

void ADXL345_WE::setGeneralTapParameters(adxl345_actTapSet axes, float threshold, float duration, float latent){
	regVal = readRegister8(ADXL345_TAP_AXES);
	regVal &= 0b11111000;
	regVal |= uint8_t(axes);
	writeRegister(ADXL345_TAP_AXES, regVal);
	
	regVal = (uint8_t)(round(threshold / 0.0625));
	if(regVal<1){
		regVal = 1;
	}
	writeRegister(ADXL345_THRESH_TAP,regVal);
	
	regVal = (uint8_t)(round(duration / 0.625));
	if(regVal<1){
		regVal = 1;
	}
	writeRegister(ADXL345_DUR, regVal);
	
	regVal = (uint8_t)(round(latent / 1.25));
	if(regVal<1){
		regVal = 1;
	}
	writeRegister(ADXL345_LATENT, regVal);		
}

void ADXL345_WE::setAdditionalDoubleTapParameters(bool suppress, float window){
	regVal = readRegister8(ADXL345_TAP_AXES);
	if(suppress){
		regVal |= (1<<ADXL345_SUPPRESS);
	}
	else{
		regVal &= ~(1<<ADXL345_SUPPRESS);
	}
	writeRegister(ADXL345_TAP_AXES, regVal);
	
	regVal = (uint8_t)(round(window / 1.25));
	writeRegister(ADXL345_WINDOW, regVal);
}

uint8_t ADXL345_WE::getActTapStatus(){
	return readRegister8(ADXL345_ACT_TAP_STATUS);
}

String ADXL345_WE::getActTapStatusAsString(){
	uint8_t mask = (readRegister8(ADXL345_ACT_INACT_CTL)) & 0b01110000;
	mask |= ((readRegister8(ADXL345_TAP_AXES)) & 0b00000111);
		
	String returnStr = "";
	regVal = readRegister8(ADXL345_ACT_TAP_STATUS);	
	regVal &= mask;
		
	if(regVal & (1<<ADXL345_TAP_Z)) { returnStr += "TAP-Z "; }
	if(regVal & (1<<ADXL345_TAP_Y)) { returnStr += "TAP-Y "; }
	if(regVal & (1<<ADXL345_TAP_X)) { returnStr += "TAP-X "; }
	if(regVal & (1<<ADXL345_ACT_Z)) { returnStr += "ACT-Z "; }
	if(regVal & (1<<ADXL345_ACT_Y)) { returnStr += "ACT-Y "; }
	if(regVal & (1<<ADXL345_ACT_X)) { returnStr += "ACT-X "; }
	
	return returnStr;
}

/************ FIFO ************/

void ADXL345_WE::setFifoParameters(adxl345_triggerInt intNumber, uint8_t samples){
	regVal = 0;
	regVal |= (samples-1);
	if(intNumber == ADXL345_TRIGGER_INT_2){
		regVal |= 0x20;
	}
	writeRegister(ADXL345_FIFO_CTL, regVal);
}

void ADXL345_WE::setFifoMode(adxl345_fifoMode mode){
	regVal = readRegister8(ADXL345_FIFO_CTL);
	regVal &= 0b00111111;
	regVal |= (mode<<6);
	writeRegister(ADXL345_FIFO_CTL,regVal);
}

uint8_t ADXL345_WE::getFifoStatus(){
	return readRegister8(ADXL345_FIFO_STATUS);
}

void ADXL345_WE::resetTrigger(){
	setFifoMode(ADXL345_BYPASS);
    setFifoMode(ADXL345_TRIGGER);
}


/************************************************ 
	private functions
*************************************************/

uint8_t ADXL345_WE::writeRegister(uint8_t reg, uint8_t val){
  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.write(val);
  
  return Wire.endTransmission();
}
  
uint8_t ADXL345_WE::readRegister8(uint8_t reg){
  uint8_t regValue = 0;
  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress,1);
  if(Wire.available()){
    regValue = Wire.read();
  }
  return regValue;
}


int16_t ADXL345_WE::readRegister16(uint8_t reg){
  uint8_t MSByte = 0, LSByte = 0;
  int16_t regValue = 0;
  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress,2);
  if(Wire.available()){
    LSByte = Wire.read();
    MSByte = Wire.read();
  }
  regValue = (MSByte<<8) + LSByte;
  return regValue;
}





