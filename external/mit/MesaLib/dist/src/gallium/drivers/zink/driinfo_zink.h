// zink specific driconf options

DRI_CONF_SECTION_DEBUG
   DRI_CONF_DUAL_COLOR_BLEND_BY_LOCATION(false)
   DRI_CONF_OPT_B(radeonsi_inline_uniforms, false, "Optimize shaders by replacing uniforms with literals")
DRI_CONF_SECTION_END

DRI_CONF_SECTION_PERFORMANCE

DRI_CONF_SECTION_END
