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

#include "VFD.hpp"
#include "TMP117.hpp"
//#include "INA219.hpp"
#include "QwiicTwist.hpp"

#include "DisplayMgr.hpp"

bool getCPUTemp(double & tempOut) {
	bool didSucceed = false;
	
	try{
		std::ifstream   ifs;
		ifs.open("/sys/class/thermal/thermal_zone0/temp", ios::in);
		if( ifs.is_open()){
			
			if(tempOut){
				string val;
				ifs >> val;
				ifs.close();
				double temp = std::stod(val);
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


int main(int argc, const char * argv[]) {
	
	
	VFD 		vfd;
	TMP117 	tmp117;
	//	WittyPi3	pwr;
	//	INA219	in219;
	QwiicTwist	twist;
	
	DisplayMgr display;
	
	sleep(2);

	for(int i = 0; i < 20; i++){
		
		if(i == 1)
			display.showVolumeChange();
		if(i == 2){
			display.showVolumeChange(); usleep(1);
			display.showVolumeChange();  usleep(1);
			display.showVolumeChange(); usleep(1);
			display.showVolumeChange(); usleep(1);
		}
		if(i == 4){
			display.showRadioChange();
			sleep(2);
		}
		
		if(i == 5)
			display.showVolumeChange();
 
		if(i == 7){
			display.showRadioChange();
			sleep(2);
		}

 	sleep(1);

	}
 
	
	return EXIT_SUCCESS;

	try {
		uint8_t dimLevel = 4;
		
		printf("Test start\n");
		
		if(!vfd.begin("/dev/ttySC0",B57600))
			throw Exception("failed to setup VFD ");
		
		if(!tmp117.begin(0x4A))
			throw Exception("failed to setup TMP117 ");
		//
		//		if(!pwr.begin())
		//			throw Exception("failed to setup WittyPi3 ");
		
		if(!twist.begin())
			throw Exception("failed to setup QwiicTwist ");
		
		// if(!twist.setColor(0, 128, 0))
		// 	throw Exception("failed to set QwiicTwist color ");
		
		// if(!twist.setCount(0))
		// 	throw Exception("failed to set QwiicTwist count ");
		
		//		if(!twist.setLimit(8))
		//			throw Exception("failed to set QwiicTwist count ");
		//
		//		if(!in219.begin())
		//			throw Exception("failed to setup IN219 ");
		
		for(int i = 0;; i++){
			if(vfd.reset()) break;
			if(i > 9)
				throw Exception("failed to RESET VFD ");
		}
		
		usleep(800);
		
		if(!vfd.setBrightness(dimLevel))
			throw Exception("failed to Set Brightness VFD ");
		
#define TRY(_statement_) if(!(_statement_)) { \
printf("FAIL AT line: %d\n", __LINE__ ); \
}
		
		
		// const uint8_t* msg = (uint8_t*) "\x1B\x00\x25\x1D\x10\x15\x1FPLEASE WAIT\x10\x07\x29Initializing System";
		// vfd.writePacket(msg, 40, 10000);
		// sleep(1);
		
		while(true){
			
			struct timeval begin, end;
			gettimeofday(&begin, 0);
			
			char buffer[128] = {0};
			
			time_t now = time(NULL);
			struct tm *t = localtime(&now);
			double cpuTemp = 0;
			double lastCPUtemp = 0;
			
			float temp;
			bool moved = false;
			bool clicked = false;
			
			std::strftime(buffer, sizeof(buffer)-1, "%l:%M:%S%P", t);
			//
			tmp117.readTempF(temp);
			getCPUTemp(cpuTemp);
			
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
			TRY(vfd.write(buffer));
			
			TRY(vfd.setCursor(10, 25));
			TRY(vfd.setFont(VFD::FONT_5x7));
			sprintf(buffer, "%3d\xA0\x46", (int) round(temp) );
			TRY(vfd.write(buffer));
			
			if(fabs(cpuTemp - lastCPUtemp) > 1.0) {
				TRY(vfd.setCursor(65, 25));
				//TRY(vfd.setFont(VFD::FONT_5x7));
				sprintf(buffer, "CPU:%3d\xA0\x43", (int) round(cpuTemp) );
				TRY(vfd.write(buffer));
			}
			lastCPUtemp = cpuTemp;
			//	usleep(100);
			
			//			TRY(vfd.setCursor(10, 35));
			//			TRY(vfd.setFont(VFD::FONT_5x7));
			//			sprintf(buffer, "Pwr: %-2.2fV", vIn);
			//			TRY(vfd.write(buffer));
			//			usleep(100);
			//
			//			TRY(vfd.setCursor(70, 35));
			//			TRY(vfd.setFont(VFD::FONT_5x7));
			//			sprintf(buffer, "  %-2.2fA  ", iOut);
			//			TRY(vfd.write(buffer));
			//	usleep(100);
			
			TRY(vfd.setCursor(10, 45));
			TRY(vfd.setFont(VFD::FONT_5x7));
			sprintf(buffer, "Dim: %d ", dimLevel);
			TRY(vfd.write(buffer));
			//	usleep(100);
			
			
			//			vfd.setCursor(10, 60);
			//			vfd.setFont(VFD::FONT_5x7);
			//			sprintf(buffer, "Batt: %-2.2fV", vBatt);
			//			vfd.write(buffer);
			
			if(twist.isClicked(clicked) && clicked) break;
			
			//usleep(500);
			
			
			gettimeofday(&end, 0);
			long seconds = end.tv_sec - begin.tv_sec;
			long microseconds = end.tv_usec - begin.tv_usec;
			double elapsed = seconds + microseconds*1e-6;
			if(elapsed > 1.0)
				printf(" %.3f seconds\n", elapsed);
			
		}
		vfd.reset();
		vfd.reset();
		vfd.stop();
		//	twist.setColor(0, 0, 0);
		
		
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
