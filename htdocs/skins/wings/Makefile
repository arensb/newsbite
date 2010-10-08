CPP =		cpp
INCLUDES =	# -Ijs
CPP_FLAGS =	-P $(INCLUDES)
RM =		rm -f
MAKEDEPEND =	cpp -M

JS_SRC =	wings.jsh
JS_FILES = 	$(patsubst %.jsh,%.js,${JS_SRC})

.SUFFIXES:	.jsh .js
.PHONY:		clean depend

all:	Make.depend ${JS_FILES}

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

clean::
	$(RM) Make.depend
