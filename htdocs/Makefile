include ../Makefile.common

PHP_FILES =	$(wildcard *.php)
OTHER_FILES =	htaccess.sample

all::	$(OTHER_FILES)

htaccess.sample:	htaccess.sample.in
	sed ${SED_REPLACEMENTS} < htaccess.sample.in > $@

clean::
	rm -f htaccess.sample

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
