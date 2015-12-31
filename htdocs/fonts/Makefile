TOP =		../..
FONT_FILES =	$(wildcard *.woff)

include $(TOP)/Makefile.common

install::
	mkdir -p "${INSTALL_HTDOCS}/fonts"
	for i in ${FONT_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_HTDOCS}/fonts; \
	done
