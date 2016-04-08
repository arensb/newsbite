include ../../Makefile.common

INC_FILES =	$(wildcard *.inc)
OTHER_FILES =	

all::
	@:

install::
	mkdir -p ${INSTALL_LIB_REST}
	for i in ${INC_FILES} ${OTHER_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_LIB_REST}; \
	done

clean::
	@:

distclean::	clean
	@:

depend::
	@:
