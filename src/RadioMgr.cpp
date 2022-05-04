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

 std::string  RadioMgr::hertz_to_string(double hz, int precision){
	
	char buffer[128] = {0};
 
	if(hz >= 1.615e6) { // Mhz
		sprintf(buffer, "%3.*f", precision, hz/1.0e6);
	} else if(hz >= 1.0e3) {
		sprintf(buffer, "%4d", (int)round( hz/1.0e3));
	}
	
	return string(buffer);
}
