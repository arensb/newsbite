TOP =		../..
CSS_FILES =	$(wildcard *.css)

include $(TOP)/Makefile.common

install::
	mkdir -p "${INSTALL_HTDOCS}/css"
	for i in ${CSS_FILES}; do \
		install -c -m 444 "$$i" ${INSTALL_HTDOCS}/css; \
		if [ x"${COMPRESS_CSS}" = xyes ]; then \
			${GZIP} -f "${INSTALL_HTDOCS}/css/$$i"; \
		fi; \
	done
