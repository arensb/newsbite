include ../Makefile.common

HTML_FILES =	$(wildcard *.html)
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
	mkdir -p ${INSTALL_HTDOCS}
	for i in ${HTML_FILES} ${PHP_FILES} ${OTHER_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_HTDOCS}; \
	done

clean::
	@:

distclean::	clean
	@:

depend::
	@:

FORCE:
