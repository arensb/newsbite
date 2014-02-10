TOP =		../..
JQUERY =	jquery-2.1.0.min.js

include $(TOP)/Makefile.common

install::
	mkdir -p "${INSTALL_BACKEND}/js"
	install -c -m 444 "${JQUERY}" ${INSTALL_BACKEND}/js/jquery.js
	if [ x"${COMPRESS_JS}" = xyes ]; then \
		${GZIP} -f "${INSTALL_BACKEND}/js/jquery.js"; \
	fi
