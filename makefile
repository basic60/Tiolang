all:
	$(MAKE) all -C lib
	$(MAKE) all -C compiler
	$(MAKE) all -C vm

clean:
	$(MAKE) clean -C lib
	$(MAKE) clean -C compiler
	$(MAKE) clean -C vm