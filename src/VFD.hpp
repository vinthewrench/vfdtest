//
//  VFD.hpp
//  vfdtest
//
//  Created by Vincent Moscaritolo on 4/13/22.
//
#pragma once

#include "I2C.hpp"
#include  <stddef.h>
#include <unistd.h>


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
	
  bool begin();		// alwsys uses a fixed address
  bool begin(int &error);
  void stop();

 	bool reset();

	bool write(string str);

	bool setBrightness(uint8_t);  //  0 == off - 7 == max

	bool setCursor(uint8_t x, uint8_t y); 
	bool  setFont(font_t font);

private:
	
	bool writePacket(const uint8_t *data , size_t len , useconds_t waitusec = 50);

	I2C 		_i2c;
	bool		_isSetup;


};
