XCOMM $XFree86$

#define ModuleName BIG5
MODULENAME = ModuleName
MODULESRCDIR = $(XTTSOURCEDIR)/$(MODULENAME)

SRCS =  main.c BIG5toUCS2.c
OBJS =  main.o BIG5toUCS2.o

LinkSourceFile(main.c,$(MODULESRCDIR))
LinkSourceFile(BIG5toUCS2.c,$(MODULESRCDIR))

