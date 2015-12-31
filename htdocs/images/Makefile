TOP =		../..

GIF_FILES =	$(wildcard *.gif)
JPG_FILES =	$(wildcard *.jpg *.jpeg)

IMAGES =	$(GIF_FILES) $(JPG_FILES)

include $(TOP)/Makefile.common

install::
	mkdir -p "${INSTALL_HTDOCS}/images"
	for i in ${IMAGES}; do \
		install -c -m 444 "$$i" ${INSTALL_HTDOCS}/images/; \
	done
