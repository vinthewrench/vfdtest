//
//  DisplayMgr.cpp
//  vfdtest
//
//  Created by Vincent Moscaritolo on 4/25/22.
//

#include "DisplayMgr.hpp"
#include <string>
#include <iomanip>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <stdlib.h>
#include <cmath>

#define TRY(_statement_) if(!(_statement_)) { \
printf("FAIL AT line: %d\n", __LINE__ ); \
}

std::string  freq_string(double hz){
	
 	if(hz >= 2.0e6) { // Mhz
		return "Mhz";
	} else if(hz >= 1.0e3) {
		return "Khz";
	}
	
	return "";
}

std::string  hertz_to_string(double hz, int precision = 1){
	
	char buffer[128] = {0};
 
	if(hz >= 2.0e6) { // Mhz
		sprintf(buffer, "%0.*f", precision, hz/1.0e6);
	} else if(hz >= 1.0e3) {
		sprintf(buffer, "%d", (int)round( hz/1.0e3));
	}
	
	return string(buffer);
}

DisplayMgr *DisplayMgr::sharedInstance = NULL;

typedef void * (*THREADFUNCPTR)(void *);

 
DisplayMgr::DisplayMgr(){
	
	pthread_create(&_updateTID, NULL,
								  (THREADFUNCPTR) &DisplayMgr::DisplayUpdateThread, (void*)this);
 
	showStartup();
}

DisplayMgr::~DisplayMgr(){
	
	_event |= DISPLAY_EVENT_EXIT;
	pthread_cond_signal(&_cond);
	pthread_join(_updateTID, NULL);
}


bool DisplayMgr::begin(string path, speed_t speed){
	int error = 0;

	return begin(path, speed, error);
}

bool DisplayMgr::begin(string path, speed_t speed,  int &error){
	
	_isSetup = false;
	
	if(!_vfd.begin(path,speed,error))
		throw Exception("failed to setup VFD ");
	
	if(_vfd.reset())
		_isSetup = true;
	
	
	return _isSetup;
}


void DisplayMgr::stop(){
 
	if(_isSetup){
		_vfd.stop();
	}
	
	_isSetup = false;
}

// MARK: -  display tools

bool DisplayMgr::setBrightness(uint8_t level) {
	
	bool success = false;
	if(_isSetup){
		success = _vfd.setBrightness(level);
	}
	
	return success;
}


// MARK: -  change modes

 
void DisplayMgr::showStartup(){
	setEvent(DISPLAY_EVENT_STARTUP);
 }


void DisplayMgr::showTime(){
	setEvent(DISPLAY_EVENT_TIME);
 }

void DisplayMgr::showDiag(){
	setEvent(DISPLAY_EVENT_DIAG);
 }


void DisplayMgr::showVolumeChange(){
	setEvent(DISPLAY_EVENT_VOLUME);
}

void DisplayMgr::showRadioChange(){
	setEvent(DISPLAY_EVENT_RADIO);
 }

void DisplayMgr::setEvent(uint16_t evt){
	pthread_mutex_lock (&_mutex);
	_event |= evt;
	pthread_cond_signal(&_cond);
	pthread_mutex_unlock (&_mutex);

}

// MARK: -  mode utils

string DisplayMgr::modeString(){
	
	switch (_current_mode) {
		case MODE_UNKNOWN: return("MODE_UNKNOWN");
		case MODE_STARTUP: return("MODE_STARTUP");
		case MODE_TIME: return("MODE_TIME");
		case MODE_VOLUME: return("MODE_VOLUME");
		case MODE_RADIO: return("MODE_RADIO");
		case MODE_DIAG: return("MODE_DIAG");
		case MODE_SHUTDOWN: return("MODE_SHUTDOWN");
			
	}
	return "";
}


bool DisplayMgr::isStickyMode(mode_state_t md){
	bool isSticky = false;
	
	switch(md){
		case MODE_TIME:
		case MODE_DIAG:
			isSticky = true;
			break;
			
		default:
			isSticky = false;
	}
		
	return isSticky;
}


bool DisplayMgr::pushMode(mode_state_t newMode){
	
	bool didChange = false;
	if(_current_mode != newMode){
		if(isStickyMode(_current_mode))
			_saved_mode = _current_mode;
		_current_mode = newMode;
		didChange = true;

	}
	return didChange;
}

void  DisplayMgr::popMode(){
	_current_mode = _saved_mode==MODE_UNKNOWN ? MODE_TIME:_saved_mode;
	_saved_mode = MODE_UNKNOWN;

}

 

// MARK: -  DisplayUpdate thread

void DisplayMgr::DisplayUpdate(){
	
	bool shouldQuit = false;
	
	constexpr time_t sleepTime = 1;
	
	printf("start DisplayUpdate\n");
	
	while(!shouldQuit){
		
		bool shouldRedraw = false;

		// --check if any events need processing else wait for a timeout
		struct timespec ts = {0, 0};
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += sleepTime;
		
		pthread_mutex_lock (&_mutex);
		if (_event == 0)
			pthread_cond_timedwait(&_cond, &_mutex, &ts);
		
		// get a new mode for the event. and reset that event bit
		mode_state_t newMode = MODE_UNKNOWN;
		
		if((_event & DISPLAY_EVENT_STARTUP ) != 0){
			newMode = MODE_STARTUP;
			_event &= ~DISPLAY_EVENT_STARTUP;
		}
		else if((_event & DISPLAY_EVENT_VOLUME ) != 0){
			newMode = MODE_VOLUME;
			_event &= ~DISPLAY_EVENT_VOLUME;
		}
		else if((_event & DISPLAY_EVENT_RADIO ) != 0){
			newMode = MODE_RADIO;
			_event &= ~DISPLAY_EVENT_RADIO;
		}
		else if((_event & DISPLAY_EVENT_TIME ) != 0){
			newMode = MODE_TIME;
			_event &= ~DISPLAY_EVENT_TIME;
		}
		else if((_event & DISPLAY_EVENT_DIAG ) != 0){
			newMode = MODE_DIAG;
			_event &= ~DISPLAY_EVENT_DIAG;
		}
		 else if((_event & DISPLAY_EVENT_EXIT ) != 0){
			 _event &= ~DISPLAY_EVENT_EXIT;
			 shouldQuit = true;
		 }
		
		pthread_mutex_unlock (&_mutex);
 
		if(newMode != MODE_UNKNOWN){
			if(pushMode(newMode)){
				gettimeofday(&_lastEventTime, NULL);
				shouldRedraw = true;
			}
		}

		// no event is a timeout so  update the current mode
		if(newMode == MODE_UNKNOWN ){
			timeval now, diff;
			gettimeofday(&now, NULL);
			timersub(&now, &_lastEventTime, &diff);
			
			if(_current_mode == MODE_STARTUP) {
				if(diff.tv_sec >=  3) {
					pushMode(MODE_TIME);
					shouldRedraw = true;
				}
			}
			else if(diff.tv_sec >=  2){
				// should we pop the mode?
				if(!isStickyMode(_current_mode)){
					popMode();
					shouldRedraw = true;
				}
			}
		}
		
		drawCurrentMode(shouldRedraw);
	}
	
}

void* DisplayMgr::DisplayUpdateThread(void *context){
	DisplayMgr* d = (DisplayMgr*)context;

	//   the pthread_cleanup_push needs to be balanced with pthread_cleanup_pop
	pthread_cleanup_push(   &DisplayMgr::DisplayUpdateThreadCleanup ,context);
 
	d->DisplayUpdate();
	
	pthread_exit(NULL);
	
	pthread_cleanup_pop(0);
	return((void *)1);
}

 
void DisplayMgr::DisplayUpdateThreadCleanup(void *context){
	DisplayMgr* d = (DisplayMgr*)context;

	if(d->_event){
		
	}

	printf("cleanup display\n");
}

// MARK: -  Display Draw code

void DisplayMgr::drawCurrentMode(bool redraw){
	
	if(!_isSetup)
		return;
	try {
		switch (_current_mode) {
				
			case MODE_STARTUP:
				drawStartupScreen(redraw);
				break;
				
			case MODE_TIME:
				drawTimeScreen(redraw);
				break;
				
			case MODE_VOLUME:
				drawVolumeScreen(redraw);
				break;
				
			case MODE_RADIO:
				drawRadioScreen(redraw);
				break;
				
			case MODE_DIAG:
				drawDiagScreen(redraw);
				
				
			default:
				drawInternalError(redraw);
		}
		
	}
	catch ( const Exception& e)  {
		printf("\tError %d %s\n\n", e.getErrorNumber(), e.what());
		 
	}
	catch (std::invalid_argument& e)
	{
		printf("EXCEPTION: %s ",e.what() );
	 
	}
}


void DisplayMgr::drawStartupScreen(bool redraw){
	
 
	if(redraw)
		_vfd.clearScreen();
	
	TRY(_vfd.setCursor(14,40));
	TRY(_vfd.setFont(VFD::FONT_5x7));
	TRY(_vfd.write("Starting Up..."));
	
//	printf("displayStartupScreen %s\n",redraw?"REDRAW":"");
}

void DisplayMgr::drawTimeScreen(bool redraw){
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	char buffer[128] = {0};
	
	if(redraw)
		_vfd.clearScreen();

	std::strftime(buffer, sizeof(buffer)-1, "%2l:%M:%S", t);
	
	TRY(_vfd.setCursor(10,35));
	TRY(_vfd.setFont(VFD::FONT_10x14));
	TRY(_vfd.write(buffer));

	TRY(_vfd.setFont(VFD::FONT_5x7));
	TRY(_vfd.write( (t->tm_hour > 12)?"PM":"AM"));
	
	float temp = 0;
	if(_dataSource
			&& _dataSource->getFloatForKey(DS_KEY_OUTSIDE_TEMP, temp)){
		
		char buffer[64] = {0};
		
			TRY(_vfd.setCursor(10, 55));
			TRY(_vfd.setFont(VFD::FONT_5x7));
			sprintf(buffer, "%3d\xA0\x46", (int) round(temp) );
			TRY(_vfd.write(buffer));
	}
		
		
}

void DisplayMgr::drawVolumeScreen(bool redraw){
	
	constexpr uint8_t rightbox 	= 13;
	constexpr uint8_t leftbox 		= 112;
	constexpr uint8_t topbox 		= 34;
	constexpr uint8_t bottombox 	= 44;
	
	constexpr uint8_t VFD_OUTLINE = 0x14;
	constexpr uint8_t VFD_CLEAR_AREA = 0x12;
	constexpr uint8_t VFD_SET_AREA = 0x11;
	
	try{
		if(redraw){
			_vfd.clearScreen();
			
			// draw centered heading
			_vfd.setFont(VFD::FONT_5x7);
			string str = "Volume";
			_vfd.setCursor(( (126 - (str.size()*6)) /2 ), 29);
			_vfd.write(str);
			
			//draw box outline
			uint8_t buff1[] = {VFD_OUTLINE,rightbox,topbox,leftbox,bottombox};
			_vfd.writePacket(buff1, sizeof(buff1), 0);
		}
		
		float vol = 0;
		if(_dataSource
			&& _dataSource->getFloatForKey(DS_KEY_RADIO_VOLUME, vol)){
			
			uint8_t rndVol =  (int) round(vol * 100);
			uint8_t midBox =  ((uint8_t) round((leftbox - rightbox) * vol)) + rightbox - 1;
			
			// fill volume area box
			uint8_t buff3[] = {VFD_SET_AREA,rightbox,topbox+1,midBox,bottombox-1};
			_vfd.writePacket(buff3, sizeof(buff3), 0);
			
			// clear rest of inside of box
			if(rndVol < 100){
				uint8_t buff2[] = {VFD_CLEAR_AREA, static_cast<uint8_t>(midBox+1),topbox+1,leftbox-1,bottombox-1};
				_vfd.writePacket(buff2, sizeof(buff2), 0);
			}
		}
	} catch (...) {
		// ignore fail
	}
 }


void DisplayMgr::drawRadioScreen(bool redraw){
//	printf("display RadioScreen %s\n",redraw?"REDRAW":"");

	try{
		if(redraw){
			_vfd.clearScreen();
			
		}
		
		double freq = 0;
		int temp;
		

		if(_dataSource
			&& _dataSource->getDoubleForKey(DS_KEY_RADIO_FREQ, freq)
			&& _dataSource->getIntForKey(DS_KEY_MODULATION_MODE, temp)
			){
				modulation_mode_t mode = (modulation_mode_t) temp;
				
			int precision = 0;
			switch (mode) {
				case MM_BROADCAST_AM: precision = 0;break;
				case MM_BROADCAST_FM: precision = 1;break;
				case MM_FM: precision = 3; break;
				default :;
				}
			
			string str = hertz_to_string(freq, precision);
			string hzstr = freq_string(freq);

			TRY(_vfd.setCursor(10,35));
			TRY(_vfd.setFont(VFD::FONT_10x14));
			TRY(_vfd.write(str));
			TRY(_vfd.setFont(VFD::FONT_5x7));
			TRY(_vfd.write(hzstr));


		}
	} catch (...) {
		// ignore fail
	}
}

void DisplayMgr::drawDiagScreen(bool redraw){
	printf("displayDiagScreen %s\n",redraw?"REDRAW":"");

}


void DisplayMgr::drawInternalError(bool redraw){
	
	printf("displayInternalError %s\n",redraw?"REDRAW":"");
}

