#
# Tell the builder various stuff about what we decided.
#


# Describe install paths
message(STATUS "System-wide config in      ${ETCDIR}")
message(STATUS "Installing ctwm in         ${BINDIR}")
message(STATUS "Installing runtime data in ${DATADIR}")
message(STATUS "Installing docs in         ${DOCDIR}")
message(STATUS "Installing examples in     ${EXAMPLEDIR}")
message(STATUS "Installing manpage to      ${MAN1PATH}")


message(STATUS "Building for ctwm version ${ctwm_version_str}")
