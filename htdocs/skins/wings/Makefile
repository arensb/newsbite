CPP =		cpp
INCLUDES =	# -Ijs
CPP_FLAGS =	-P $(INCLUDES)
RM =		rm -f
MAKEDEPEND =	cpp -M

JS_SRC =	wings.jsh
JS_FILES = 	$(patsubst %.jsh,%.js,${JS_SRC})
MANIFESTS =	default.manifest \
		iphone.manifest \
		ipad.manifest

.SUFFIXES:	.jsh .js
.PHONY:		clean depend

all:	Make.depend ${JS_FILES} ${MANIFESTS}

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

# The client only downloads new copies of cached file when the text of
# the manifest file changes. So we need to include a nonce, a string
# that changes every time we run 'make', in the manifest.
MANIF_NONCE =	$(shell date)

iphone.manifest:	manifest.h
	$(CPP) $(CPP_FLAGS) -DMOBILE -DIPHONE -DNONCE="$(MANIF_NONCE)" -o $@ manifest.h

ipad.manifest:		manifest.h
	$(CPP) $(CPP_FLAGS) -DMOBILE -DIPAD -DNONCE="$(MANIF_NONCE)" -o $@ manifest.h

default.manifest:	manifest.h
	$(CPP) $(CPP_FLAGS) -DNONCE="$(MANIF_NONCE)" -o $@ manifest.h
