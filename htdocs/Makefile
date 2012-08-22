include ../Makefile.common

PHP_FILES =	$(wildcard *.php)
OTHER_FILES =	.htaccess

all::
	@:

install::
	mkdir -p ${INSTALL_BACKEND}
	for i in ${PHP_FILES} ${OTHER_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_BACKEND}; \
	done

clean::
	@:

distclean::	clean
	@:

depend::
	@:
