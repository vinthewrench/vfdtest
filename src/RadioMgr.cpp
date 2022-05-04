//
//  RadioMgr.cpp
//  vfdtest
//
//  Created by Vincent Moscaritolo on 5/4/22.
//

#include "RadioMgr.hpp"
#include <cmath>

RadioMgr *RadioMgr::sharedInstance = NULL;

RadioMgr::RadioMgr(){
	_mode = RADIO_OFF;
	
 }


RadioMgr::~RadioMgr(){
 }

 
bool RadioMgr::setRadioMode(radio_mode_t newMode){
	_mode = newMode;
	return true;
}

bool RadioMgr::setFrequency(double newFreq){
	_frequency = newFreq;
	return true;
}
 


std::string  RadioMgr::freqSuffixString(double hz){
	
	if(hz >= 1.615e6) { // Mhz
		return "Mhz";
	} else if(hz >= 1.0e3) {
		return "Khz";
	}
	
	return "";
}

double RadioMgr::nextFrequency(bool up){
	
	double newfreq = _frequency;
	
	switch (_mode) {
		case BROADCAST_AM:
			// AM steps are 10khz
			if(up) {
				newfreq+=10.e3;
			}
			else {
				newfreq+=10.e3;
 			}
			if(newfreq > 1605e3) newfreq =1605e3;
			else if(newfreq < 535e3) newfreq =535e3;
		break;
	
		case BROADCAST_FM:
			// AM steps are 200khz
			if(up) {
				newfreq+=200.e3;
			}
			else {
				newfreq+=200.e3;
			}
			if(newfreq > 108.e6) newfreq = 108.e6;
			else if(newfreq < 88.1e6) newfreq =88.1e6;
		break;

		default:
			break;
	}
	return newfreq;
}

 std::string  RadioMgr::hertz_to_string(double hz, int precision){
	
	char buffer[128] = {0};
 
	if(hz >= 1.615e6) { // Mhz
		sprintf(buffer, "%3.*f", precision, hz/1.0e6);
	} else if(hz >= 1.0e3) {
		sprintf(buffer, "%4d", (int)round( hz/1.0e3));
	}
	
	return string(buffer);
}
