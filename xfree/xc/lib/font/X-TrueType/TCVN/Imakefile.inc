XCOMM $XFree86$

#define ModuleName TCVN
MODULENAME = ModuleName
MODULESRCDIR= $(XTTSOURCEDIR)/$(MODULENAME)

SRCS =  main.c TCVNtoUCS2.c
OBJS =  main.o TCVNtoUCS2.o

LinkSourceFile(main.c,$(MODULESRCDIR))
LinkSourceFile(TCVNtoUCS2.c,$(MODULESRCDIR))
