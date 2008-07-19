# Variables
PROJECT =	newsbite
VERSION =	0.1.3

# Commands
TAR =	tar
GZIP =	gzip

.PHONY:	dist

all:

dist:
	if [ ! -d dist ]; then mkdir dist; fi
	if [ ! -d dist/"${PROJECT}-${VERSION}" ]; then \
		mkdir dist/"${PROJECT}-${VERSION}"; \
	fi
	${TAR} cnf - `cat MANIFEST` | \
		(cd dist/"${PROJECT}-${VERSION}"; tar xBpf -)
	(cd dist; tar cvf - "${PROJECT}-${VERSION}") | \
		${GZIP} --best > "${PROJECT}-${VERSION}.tar.gz"

clean::
	rm -r dist
