CC=g++
AR=ar

tiologSrc=$(shell find ./log -name "*.cc")
tiologObj=$(patsubst %.cc,%.o,$(tiologSrc))
strhelperSrc=$(shell find ./str_helper -name "*.cc")
strhelperObj=$(patsubst %.cc,%.o,$(strhelperSrc))
libTioLog=libtiolog.a
libStrHelper=libstrhelper.a

all: $(tiologObj) $(strhelperObj) link

link:$(tiologObj) $(strhelperObj)
	$(AR) rvs $(libTioLog) $(tiologObj)
	$(AR) rvs $(libStrHelper) $(strhelperObj)

%.o:%.cc
	$(CC) -std=c++17 -g -c -O2 -o $@ $<

clean:
	rm $(tiologObj) $(libTioLog) $(libStrHelper) $(strhelperObj)