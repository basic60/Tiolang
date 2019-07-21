CPUS=$(shell cat /proc/cpuinfo | grep -c processor)

all:
	$(MAKE) all -j$(CPUS) -C lib
	$(MAKE) all -j$(CPUS) -C compiler
	$(MAKE) all -j$(CPUS) -C vm

clean:
	$(MAKE) clean -C lib
	$(MAKE) clean -C compiler
	$(MAKE) clean -C vm