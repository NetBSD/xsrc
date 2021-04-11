# This just shortcuts stuff through to cmake
client all build ctwm: ../build/Makefile
	( cd ../build && ${MAKE} ${@} )

../build/Makefile cmake: ../CMakeLists.txt
	( cd .. && ${MAKE} cmake )

clean allclean:
	( cd .. && ${MAKE} ${@} )


# Reindent files
indent:
	astyle -n --options=../tools/ctwm.astyle *.h *.c
