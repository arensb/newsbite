TOP =		../..
JQUERY =	jquery-2.1.3.min.js

# *.jsh: files that will be run through cpp
JS_SRC =	$(wildcard *.jsh)
JS_FILES = 	$(patsubst %.jsh,%.js,${JS_SRC})

INCLUDES =	-I$(TOP)/js-src
# Need to force CPP, apparently. Can also use
# gcc -E -x c
# (or any other language). Otherwise gcc complains that "linking not
# done" and doesn't generate the output file.
CPP =		cpp
CPP_FLAGS =	-P $(INCLUDES)
RM =		rm -f

include $(TOP)/Makefile.common

.SUFFIXES:	.jsh .js

all::	Make.depend ${JS_FILES} ${OTHER_FILES}
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
		$(MAKEDEPEND) -I$(TOP)/js-src -MT `echo "$$i" | sed 's/\.jsh$$/.js/'` $$i >> $@; \
	done

install::	install-js install-other

install-js:	${JS_FILES}
	mkdir -p ${INSTALL_HTDOCS}/js
	for i in ${JS_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_HTDOCS}/js; \
		if [ x"${COMPRESS_JS}" = xyes ]; then \
			${GZIP} -f "${INSTALL_HTDOCS}/js/$$i"; \
		fi; \
	done

install-other::
	mkdir -p "${INSTALL_HTDOCS}/js"
	install -c -m 444 "${JQUERY}" ${INSTALL_HTDOCS}/js/jquery.js
	if [ x"${COMPRESS_JS}" = xyes ]; then \
		${GZIP} -f "${INSTALL_HTDOCS}/js/jquery.js"; \
	fi
