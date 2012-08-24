TOP =		../../..
SKIN_NAME =	fancy

CPP =		cpp
INCLUDES =	# -Ijs
CPP_FLAGS =	-P $(INCLUDES)
RM =		rm -f
MAKEDEPEND =	cpp -M

JS_SRC =	view.jsh feeds.jsh

# Files to install
PHP_FILES =	$(wildcard *.php)
JS_FILES = 	$(patsubst %.jsh,%.js,${JS_SRC})
CSS_FILES =	$(wildcard *.css)
IMAGE_FILES =	$(wildcard *.gif *.png *.jpg)

include ../../../Makefile.common

# Where to install this skin
INSTALL_FANCY ?=	${INSTALL_SKIN}/${SKIN_NAME}

.SUFFIXES:	.jsh .js

all::	Make.depend ${JS_FILES}
	@:

# Include dependencies: which .js files depend on what
include Make.depend

# How to build .js files from .jsh files: run them through cpp.
.jsh.js:
	$(CPP) $(CPP_FLAGS) -o $@ $<

clean::
	$(RM) $(JS_FILES)

distclean::	clean
	$(RM) Make.depend

# Make.depend
# Generate a list of which .js files depend on which source files.
depend:	Make.depend
Make.depend:	${JS_SRC}
	$(RM) $@
	for i in $(JS_SRC); do \
		$(MAKEDEPEND) -MT `echo "$$i" | sed 's/\.jsh$$/.js/'` $$i >> $@; \
	done

install::	install-php install-css install-js install-images

install-php:	${PHP_FILES}
	mkdir -p ${INSTALL_FANCY}
	for i in ${PHP_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_FANCY}; \
	done

install-css:	${CSS_FILES}
	mkdir -p ${INSTALL_FANCY}
	for i in ${CSS_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_FANCY}; \
		if [ x"${COMPRESS_CSS}" = xyes ]; then \
			${GZIP} -f "${INSTALL_FANCY}/$$i"; \
		fi; \
	done

install-js:	${JS_FILES}
	mkdir -p ${INSTALL_FANCY}
	for i in ${JS_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_FANCY}; \
		if [ x"${COMPRESS_JS}" = xyes ]; then \
			${GZIP} -f "${INSTALL_FANCY}/$$i"; \
		fi; \
	done

install-images:	${IMAGE_FILES}
	mkdir -p ${INSTALL_FANCY}
	for i in ${IMAGE_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_FANCY}; \
	done
