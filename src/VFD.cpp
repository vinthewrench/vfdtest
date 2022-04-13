//
//  VFD.cpp
//  vfdtest
//
//  Created by Vincent Moscaritolo on 4/13/22.
//

#include "VFD.hpp"


VFD::VFD(){
_isSetup = false;}

VFD::~VFD(){
	stop();
}

constexpr uint8_t kAddr = 0x38;
 
bool VFD::begin(){
	int error = 0;

	return begin(error);
}

 
bool VFD::begin(int &error){
	
	_isSetup = false;
 
	if( _i2c.begin(kAddr, error) )
	{
		_isSetup = true;
	}
 
	return _isSetup;
}

void VFD::stop(){
	_isSetup = false;
	_i2c.stop();
 }



bool VFD::reset(){
	return  writePacket( "\x19");
}



bool VFD:: write(string str){
	return  writePacket( str);
}


bool VFD:: writePacket(string str){

	bool success = false;
	I2C::i2c_block_t block;
		
	if(1) {
		auto bytesLeft = str.size();
		const char* data = str.c_str();
		
		while(bytesLeft > 0) {
			
			uint8_t len = bytesLeft < 28?bytesLeft:28;
			uint8_t checksum = 0;
			
			uint8_t *p = block;
			*p++ = 0x02;
			*p++ =  len;
			
			for(int i = 0; i < len; i++){
				checksum += *data;
				*p++ = *data++;
			}
			*p++ = checksum;
			*p++ =  0x03;
			
			for(int i = 0; i < len +4; i++){
				 if(!_i2c.writeByte(block[i]))
					return false;
			}

		
			uint8_t reply = 0;
			if(! _i2c.readByte(reply) ||  reply != 0x50)
				return false;

			// if(!writeBlock(len+ 4, block))
			// 	return false;
		
			bytesLeft-=len;
		}
		
		
		success = true;
	}
	
	return success;
	
}

