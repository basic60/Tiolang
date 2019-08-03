CC=g++

cppSrc=$(shell find . -name "*.cc")
cppObj=$(patsubst %.cc,%.o,$(cppSrc))
executableFile=tio
CXXFLAGS=-I../lib/ -std=c++17 -g -c -O2 -o 

all: $(cppObj) link 

link:$(cppObj)
	$(CC) $(cppObj) ../lib/libtiolog.a ../lib/libstrhelper.a -o $(executableFile)

%.o:%.cc
	$(CC) $(CXXFLAGS) $@ $<

clean:
	rm $(cppObj) $(executableFile)