//
//  VFD.cpp
//  vfdtest
//
//  Created by Vincent Moscaritolo on 4/13/22.
//

#include "VFD.hpp"
#include <fcntl.h>
#include <errno.h> // Error integer and strerror() function
#include "ErrorMgr.hpp"


VFD::VFD(){
_isSetup = false;
	_fd = -1;
	
	
}

VFD::~VFD(){
	stop();
}


bool VFD::begin(string path, speed_t speed){
	int error = 0;

	return begin(path, speed, error);
}

 
bool VFD::begin(string path, speed_t speed,  int &error){
	
	_isSetup = false;
	struct termios options;
	
	int fd ;
	
		if((fd = ::open( path.c_str(), O_RDWR | O_NOCTTY)) <0) {
		ELOG_ERROR(ErrorMgr::FAC_DEVICE, 0, errno, "OPEN %s", path.c_str());
		error = errno;
		return false;
	}
	
	fcntl(fd, F_SETFL, 0);      // Clear the file status flags

		// Back up current TTY settings
	if( tcgetattr(fd, &_tty_opts_backup)<0) {
		ELOG_ERROR(ErrorMgr::FAC_DEVICE, 0, errno, "tcgetattr %s", path.c_str());
		error = errno;
		return false;
	}
	
	cfmakeraw(&options);
	options.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
	options.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
	options.c_cflag &= ~CSIZE; // Clear all bits that set the data size
	options.c_cflag |= CS8; // 8 bits per byte (most common)
	// options.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control 	options.c_cflag |=  CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
	options.c_cflag |=  CRTSCTS; // DCTS flow control of output
	options.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
	
	options.c_lflag &= ~ICANON;
	options.c_lflag &= ~ECHO; // Disable echo
	options.c_lflag &= ~ECHOE; // Disable erasure
	options.c_lflag &= ~ECHONL; // Disable new-line echo
	options.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
	options.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
	
	options.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	options.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
 
	cfsetospeed (&options, speed);
	cfsetispeed (&options, speed);

	 if (tcsetattr(fd, TCSANOW, &options) < 0){
		ELOG_ERROR(ErrorMgr::FAC_DEVICE, 0, errno, "Unable to tcsetattr %s", path.c_str());
		error = errno;
		return false;
	}
  
	_fd = fd;
	_isSetup = true;
	return _isSetup;
}

void VFD::stop(){
	
	if(_isSetup && _fd > -1){
		// Restore previous TTY settings
		tcsetattr(_fd, TCSANOW, &_tty_opts_backup);
		close(_fd);
		_fd = -1;
	}
	
	_isSetup = false;
 }

 

bool VFD::reset(){
	
	uint8_t buffer[] = {0x19};
	return  writePacket(buffer, sizeof(buffer), 1000);
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


bool VFD::clearScreen(){
	
	uint8_t buffer[] = {0x12, 0, 0, 0xff, 0xff};

	return  writePacket(buffer, sizeof(buffer), 50);
}
 


bool VFD::write(const char* str){
	return  writePacket( (uint8_t *) str, strlen(str), 500);

	
}

bool VFD:: write(string str){
	
	return  writePacket( (uint8_t *) str.c_str(), str.size(), 500);
}
 

#define PACKET_MODE 0

bool VFD:: writePacket(const uint8_t * data, size_t len, useconds_t waitusec){
	
	bool success = false;
 
#if PACKET_MODE
	constexpr size_t blocksize = 32;
	uint8_t buffer[blocksize + 4 ];

	size_t bytesLeft = len;
		while(bytesLeft > 0) {
	
			uint8_t len = bytesLeft < 28?bytesLeft:28;
			uint8_t checksum = 0;
	
			uint8_t *p = buffer;
			*p++ = 0x02;
			*p++ =  len;
	
			for(int i = 0; i < len; i++){
				checksum += *data;
				*p++ = *data++;
			}
			*p++ = checksum;
			*p++ =  0x03;
	
	#if 1
		for(int i = 0; i < len +4; i++){
			success = (::write(_fd,&buffer[i] , 1) == 1);
				if(!success) return false;
				 usleep(10);
		}

	#else
			success = (::write(_fd,buffer , len) == len);
			if(!success) break;
			usleep(waitusec);
	#endif
			
			usleep(waitusec);

		if(!success) break;
		bytesLeft-=len;
		}

#else

	success = (::write(_fd, data , len) == len);

if(!success)
	printf("error %d\n",errno);
 
	// for(int i = 0; i<len; i++){
	// 	success = (::write(_fd, &data[i], 1) == 1);
	//  	if(!success) break;
	// }
	// usleep(waitusec);
#endif
	
	return success;
}

