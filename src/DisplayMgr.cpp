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

#define TRY(_statement_) if(!(_statement_)) { \
printf("FAIL AT line: %d\n", __LINE__ ); \
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
				if(diff.tv_sec >=  5) {
					pushMode(MODE_TIME);
					shouldRedraw = true;
				}
			}
			else if(diff.tv_sec >=  4){
				// should we pop the mode?
				if(!isStickyMode(_current_mode)){
					popMode();
					shouldRedraw = true;
				}
			}
		}
		
		displayUpdate(shouldRedraw);
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

void DisplayMgr::displayUpdate(bool redraw){
	
	if(!_isSetup)
		return;
	try {
		switch (_current_mode) {
				
			case MODE_STARTUP:
				displayStartupScreen(redraw);
				break;
				
			case MODE_TIME:
				displayTimeScreen(redraw);
				break;
				
			case MODE_VOLUME:
				displayVolumeScreen(redraw);
				break;
				
			case MODE_RADIO:
				displayRadioScreen(redraw);
				break;
				
			case MODE_DIAG:
				displayDiagScreen(redraw);
				
				
			default:
				displayInternalError(redraw);
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


void DisplayMgr::displayStartupScreen(bool redraw){
	
 
	if(redraw)
		_vfd.clearScreen();
	
	TRY(_vfd.setCursor(10,14));
	TRY(_vfd.setFont(VFD::FONT_5x7));
	TRY(_vfd.write("Starting UP"));
	
//	printf("displayStartupScreen %s\n",redraw?"REDRAW":"");
}

void DisplayMgr::displayTimeScreen(bool redraw){
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	char buffer[128] = {0};
	
	if(redraw)
		_vfd.clearScreen();

	std::strftime(buffer, sizeof(buffer)-1, "%l:%M:%S%P", t);
	
	TRY(_vfd.setCursor(10,14));
	
	TRY(_vfd.setFont(VFD::FONT_10x14));
	TRY(_vfd.write(buffer));
 
//	printf("displayTimeScreen %s\n",redraw?"REDRAW":"");

}

void DisplayMgr::displayVolumeScreen(bool redraw){
	printf("displayVolumeScreen %s\n",redraw?"REDRAW":"");

	if(redraw)
		_vfd.clearScreen();
	 	
	TRY(_vfd.setCursor(10,14));
	TRY(_vfd.setFont(VFD::FONT_5x7));
	TRY(_vfd.write("Volume Change"));

}

void DisplayMgr::displayRadioScreen(bool redraw){
	printf("display RadioScreen %s\n",redraw?"REDRAW":"");

}

void DisplayMgr::displayDiagScreen(bool redraw){
	printf("displayDiagScreen %s\n",redraw?"REDRAW":"");

}


void DisplayMgr::displayInternalError(bool redraw){
	
	printf("displayInternalError %s\n",redraw?"REDRAW":"");
}

