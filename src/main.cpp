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

#include "CommonDefs.hpp"

#include "VFD.hpp"
#include "TMP117.hpp"
#include "WittyPi3.hpp"
#include "INA219.hpp"
#include "QwiicTwist.hpp"

int main(int argc, const char * argv[]) {
	
	
	VFD 		vfd;
	TMP117 	tmp117;
	WittyPi3	pwr;
	INA219	in219;
	QwiicTwist	twist;
	
	
	try {
		printf("Test start\n");
		
		if(!tmp117.begin())
			throw Exception("failed to setup TMP117 ");
		
		if(!pwr.begin())
			throw Exception("failed to setup WittyPi3 ");
		
		if(!twist.begin())
			throw Exception("failed to setup QwiicTwist ");
		
		if(!twist.setColor(0, 128, 0))
			throw Exception("failed to set QwiicTwist color ");
		
		if(!twist.setCount(0))
			throw Exception("failed to set QwiicTwist count ");
 
//		if(!in219.begin())
//			throw Exception("failed to setup IN219 ");

		if(!vfd.begin())
			throw Exception("failed to setup VFD ");
		
		if(!vfd.reset())
			throw Exception("failed to RESET VFD ");
		
		if(!vfd.setBrightness(4))
			throw Exception("failed to Set Brightness VFD ");
		
		
		while(true){
			
			char buffer[128] = {0};
		 
			time_t now = time(NULL);
			struct tm *t = localtime(&now);
		
			
			float temp;
			float vIn;
			float iOut;
	//		float vBatt;
			int16_t twistCount = 0;
			bool clicked = false;
			
			tmp117.readTempF(temp);
			pwr.voltageOut(vIn);
			pwr.currentOut(iOut);
	//		vBatt = in219.getBusVoltage_V();
			twist.getCount(twistCount);
			twist.isClicked(clicked);
	 
			vfd.setCursor(10,25);
			vfd.setFont(VFD::FONT_10x14);
			std::strftime(buffer, sizeof(buffer)-1, "%l:%M:%S%P", t);
			vfd.write(buffer);

			vfd.setCursor(16, 40);
			vfd.setFont(VFD::FONT_5x7);
			sprintf(buffer, "Temp: %d\xA0\x46", (int) round(temp) );
			vfd.write(buffer);

			vfd.setCursor(10, 50);
			vfd.setFont(VFD::FONT_5x7);
			sprintf(buffer, "Pwr: %-2.2fV", vIn);
			vfd.write(buffer);
 
			vfd.setCursor(70, 50);
			vfd.setFont(VFD::FONT_5x7);
			sprintf(buffer, "  %-2.2fA  ", iOut);
			vfd.write(buffer);
			
			vfd.setCursor(10, 60);
			vfd.setFont(VFD::FONT_5x7);
			sprintf(buffer, "Twist: %-2d %s ", twistCount, clicked?"C": " ");
			vfd.write(buffer);

//
//			vfd.setCursor(10, 60);
//			vfd.setFont(VFD::FONT_5x7);
//			sprintf(buffer, "Batt: %-2.2fV", vBatt);
//			vfd.write(buffer);
		
			
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
