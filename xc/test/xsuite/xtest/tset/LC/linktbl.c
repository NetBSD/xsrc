/* $XConsortium: linktbl.c,v 1.1 94/01/29 16:02:15 rws Exp $ */
#include  "xtest.h"
 
extern struct linkinfo EXFrFntSt;
extern struct linkinfo EXSpprtsLcl;
extern struct linkinfo EXStLclMdfrs;
extern struct linkinfo EXBsFntNmLstO;
extern struct linkinfo EXCntxtDpndnt;
extern struct linkinfo EXmbDrwStr;
extern struct linkinfo EXExtntsOfFnt;
extern struct linkinfo EXLclOfFntSt;
extern struct linkinfo EXmbTxtEscpmn;
extern struct linkinfo EXwcTxtEscpmn;
extern struct linkinfo EXmbTxtExtnts;
extern struct linkinfo EXwcTxtExtnts;
extern struct linkinfo EXmbTxtPrChrE;
extern struct linkinfo EXwcTxtPrChrE;
extern struct linkinfo EXrmStDtbs;
extern struct linkinfo EXwcDrwStr;
extern struct linkinfo EXmbDrwImgStr;
extern struct linkinfo EXwcDrwImgStr;
extern struct linkinfo EXmbDrwTxt;
extern struct linkinfo EXwcDrwTxt;
extern struct linkinfo EXrmGtDtbs;
extern struct linkinfo EXrmLclOfDtbs;
extern struct linkinfo EXScrnRsrcStr;
extern struct linkinfo EXCrtFntSt;
extern struct linkinfo EXFntsOfFntSt;
 
struct linkinfo *linktbl[] = {
    &EXFrFntSt,
    &EXSpprtsLcl,
    &EXStLclMdfrs,
    &EXBsFntNmLstO,
    &EXCntxtDpndnt,
    &EXmbDrwStr,
    &EXExtntsOfFnt,
    &EXLclOfFntSt,
    &EXmbTxtEscpmn,
    &EXwcTxtEscpmn,
    &EXmbTxtExtnts,
    &EXwcTxtExtnts,
    &EXmbTxtPrChrE,
    &EXwcTxtPrChrE,
    &EXrmStDtbs,
    &EXwcDrwStr,
    &EXmbDrwImgStr,
    &EXwcDrwImgStr,
    &EXmbDrwTxt,
    &EXwcDrwTxt,
    &EXrmGtDtbs,
    &EXrmLclOfDtbs,
    &EXScrnRsrcStr,
    &EXCrtFntSt,
    &EXFntsOfFntSt,
    0,
};
