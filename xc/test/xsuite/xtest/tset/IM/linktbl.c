/* $XConsortium: linktbl.c,v 1.1 94/01/29 16:06:56 rws Exp $ */
#include  "xtest.h"
 
extern struct linkinfo EXClsIM;
extern struct linkinfo EXGtIMVls;
extern struct linkinfo EXLclOfIM;
extern struct linkinfo EXOpnIM;
extern struct linkinfo EXDsplyOfIM;
extern struct linkinfo EXDstryIC;
extern struct linkinfo EXCrtIC;
extern struct linkinfo EXIMOfIC;
extern struct linkinfo EXStICVls;
extern struct linkinfo EXGtICVls;
extern struct linkinfo EXFltrEvnt;
extern struct linkinfo EXmbLkpStr;
extern struct linkinfo EXwcLkpStr;
extern struct linkinfo EXVCrtNstdLst;
 
struct linkinfo *linktbl[] = {
    &EXClsIM,
    &EXGtIMVls,
    &EXLclOfIM,
    &EXOpnIM,
    &EXDsplyOfIM,
    &EXDstryIC,
    &EXCrtIC,
    &EXIMOfIC,
    &EXStICVls,
    &EXGtICVls,
    &EXFltrEvnt,
    &EXmbLkpStr,
    &EXwcLkpStr,
    &EXVCrtNstdLst,
    0,
};
