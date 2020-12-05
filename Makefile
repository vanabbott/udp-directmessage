#Van Abbott fabbott@nd.edu

#g++ is for the gcc compiler for c++
PP := g++

#these are all the flags we want to use when compiling c
#FLAGS := -O0 -g -Wall -Wextra -Wconversion -Wshadow -pedantic -lm
FLAGS := -g
#these are all the flags we want to use when compiling c++
#CXXFLAGS := -m64 -std=c++11 -Weffc++ $(FLAGS)
CXXFLAGS := -std=c++11
TARGET := udpclien

#creates the testFloat executable for testFLoat.cpp program

udpclient: udpclient.cpp
		$(PP) $(CXXFLAGS) udpclient.cpp -lcrypto -lz -o udpclient

udpserver: udpserver.cpp
		$(PP) $(CXXFLAGS) udpserver.cpp -lcrypto -lz -o udpserver

#makes all the exectuables
all: udpclient udpserver

#cleans up everything
clean:
		rm -rf *.o udpclient udpserver


