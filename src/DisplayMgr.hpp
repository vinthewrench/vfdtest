//
//  DisplayMgr.hpp
//  vfdtest
//
//  Created by Vincent Moscaritolo on 4/25/22.
//

#pragma once

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <mutex>
#include <bitset>
#include <sys/time.h>

#include "VFD.hpp"
#include "ErrorMgr.hpp"
#include "CommonDefs.hpp"

using namespace std;
 

class DisplayMgr {
	
	typedef enum  {
		MODE_UNKNOWN = 0,
		MODE_STARTUP,
		MODE_TIME,
		MODE_VOLUME,
		MODE_RADIO,
		MODE_DIAG,
	
		MODE_SHUTDOWN,		// shutdown 

	}mode_state_t;

public:

	  static DisplayMgr *shared() {
		  if(!sharedInstance){
			  sharedInstance = new DisplayMgr;
		  }
		  return sharedInstance;
	  }

	DisplayMgr();
	~DisplayMgr();
	
	bool begin(string path, speed_t speed =  B19200);
	bool begin(string path, speed_t speed, int &error);
	void stop();

	 bool reset();
		
	bool setBrightness(uint8_t level);  // 0-7

	void showTime();
	void showDiag();

	void showStartup();
	void showVolumeChange();
	void showRadioChange();

	void displayUpdate(bool redraw);
	
private:
  
	void displayStartupScreen(bool redraw);
	void displayTimeScreen(bool redraw);
	void displayVolumeScreen(bool redraw);
	void displayRadioScreen(bool redraw);
	void displayDiagScreen(bool redraw);
	void displayInternalError(bool redraw);

	mode_state_t _current_mode = MODE_UNKNOWN;
	mode_state_t _saved_mode   = MODE_UNKNOWN;
	timeval		_lastEventTime = {0,0};
	
	bool isStickyMode(mode_state_t);
	bool pushMode(mode_state_t);
	void popMode();
	
	void setEvent(uint16_t);
 
	void DisplayUpdate();		// C++ version of thread
	// C wrappers for DisplayUpdate;
	static void* DisplayUpdateThread(void *context);
	static void DisplayUpdateThreadCleanup(void *context);
	


#define DISPLAY_EVENT_STARTUP	0x0002
#define DISPLAY_EVENT_EXIT 	0x0004
#define DISPLAY_EVENT_TIME 	0x0008
#define DISPLAY_EVENT_VOLUME 	0x0010
#define DISPLAY_EVENT_RADIO 	0x0020
#define DISPLAY_EVENT_DIAG 	0x0040

	uint16_t				_event = 0;
	 
	pthread_cond_t 	_cond = PTHREAD_COND_INITIALIZER;
 	pthread_mutex_t 	_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_t			_updateTID;
	
	static DisplayMgr *sharedInstance;
 
	// display
	bool 		_isSetup = false;
	VFD 		_vfd;

	// debug stuff
	
	
	string modeString();

};
