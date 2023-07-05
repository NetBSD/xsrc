# Test various rlayout-related bits

# Bits directly testing the funcs
ctwm_simple_unit_test(rlayout_overlap
	BIN test_layout
	ARGS overlap.tst
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

ctwm_simple_unit_test(rlayout_test
	BIN test_layout
	ARGS test.tst
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)


# Test config parsing of MonitorLayout{} block
ctwm_simple_unit_test(rlayout_monitor_layout
	BIN test_monitor_layout
	ARGS -f ${CMAKE_CURRENT_SOURCE_DIR}/monitor_layout.ctwmrc)
