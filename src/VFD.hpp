//
//  VFD.hpp
//  vfdtest
//
//  Created by Vincent Moscaritolo on 4/13/22.
//
#pragma once

#include "I2C.hpp"


using namespace std;

class VFD {
public:

	VFD();
  ~VFD();
	
  bool begin();		// alwsys uses a fixed address
  bool begin(int &error);
  void stop();

 	bool reset();

	bool write(string str);

private:
	
	bool writePacket(string str);

	I2C 		_i2c;
	bool		_isSetup;


};
