TOP =		..

# Where to install jQuery UI.
JQUI_DEST =	"${INSTALL_BACKEND}/js/jquery-ui"

# Which jQuery UI files to install:
JQUI_JS_FILES = \
	jquery-ui/jquery-ui.min.js \

JQUI_CSS_FILES = \
	jquery-ui/jquery-ui.min.css \

JQUI_IMG_FILES = \
	jquery-ui/external/jquery/jquery.js \
	jquery-ui/images/ui-bg_flat_0_aaaaaa_40x100.png \
	jquery-ui/images/ui-bg_flat_75_ffffff_40x100.png \
	jquery-ui/images/ui-bg_glass_55_fbf9ee_1x400.png \
	jquery-ui/images/ui-bg_glass_65_ffffff_1x400.png \
	jquery-ui/images/ui-bg_glass_75_dadada_1x400.png \
	jquery-ui/images/ui-bg_glass_75_e6e6e6_1x400.png \
	jquery-ui/images/ui-bg_glass_95_fef1ec_1x400.png \
	jquery-ui/images/ui-bg_highlight-soft_75_cccccc_1x100.png \
	jquery-ui/images/ui-icons_222222_256x240.png \
	jquery-ui/images/ui-icons_2e83ff_256x240.png \
	jquery-ui/images/ui-icons_454545_256x240.png \
	jquery-ui/images/ui-icons_888888_256x240.png \
	jquery-ui/images/ui-icons_cd0a0a_256x240.png \
	jquery-ui/index.html \
	jquery-ui/jquery-ui.css \
	jquery-ui/jquery-ui.js \
	jquery-ui/jquery-ui.min.css \
	jquery-ui/jquery-ui.min.js \
	jquery-ui/jquery-ui.structure.css \
	jquery-ui/jquery-ui.structure.min.css \
	jquery-ui/jquery-ui.theme.css \
	jquery-ui/jquery-ui.theme.min.css

# Regular jQuery UI:
#	jquery-ui/images/ui-bg_diagonals-thick_18_b81900_40x40.png \
#	jquery-ui/images/ui-bg_diagonals-thick_20_666666_40x40.png \
#	jquery-ui/images/ui-bg_flat_10_000000_40x100.png \
#	jquery-ui/images/ui-bg_glass_100_f6f6f6_1x400.png \
#	jquery-ui/images/ui-bg_glass_100_fdf5ce_1x400.png \
#	jquery-ui/images/ui-bg_glass_65_ffffff_1x400.png \
#	jquery-ui/images/ui-bg_gloss-wave_35_f6a828_500x100.png \
#	jquery-ui/images/ui-bg_highlight-soft_100_eeeeee_1x100.png \
#	jquery-ui/images/ui-bg_highlight-soft_75_ffe45c_1x100.png \
#	jquery-ui/images/ui-icons_222222_256x240.png \
#	jquery-ui/images/ui-icons_228ef1_256x240.png \
#	jquery-ui/images/ui-icons_ef8c08_256x240.png \
#	jquery-ui/images/ui-icons_ffd27a_256x240.png \
	jquery-ui/images/ui-icons_ffffff_256x240.png \

#	jquery-ui/index.html \
#	jquery-ui/jquery-ui.css \
#	jquery-ui/jquery-ui.js \
#	jquery-ui/jquery-ui.structure.css \
#	jquery-ui/jquery-ui.structure.min.css \
#	jquery-ui/jquery-ui.theme.css \
#	jquery-ui/jquery-ui.theme.min.css \


include $(TOP)/Makefile.common

install::
	mkdir -p "${JQUI_DEST}"
	for i in ${JQUI_JS_FILES}; do \
		install -c -m 444 "$$i" "${JQUI_DEST}"; \
		if [ x"${COMPRESS_JS}" = xyes ]; then \
			${GZIP} -f "${JQUI_DEST}/`basename $$i`"; \
		fi \
	done
	for i in ${JQUI_CSS_FILES}; do \
		install -c -m 444 "$$i" "${JQUI_DEST}"; \
		if [ x"${COMPRESS_CSS}" = xyes ]; then \
			${GZIP} -f "${JQUI_DEST}/`basename $$i`"; \
		fi \
	done
	mkdir -p "${JQUI_DEST}/images"
	for i in ${JQUI_IMG_FILES}; do \
		install -c -m 444 "$$i" "${JQUI_DEST}/images"; \
	done
