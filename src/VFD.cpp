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
constexpr uint8_t kMaxRetry = 4;

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
	
	if(!_isSetup) return false;
	
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
		
		// try to send a max ofkMaxRetry tries
		success = false;
		for(uint8_t retryCnt = 0; retryCnt < kMaxRetry; retryCnt++){
			
			for(int i = 0; i < len +4; i++){
				if(!_i2c.writeByte(block[i]))
					return false;
			}
			
			// if we dont get a Success code, then try again
			uint8_t reply = 0;
			if( _i2c.readByte(reply) &&  reply == 0x50){
				success = true;
				break;
			}
		}
		 
		if(!success) break;
		
		bytesLeft-=len;
	}
	
 
	return success;
	
}

