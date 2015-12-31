# Variables
PROJECT =	newsbite

# REV_CMD: command to figure out which revision we're using.
REV_CMD =	git describe

# Hack: BSD make uses "VAR != cmd" to assign $VAR the output of `cmd',
# while GNU make uses "VAR = $(shell cmd)". Having things in this
# order makes it work. I think it's because GNU make doesn't recognize
# "!=", so it doesn't see the second assignment, while BSD make does,
# so it ignores the first assignment.
REV =		$(shell ${REV_CMD})

DISTNAME =	${PROJECT}-${REV}

# Include a site-local Makefile, if it exists.
# The $(wildcard) statement is to see whether it exists. Then, if it
# does, we include it.
# The user can also use 'make MAKEFILE_LOCAL=/path/to/file'.
#MAKEFILE_LOCAL = $(wildcard Makefile.local)
#ifneq "${MAKEFILE_LOCAL}" ""
#include ${MAKEFILE_LOCAL}
#endif
include Makefile.common

RECURSIVE_DIRS =	\
	htdocs \
	htdocs/css \
	htdocs/fonts \
	htdocs/images \
	htdocs/js \
	lib \
	plugins

# XXX - Create htdocs/.htaccess based on the directories above
# XXX - Create lib/config.inc based on the directories above

.PHONY:	dist

all::	htmlpurifier

dist:	all
	if [ ! -d dist ]; then mkdir dist; fi
	if [ ! -d dist/"${DISTNAME}" ]; then \
		mkdir dist/"${DISTNAME}"; \
	fi
	${TAR} cnf - --no-recursion `cat MANIFEST` | \
		(cd dist/"${DISTNAME}"; tar xBpf -)
	(cd dist; ${TAR} cvf - "${DISTNAME}") | \
		${GZIP} --best > "${DISTNAME}.tar.gz"

install::

clean::
	rm -rf dist

distclean::	clean

test check:	missing extras syntax-check
# XXX - Should have a check to make sure that everything in MANIFEST
# is in git.

# Look for files missing from the manifest
missing:
	@git ls-files | \
		perl -lane '$$f=$$F[-1]; print "Missing: $$f" if -f $$f' | \
		fgrep -vf MANIFEST || true

# Look for files in the manifest that don't exist.
# The manifest includes files that are built from other files, hence
# the 'all' dependency.
# XXX - I suppose this really should build the 'dist' directory, then
# match the files in MANIFEST against what's in dist/. That way, we're
# matching against the distro, rather than the source tree.
extras:	all
	@cat MANIFEST | \
	while read fname; do \
		if [ ! -e "$$fname" ]; then \
			echo "$$fname in MANIFEST doesn't exist"; \
		fi \
	done

# Check for syntax errors in PHP files
syntax-check:
	@$(EGREP) '\.(php|inc)$$' MANIFEST | \
	while read fname; do \
		errmsg=`$(PHP) -l "$$fname" 2>&1`; \
		if [ "$$?" != 0 ]; then \
			echo "Syntax error in $$fname:"; \
			echo "$$errmsg"; \
		fi \
	done

# Generate ChangeLog file from the beginning of this year until now
.PHONY:	ChangeLog.git	# Force it to be rebuilt
ChangeLog.git:
	git log 'HEAD@{Jan 1}'.. > $@

# Recursive targets
all install clean distclean depend::
	@for dir in ${RECURSIVE_DIRS}; do \
		(cd "$$dir" && ${MAKE} -${MAKEFLAGS} $@); \
	done

install::
	@echo "Done. Please check your local customization file for updates:"
	@echo "  ${INSTALL_LIB}/config.inc"
	@echo "  ${INSTALL_BACKEND}/.htaccess"

htmlpurifier:	HTMLPurifier/README
HTMLPurifier/README:
	git submodule update HTMLPurifier
