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
		uint8_t dimLevel = 4;
		
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
  
//		if(!twist.setLimit(8))
//			throw Exception("failed to set QwiicTwist count ");
//
//		if(!in219.begin())
//			throw Exception("failed to setup IN219 ");

		if(!vfd.begin())
			throw Exception("failed to setup VFD ");
		
		for(int i = 0;; i++){
			if(vfd.reset()) break;
			if(i > 9)
				throw Exception("failed to RESET VFD ");
 		}
 
		
		if(!vfd.setBrightness(dimLevel))
			throw Exception("failed to Set Brightness VFD ");
		
#define TRY(_statement_) if(!(_statement_)) { \
			printf("FAIL AT line: %d\n", __LINE__ ); \
			if(!vfd.begin()) Exception("failed to reset VFD "); continue; }

		while(true){
			
			char buffer[128] = {0};
 
			time_t now = time(NULL);
			struct tm *t = localtime(&now);
			
			float temp;
			float vIn;
			float iOut;
	//		float vBatt;
			bool moved = false;
			
			tmp117.readTempF(temp);
			pwr.voltageOut(vIn);
			pwr.currentOut(iOut);
	//		vBatt = in219.getBusVoltage_V();
			
			twist.isMoved(moved);
			if(moved){
				
				int16_t twistCount = 0;
				if(twist.getDiff(twistCount, true)) {
					int newLevel = dimLevel + twistCount;
					if(newLevel > 7) newLevel = 7;
					if(newLevel < 0) newLevel = 0;
					dimLevel = newLevel;
					
					TRY(vfd.setBrightness(dimLevel));
					twist.setColor(0, dimLevel << 5, 0);
				}
			}
			
			TRY(vfd.setCursor(10,14));
			TRY(vfd.setFont(VFD::FONT_10x14));
			std::strftime(buffer, sizeof(buffer)-1, "%l:%M:%S%P", t);
//			std::strftime(buffer, sizeof(buffer)-1, "%H:%M:%S%P", t);
			TRY(vfd.write(buffer));
			usleep(100);
			
			TRY(vfd.setCursor(10, 25));
			TRY(vfd.setFont(VFD::FONT_5x7));
			sprintf(buffer, "Temp: %d\xA0\x46", (int) round(temp) );
			TRY(vfd.write(buffer));
			usleep(100);


			TRY(vfd.setCursor(10, 35));
			TRY(vfd.setFont(VFD::FONT_5x7));
			sprintf(buffer, "Pwr: %-2.2fV", vIn);
			TRY(vfd.write(buffer));
			usleep(100);

			TRY(vfd.setCursor(70, 35));
			TRY(vfd.setFont(VFD::FONT_5x7));
			sprintf(buffer, "  %-2.2fA  ", iOut);
			TRY(vfd.write(buffer));
			usleep(100);

			TRY(vfd.setCursor(10, 45));
			TRY(vfd.setFont(VFD::FONT_5x7));
			sprintf(buffer, "Dim: %d ", dimLevel);
			TRY(vfd.write(buffer));
			usleep(100);

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
