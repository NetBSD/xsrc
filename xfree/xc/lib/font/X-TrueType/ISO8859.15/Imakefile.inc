XCOMM $XFree86$

#define ModuleName ISO8859_15
MODULENAME = ModuleName
MODULESRCDIR= $(XTTSOURCEDIR)/ISO8859.15

SRCS =  main.c
OBJS =  main.o

LinkSourceFile(main.c,$(MODULESRCDIR))
