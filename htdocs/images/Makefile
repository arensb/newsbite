TOP =		../..

GIF_FILES =	$(wildcard *.gif)
JPG_FILES =	$(wildcard *.jpg *.jpeg)
PNG_FILES =	$(wildcard *.png)

IMAGES =	$(GIF_FILES) $(JPG_FILES) $(PNG_FILES)

include $(TOP)/Makefile.common

install::
	mkdir -p "${INSTALL_HTDOCS}/images"
	for i in ${IMAGES}; do \
		install -c -m 444 "$$i" ${INSTALL_HTDOCS}/images/; \
	done
