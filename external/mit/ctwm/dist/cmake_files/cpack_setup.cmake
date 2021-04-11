#
# Setup cpack stuff for packaging
#

# Basic stuff
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A window manager for the X Window System.")
set(CPACK_PACKAGE_VENDOR "ctwm")
set(CPACK_PACKAGE_CONTACT "ctwm@ctwm.org")
set(CPACK_PACKAGE_RELOCATABLE OFF)
set(CPACK_PACKAGE_VERSION ${ctwm_version_str})

# Per-packaging-system stuff
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "User Interface/X")
set(CPACK_RPM_PACKAGE_REQUIRES "m4")
set(CPACK_RPM_PACKAGE_URL "https://www.ctwm.org/")

set(CPACK_DEBIAN_PACKAGE_SECTION "x11")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "m4")

include(CPack)
