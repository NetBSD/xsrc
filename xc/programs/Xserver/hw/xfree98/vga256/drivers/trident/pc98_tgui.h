/* $XFree86: xc/programs/Xserver/hw/xfree98/vga256/drivers/trident/pc98_tgui.h,v 3.1 1996/09/29 13:47:46 dawes Exp $ */

typedef enum { PC98Unknown , PC98PCIBus , PC98CBus } PC98BusType;
typedef enum { PC98NoExist , PC98NEC9680 , PC98NEC9320
		 , PC98DRV9680 } PC98TGUiType;

typedef struct {
  PC98TGUiType  TGUiType;
  PC98BusType   BusType;
  unsigned long vramBase;
  unsigned long mmioBase;
  unsigned char DRAMspeed;
  int MCLK;
  int Bpp_Clocks[4];
  void (*crtsw)(short);
  Bool (*chiptest)();
} PC98TGUiTable;
