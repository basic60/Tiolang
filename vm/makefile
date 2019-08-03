CC=g++

vmSrc=$(shell find . -name "*.cc")
vmObj=$(patsubst %.cc,%.o,$(vmSrc))
executableFile=tiovm
CXXFLAGS=-I../lib/ -std=c++17 -g -c -O2 -o 

all: $(vmObj) link 

link:$(vmObj)
	$(CC) $(vmObj) ../lib/libtiolog.a ../lib/libstrhelper.a -o $(executableFile)

%.o:%.cc
	$(CC) $(CXXFLAGS) $@ $<

clean:
	rm $(vmObj) $(executableFile)
