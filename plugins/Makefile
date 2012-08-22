include ../Makefile.common

PHP_FILES =	$(wildcard *.php)

all::
	@:

# XXX
install::
	mkdir -p ${INSTALL_PLUGINS}
	for i in ${PHP_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_PLUGINS}; \
	done

clean::
	@:

distclean::	clean
	@:

depend::
	@:
