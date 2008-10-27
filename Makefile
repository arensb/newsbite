# Variables
PROJECT =	newsbite
#VERSION =	1.1.9
# XXX - The "!=" is a BSDmake-ism.
REV !=		svn status -uq | grep "Status against revision:"|awk '{print $$4}'
# In GNU make, one would write:
#REV :=		$(shell svn status -uq | grep "Status against revision:"|awk '{print $$4}')
#DISTNAME =	${PROJECT}-${VERSION}
DISTNAME =	${PROJECT}-r${REV}

# Commands
TAR =	tar
GZIP =	gzip

.PHONY:	dist

all:

dist:
	if [ ! -d dist ]; then mkdir dist; fi
	if [ ! -d dist/"${DISTNAME}" ]; then \
		mkdir dist/"${DISTNAME}"; \
	fi
	${TAR} cnf - `cat MANIFEST` | \
		(cd dist/"${DISTNAME}"; tar xBpf -)
	chgrp www "dist/${DISTNAME}/htdocs/smarty/templates_c"
	chmod g+w "dist/${DISTNAME}/htdocs/smarty/templates_c"
	(cd dist; tar cvf - "${DISTNAME}") | \
		${GZIP} --best > "${DISTNAME}.tar.gz"

clean::
	rm -rf dist

# Look for files missing from the manifest
missing:
	@svn status -qv | \
		perl -lane '$$f=$$F[-1]; print $$f if -f $$f' | \
		fgrep -vf MANIFEST || true
