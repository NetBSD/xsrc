NVC0_SHADERS = xfrm2nvc0.vpc \
	       exascnvc0.fpc \
	       exacmnvc0.fpc \
	       exacanvc0.fpc \
	       exasanvc0.fpc \
	       exas8nvc0.fpc \
	       exac8nvc0.fpc \
	       videonvc0.fpc
NVE0_SHADERS = xfrm2nve0.vpc \
	       exascnve0.fpc \
	       exacmnve0.fpc \
	       exacanve0.fpc \
	       exasanve0.fpc \
	       exas8nve0.fpc \
	       exac8nve0.fpc \
	       videonve0.fpc
NVF0_SHADERS = xfrm2nvf0.vpc \
	       exascnvf0.fpc \
	       exacmnvf0.fpc \
	       exacanvf0.fpc \
	       exasanvf0.fpc \
	       exas8nvf0.fpc \
	       exac8nvf0.fpc \
	       videonvf0.fpc

SHADERS = $(NVC0_SHADERS) $(NVE0_SHADERS) $(NVF0_SHADERS)

all: $(SHADERS)

$(filter %nvc0.vpc,$(SHADERS)): %.vpc: %.vp
	cpp -DENVYAS $< | sed -e '/^#/d' | envyas -w -m nvc0 -o $@
$(filter %nvc0.fpc,$(SHADERS)): %.fpc: %.fp
	cpp -DENVYAS $< | sed -e '/^#/d' | envyas -w -m nvc0 -o $@

$(filter %nve0.vpc,$(SHADERS)): %.vpc: %.vp
	cpp -DENVYAS $< | sed -e '/^#/d' | envyas -w -m nvc0 -V nve4 -o $@
$(filter %nve0.fpc,$(SHADERS)): %.fpc: %.fp
	cpp -DENVYAS $< | sed -e '/^#/d' | envyas -w -m nvc0 -V nve4 -o $@

$(filter %nvf0.vpc,$(SHADERS)): %.vpc: %.vp
	cpp -DENVYAS $< | sed -e '/^#/d' | envyas -w -m gk110 -o $@
$(filter %nvf0.fpc,$(SHADERS)): %.fpc: %.fp
	cpp -DENVYAS $< | sed -e '/^#/d' | envyas -w -m gk110 -o $@
