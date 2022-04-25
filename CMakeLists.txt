SET (CMAKE_C_COMPILER             "clang")
SET (CMAKE_CXX_COMPILER            "clang++")


cmake_minimum_required(VERSION 3.0)

project(vfd)

set(SOURCES
	src/main.cpp
	src/ErrorMgr.cpp
	src/I2C.cpp
	src/TMP117.cpp
#	src/INA219.cpp
#	src/WittyPi3.cpp
	src/QwiicTwist.cpp
	src/VFD.cpp
	src/DisplayMgr.cpp
	)

add_executable(vfd ${SOURCES})


set_target_properties(vfd PROPERTIES
				CXX_STANDARD 17
				CXX_EXTENSIONS OFF
				)
 
set_target_properties(vfd PROPERTIES
			OUTPUT_NAME "vfd"
			)

			  
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(/usr/include)
link_directories(/usr/lib)
target_link_libraries(vfd  PRIVATE Threads::Threads)

target_include_directories(vfd
	PRIVATE
	src
)

set(CMAKE_BINARY_DIR "bin")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
