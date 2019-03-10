all: html
install-data-local: install-html

EXTRA_DIST = \
    Doxyfile \
    vdpau_data_flow.png

VDPAU_HEADER_DIR := $(top_srcdir)/include
VDPAU_HEADER_FILES := \
    $(VDPAU_HEADER_DIR)/vdpau/vdpau.h \
    $(VDPAU_HEADER_DIR)/vdpau/vdpau_x11.h

export VDPAU_HEADER_DIR
export VDPAU_HEADER_FILES
html-out/%: Doxyfile $(VDPAU_HEADER_FILES)
	$(DOXYGEN) $<

if ENABLE_DOCS
html: html-out/index.html
install-html-local:
	$(install_sh) -d "$(DESTDIR)$(docdir)/html"
	$(install_sh) -m 644 html-out/* "$(DESTDIR)$(docdir)/html"
uninstall-local:
	$(RM) -r "$(DESTDIR)$(docdir)/html"
endif

clean-local:
	$(RM) -r html-out
