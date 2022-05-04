//
//  main.cpp
//  vfdtest
//
//  Created by Vincent Moscaritolo on 4/13/22.
//


#include <stdio.h>
#include <stdlib.h>   // exit()

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <ctime>
#include <cmath>
#include <iomanip>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <filesystem> // C++17
#include <fstream>
#include "CommonDefs.hpp"


#include "DisplayMgr.hpp"
#include "RadioMgr.hpp"
#include "TMP117.hpp"
#include "QwiicTwist.hpp"

bool getCPUTemp(float & tempOut) {
	bool didSucceed = false;
	
	try{
		std::ifstream   ifs;
		ifs.open("/sys/class/thermal/thermal_zone0/temp", ios::in);
		if( ifs.is_open()){
			
			if(tempOut){
				string val;
				ifs >> val;
				ifs.close();
				float temp = std::stof(val);
				temp = temp /1000.0;
				tempOut = temp;
			}
			didSucceed = true;
		}
		
	}
	catch(std::ifstream::failure &err) {
	}
	
	
	return didSucceed;
}




class RadioDataSource: public DisplayDataSource{
public:
	
	RadioDataSource(TMP117*, QwiicTwist* );
	//	virtual ~DisplayDataSource() {}
	//
	virtual bool getStringForKey(string_view key,  string &result);
	virtual bool getFloatForKey(string_view key,  float &result);
	virtual bool getDoubleForKey(string_view key,  double &result);
	virtual bool getIntForKey(string_view key,  int &result);
	
private:
	TMP117 		*_tmp117 = NULL;
	QwiicTwist	*_vol = NULL;
	
};

RadioDataSource::RadioDataSource( TMP117* temp, QwiicTwist* vol){
	_vol = vol;
	_tmp117 = temp;
}

bool RadioDataSource::getStringForKey(string_view key,  string &result){
 
	return false;
}

bool RadioDataSource::getIntForKey(string_view key,  int &result){
	
	if(key == DS_KEY_MODULATION_MODE){
	 
		RadioMgr*	radio = RadioMgr::shared();
		result = radio->radioMode();
		return  true;
	}
 
	return false;
}

 
bool RadioDataSource::getDoubleForKey(string_view key,  double &result){
	
	if(key == DS_KEY_RADIO_FREQ){
		RadioMgr*	radio = RadioMgr::shared();
		result = radio->frequency();
		return  true;
		 
	}
 
	return false;
}

bool RadioDataSource::getFloatForKey(string_view key,  float &result){
	
	if(key == DS_KEY_OUTSIDE_TEMP){
		float temp = 0;
		if(_tmp117->readTempF(temp)) {
			result = temp;
			return  true;
		}
	}
	else if(key == DS_KEY_RADIO_VOLUME){
		int16_t twistCount = 0;
		
		static int16_t current_volume = 0;
		
		if(_vol->getDiff(twistCount, true)) {
			
			int newLevel = current_volume + twistCount;
			if(newLevel > 20) newLevel = 20;
			if(newLevel < 0) newLevel = 0;
			current_volume = newLevel;
			
			result =  current_volume /20.0;
			
			return  true;
		}
	}
	else if(key == DS_KEY_CPU_TEMP){
		float temp = 0;
		if(getCPUTemp(temp)) {
			result = temp;
			return  true;
		}
	}
 
	return false;
}

int main(int argc, const char * argv[]) {
	
	DisplayMgr*	display 	= DisplayMgr::shared();
	RadioMgr*	radio 	= RadioMgr::shared();
	TMP117 		tmp117;
	QwiicTwist	twist;
	
	RadioDataSource source(&tmp117, &twist);

	try {
		
		if(!display->begin("/dev/ttyUSB0",B9600))
			throw Exception("failed to setup Display ");
		
		display->setDataSource(&source);
		
		if(!display->setBrightness(7))
			throw Exception("failed to set brightness ");
		
		if(!tmp117.begin(0x4A))
			throw Exception("failed to setup TMP117 ");
		
		if(!twist.begin())
			throw Exception("failed to setup QwiicTwist ");
		
		// dim button down
		twist.setColor(0, 8, 0);
		
	 //	radio->setFrequency(1440e3);
	 //	radio->setFrequency(88.1e6);
	  radio->setFrequency(155.610e6);
	
		radio->setRadioMode(RadioMgr::RADIO_OFF);
		
		while(true){
			bool clicked = false;
			bool moved = false;
 
			if(twist.isMoved(moved) && moved){
				int16_t twistCount = 0;
				
				if(twist.getDiff(twistCount, true)) {
					auto newfreq = radio->nextFrequency(twistCount > 0);
	 
					if(( radio->radioMode() != RadioMgr::RADIO_OFF)
						&& radio->setFrequency(newfreq)){
						display->showRadioChange();
					}
				}
			}
			
			if(twist.isClicked(clicked) && clicked) {
				
				if(radio->radioMode() != RadioMgr::RADIO_OFF){
					radio->setRadioMode(RadioMgr::RADIO_OFF);
				}
				else {
					radio->setRadioMode(RadioMgr::VHF);
				}
				display->showRadioChange();
				
			}
			
			usleep(1);
		};
		
		
	}
	catch ( const Exception& e)  {
		printf("\tError %d %s\n\n", e.getErrorNumber(), e.what());
		return -1;
	}
	catch (std::invalid_argument& e)
	{
		printf("EXCEPTION: %s ",e.what() );
		return -1;
	}
	
	return EXIT_SUCCESS;
}
