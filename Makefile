# Variables
PROJECT =	newsbite
#VERSION =	1.1.9

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
REV !=		${REV_CMD}

#DISTNAME =	${PROJECT}-${VERSION}
DISTNAME =	${PROJECT}-r${REV}

# Commands
TAR =	tar
GZIP =	gzip

.PHONY:	dist

all::
	(cd htdocs/skins/fancy && $(MAKE))

dist:	all
	if [ ! -d dist ]; then mkdir dist; fi
	if [ ! -d dist/"${DISTNAME}" ]; then \
		mkdir dist/"${DISTNAME}"; \
	fi
	${TAR} cnf - --no-recursion `cat MANIFEST` | \
		(cd dist/"${DISTNAME}"; tar xBpf -)
	(cd dist; tar cvf - "${DISTNAME}") | \
		${GZIP} --best > "${DISTNAME}.tar.gz"

clean::
	rm -rf dist

check:	missing syntax-check

# Look for files missing from the manifest
missing:
	@svn status -qv | \
		perl -lane '$$f=$$F[-1]; print "Missing: $$f" if -f $$f' | \
		fgrep -vf MANIFEST || true

# Check for syntax errors in PHP files
# XXX - Bleah. This randomly dumps core. Why?
syntax-check:
	@$(EGREP) '\.(php|inc)$$' MANIFEST | \
	while read fname; do \
		echo "Checking $$fname"; \
		$(PHP) -l "$$fname" || exit $?; \
	done

# Generate ChangeLog file from the beginning of this year until now
ChangeLog.svn:
	svn log -v -rhead:\{`date +%Y-01-01`\} > $@

# Recursive targets
all clean depend::
	(cd htdocs/skins/fancy && $(MAKE) -$(MAKEFLAGS) $@)
