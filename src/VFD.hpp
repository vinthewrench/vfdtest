//
//  VFD.hpp
//  vfdtest
//
//  Created by Vincent Moscaritolo on 4/13/22.
//
#pragma once
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <termios.h>
#include <string>


using namespace std;

class VFD {
public:

	typedef enum  {
		FONT_MINI = 0,
		FONT_5x7 ,
		FONT_10x14,
 	}font_t;

	VFD();
  ~VFD();
	
  bool begin(string path);		// alwsys uses a fixed address
  bool begin(string path, int &error);
  void stop();

 	bool reset();

	bool write(string str);
	bool write(const char* str);
	bool writePacket(const uint8_t *data , size_t len , useconds_t waitusec = 50);

	bool setBrightness(uint8_t);  //  0 == off - 7 == max

	bool setCursor(uint8_t x, uint8_t y); 
	bool  setFont(font_t font);

private:
	

	int	 	_fd;
	bool		_isSetup;

	struct termios _tty_opts_backup;

};
