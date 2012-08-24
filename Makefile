# Variables
PROJECT =	newsbite

PHP =		php
EGREP =		egrep

# REV_CMD: command to figure out which svn revision we're using.
REV_CMD =	svn status -uq | grep "Status against revision:"|awk '{print $$4}'

# Hack: BSD make uses "VAR != cmd" to assign $VAR the output of `cmd',
# while GNU make uses "VAR = $(shell cmd)". Having things in this
# order makes it work. I think it's because GNU make doesn't recognize
# "!=", so it doesn't see the second assignment, while BSD make does,
# so it ignores the first assignment.
REV =		$(shell ${REV_CMD})

DISTNAME =	${PROJECT}-r${REV}

# Commands
TAR =	tar
GZIP =	gzip

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
	htdocs/skins/fancy \
	lib \
	plugins

# XXX - Create htdocs/.htaccess based on the directories above
# XXX - Create lib/config.inc based on the directories above

.PHONY:	dist

all::

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
# is in svn.

# Look for files missing from the manifest
missing:
	@svn status -qv | \
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
ChangeLog.svn:
	svn log -v -rhead:\{`date +%Y-01-01`\} > $@

# Recursive targets
all install clean distclean depend::
	@for dir in ${RECURSIVE_DIRS}; do \
		(cd "$$dir" && ${MAKE} -${MAKEFLAGS} $@); \
	done
