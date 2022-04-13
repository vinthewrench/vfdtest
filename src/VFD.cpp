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
	
	uint8_t buffer[] = {0x19};
	return  writePacket(buffer, sizeof(buffer), 500);
}

bool VFD::setBrightness(uint8_t level){
	
	level = level > 7?7:level;
	level |= 0xF8;
	uint8_t buffer[] = {0x1b, level};
	
	return  writePacket(buffer, sizeof(buffer), 50);
}

bool VFD::setCursor(uint8_t x, uint8_t y){
	uint8_t buffer[] = {0x10, x,y};

	return  writePacket(buffer, sizeof(buffer), 50);
}

 bool  VFD::setFont(font_t font){
	uint8_t buffer[] = {0x00};

	switch(font) {
		case FONT_MINI: 	buffer[0] = 0x1c; break;
		case FONT_5x7:	 	buffer[0] = 0x1d; break;
		case FONT_10x14:	buffer[0] = 0x1e; break;
		default:
			return false;
	}
	
	return  writePacket(buffer, sizeof(buffer), 500);

}


bool VFD:: write(string str){
	
	return  writePacket( (uint8_t *) str.c_str(), str.size(), 500);
}


bool VFD:: writePacket(const uint8_t * data, size_t len, useconds_t waitusec){
	
	bool success = false;
	I2C::i2c_block_t block;
	
	if(!_isSetup) return false;
	
	size_t bytesLeft = len;
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
			
			if(waitusec) usleep (waitusec);
			
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

