XCOMM $XFree86: xc/lib/font/X-TrueType/BIG5/Imakefile.inc,v 1.1 1999/03/28 15:31:39 dawes Exp $

#define ModuleName GEORGIAN
MODULENAME = ModuleName
MODULESRCDIR = $(XTTSOURCEDIR)/$(MODULENAME)

SRCS =  main.c GEORGIANtoUCS2.c
OBJS =  main.o GEORGIANtoUCS2.o

LinkSourceFile(main.c,$(MODULESRCDIR))
LinkSourceFile(GEORGIANtoUCS2.c,$(MODULESRCDIR))

