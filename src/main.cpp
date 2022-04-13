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

#include "CommonDefs.hpp"

#include "VFD.hpp"
#include "TMP117.hpp"
#include "WittyPi3.hpp"

int main(int argc, const char * argv[]) {
	
	
	VFD 		vfd;
	TMP117 	tmp117;
	WittyPi3	pwr;
	
	
	try {
		printf("Test start\n");
		
		if(!tmp117.begin())
			throw Exception("failed to setup TMP117 ");
		
		if(!pwr.begin())
			throw Exception("failed to setup WittyPi3 ");
		
		if(!vfd.begin())
			throw Exception("failed to setup VFD ");
		
		if(!vfd.reset())
			throw Exception("failed to RESET VFD ");
		
		if(!vfd.setBrightness(4))
			throw Exception("failed to Set Brightness VFD ");
		
		
		while(true){
			
			auto t = std::time(nullptr);
			auto tm = *std::localtime(&t);
			float temp;
			float vIn;
			float iOut;
			
			tmp117.readTempF(temp);
			pwr.voltageOut(vIn);
			pwr.currentOut(iOut);
			
			std::ostringstream oss;
	
			vfd.setCursor(16, 16);
			vfd.setFont(VFD::FONT_10x14);
			oss.str(""); oss << std::put_time(&tm, " %X");
			vfd.write(oss.str());
	
			vfd.setCursor(16, 40);
			vfd.setFont(VFD::FONT_5x7);
			oss.str(""); oss << "Temp: " << round(temp)  << "\xA0" << "F";
			vfd.write(oss.str());
		
			vfd.setCursor(16, 50);
			vfd.setFont(VFD::FONT_5x7);
			oss.str(""); oss << "Volts: " << std::fixed << std::setprecision(2)  << vIn  << "V ";
			vfd.write(oss.str());
	
			
			vfd.setCursor(64, 50);
			vfd.setFont(VFD::FONT_5x7);
			oss.str(""); oss << "Amps: " << std::fixed << std::setprecision(2)  << iOut  << "A ";
			vfd.write(oss.str());
		}
	 
		
		vfd.stop();
	 
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
