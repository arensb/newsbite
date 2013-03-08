include ../Makefile.common

PHP_FILES =	$(wildcard *.php)
OTHER_FILES =	htaccess.sample \
		newsbite.manifest

all::	$(OTHER_FILES)

htaccess.sample:	htaccess.sample.in
	sed ${SED_REPLACEMENTS} < htaccess.sample.in > $@

clean::
	rm -f htaccess.sample

newsbite.manifest:	FORCE
	$(PHP) newsbite.manifest.php > $@

clean::
	rm -f newsbite.manifest

install::	all
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

FORCE:
