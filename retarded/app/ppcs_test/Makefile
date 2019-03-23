
OS=Linux
BOARD=x64
P2P=PPCS

CC=gcc
CXX=g++
STRIP=strip


MY_PATH=$(shell pwd)
LDFLAG =-O2 -g -Wall -DLINUX -lpthread -l${P2P}_API -lstdc++ 
LDFLAG+=-I $(MY_PATH)/shangyun/Release_PPCS_3.1.0/Include/${P2P}/ 
LDFLAG+=-L $(MY_PATH)/shangyun/Release_PPCS_3.1.0/Lib/${OS}/${BOARD}/
LDFLAG+= -I../include/
LDFLAG+= -DLINUX

ifeq ($(BOARD), x64)
LDFLAG+=-m64 
exe=64
else
	ifeq ($(BOARD), x86)
		LDFLAG+=-m32 
		exe=32
	endif
endif

ifeq ($(OS), Linux)
LDFLAG+=-Wl,-rpath=./
dylib=so
endif

ifeq ($(OS), osX)
dylib=dylib
endif

all: clean
	${CXX} ./my_connection_tester.cpp -o my_connection_tester${exe} ${LDFLAG} -s
	cp -rf $(MY_PATH)/shangyun/Release_PPCS_3.1.0/Lib/${OS}/${BOARD}/lib${P2P}_API.${dylib} ./

ifeq ($(OS), osX)
	rm -rf ./*.dSYM
endif

clean:
	rm -rf *.o 
	rm -rf ./lib${P2P}_API.${dylib}
	rm -rf my_connection_tester${exe} 


