/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/linux/lnx_axp.c,v 1.4 2001/02/15 19:46:03 eich Exp $ */

#include <stdio.h>
#include "X.h"
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Axp.h"

axpDevice lnxGetAXP(void);

typedef struct 
 { char* sysName; 
   char* sysVari; 
   char* cpu; 
   axpDevice sys; }
AXP;

static AXP axpList[] = {
  { "Tsunami", NULL, NULL, TSUNAMI },
  { "Eiger", NULL, NULL, TSUNAMI }, 
  {"Noname", NULL, NULL, LCA },
  { "AlphaBook1", NULL, NULL, LCA }, 
  {"EB66", NULL, NULL, LCA}, 
  {"EB64+",NULL,NULL, APECS}, 
  {"Noritake",NULL,"EV5",CIA},
  {"Noritake",NULL,"EV56",CIA},
  {"Noritake",NULL,NULL,APECS},
  {"XL",NULL,NULL,APECS},              
  {"Avanti",NULL,NULL,APECS},
  {"Mikasa",NULL,"EV5",CIA},
  {"Mikasa",NULL,"EV56",CIA},
  {"Mikasa",NULL,NULL,APECS},
  {"EB164","EB164",NULL,CIA},
  {"EB164","PC164", NULL,CIA},
  {"EB164","LX164",NULL, PYXIS},
  {"EB164","SX164",NULL, PYXIS},
  {"EB164","RX164",NULL, POLARIS},
  {"Alcor",NULL,NULL,CIA},
  {"Takara",NULL,NULL,CIA},
  {"Sable",NULL, "EV5",T2_GAMMA},
  {"Sable",NULL,"EV56",T2_GAMMA},
  {"Sable",NULL,NULL,T2},
  {"Rawhide",NULL,NULL,MCPCIA},
  {"Jensen",NULL,NULL,JENSEN},
  {"Miata",NULL,NULL,PYXIS_CIA},
  {"Ruffian",NULL,NULL,PYXIS_CIA},
  {"Nautilus",NULL,NULL,IRONGATE},
  {NULL,NULL,NULL,NONE}
};


axpDevice
lnxGetAXP(void)
{
  FILE *file;
  int count = 0;
  char res[256];
  char cpu[255];
  char systype[255];
  char sysvari[255];
  if (!(file = fopen("/proc/cpuinfo","r")))
    return SYS_NONE;
  do {
    if (!fgets(res,0xff,file)) return SYS_NONE;
    switch (count) {
    case 1:
      sscanf(res, "cpu model : %s",cpu);
#ifdef DEBUG
      ErrorF("CPU %s\n",cpu);
#endif
      break;
    case 5:
      sscanf(res, "system type : %s",systype);
#ifdef DEBUG
      ErrorF("system type : %s\n",systype);
#endif
      break;
    case 6:
      sscanf(res, "system variation : %s",sysvari);
#ifdef DEBUG
      ErrorF("system variation: %s\n",sysvari);
#endif
      break;
    }
    count++;
  } while (count < 8);
  
  fclose(file);
  
  count = 0;
  
  do {
    if (!axpList[count].sysName || !strcmp(axpList[count].sysName,systype)) {
      if (axpList[count].sysVari && strcmp(axpList[count].sysVari,sysvari)) {
	count++;
	continue;
      };
      if (axpList[count].cpu && strcmp(axpList[count].cpu,cpu)) {
	count++;
	continue;
      }
      return axpList[count].sys;
    } 
	count++;
  } while (1);
}

