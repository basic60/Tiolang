all:
	$(MAKE) all -C lib
	$(MAKE) all -C compiler

clean:
	$(MAKE) clean -C lib
	$(MAKE) clean -C compiler