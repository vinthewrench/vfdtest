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
#include "TMP102.hpp"
#include "WittyPi3.hpp"

int main(int argc, const char * argv[]) {
	
	
	VFD 		vfd;
	TMP102 	tmp102;
	WittyPi3	pwr;
	
	
	try {
		printf("Test start\n");
		
		if(!tmp102.begin())
			throw Exception("failed to setup tmp102 ");
		
		if(!pwr.begin())
			throw Exception("failed to setup WittyPi3 ");
		
		if(!vfd.begin())
			throw Exception("failed to setup VFD ");
		
		if(!vfd.reset())
			throw Exception("failed to RESET VFD ");
		
		while(true){
			
			auto t = std::time(nullptr);
			auto tm = *std::localtime(&t);
			float temp;
			
			tmp102.readTempF(temp);
			
			string str;
			
			std::ostringstream oss;
			oss <<  "\x10\x08\x25\x1E" <<  std::put_time(&tm, " %X");
			oss <<  "\x10\x12\x35\x1D"  << round(temp)  << "\xA0" << "F";
			str = oss.str();
			//vfd.write(str);
			
			if(!vfd.write(str) ){
				//    printf("failed write\n");
				//  break;
			}
			
			// usleep(100000);
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
