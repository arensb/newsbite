TOP =		..

JS_SRC =	$(wildcard *.js)

RM =		rm -f

include $(TOP)/Makefile.common

OTHER_FILES =	config.js

all::	$(OTHER_FILES) Make.depend
	@:

# Include dependencies: which files depend on what
include Make.depend

config.js:	config.js.in
	sed ${SED_REPLACEMENTS} < $@.in > $@


install::
	@:

clean::
	rm -f config.js

distclean::	clean
	$(RM) Make.depend

# Make.depend
# Generate a list of which .js files depend on which source files.
depend::	Make.depend
Make.depend:	${JS_SRC} ${OTHER_FILES}
	$(RM) $@
	for i in $(JS_SRC); do \
		$(MAKEDEPEND) -I$(TOP)/js-src -MT `echo "$$i" | sed 's/\.jsh$$/.js/'` $$i >> $@; \
	done

