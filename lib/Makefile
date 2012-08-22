include ../Makefile.common

INC_FILES =	$(wildcard *.inc)
OTHER_FILES =	config.inc.sample

all::
	@:

install::
	mkdir -p ${INSTALL_LIB}
	for i in ${INC_FILES} ${OTHER_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_LIB}; \
	done

clean::
	@:

distclean::	clean
	@:

depend::
	@:
