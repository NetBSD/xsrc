/*
** interpret.c
**
** interprets and executes lines in the Xgc syntax.
*/

#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "xgc.h"
#include "tile"


/* interpret(string)
** -----------------
** Takes string, which is a line written in the xgc syntax, figures
** out what it means, and passes the buck to the right procedure.
** That procedure gets called with feedback set to FALSE; interpret()
** is only called if the user is selecting things interactively.
**
** This procedure will go away when I can figure out how to make yacc
** and lex read from strings as well as files.
*/

void
interpret(const char *string)
{
  char word1[20], word2[80];
  int i;

  sscanf(string,"%s",word1);
  if (!strcmp(word1,"run")) run_test();

  else {
    sscanf(string,"%s %s",word1,word2);
    print_if_recording(string);

    /* So word1 is the first word on the line and word2 is the second.
       Now the fun begins... */

    if (!strcmp(word1,TestStuff.choice.text))  {
      for (i=0;i<NUM_TESTS;++i) {
	if (!strcmp(word2,(TestStuff.data)[i].text)) {
	  change_test((TestStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,FunctionStuff.choice.text)) {
      for (i=0;i<NUM_FUNCTIONS;++i) {
	if (!strcmp(word2,(FunctionStuff.data)[i].text)) {
	  GC_change_function((FunctionStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,LinestyleStuff.choice.text)) {
      for (i=0;i<NUM_LINESTYLES;++i) {
	if (!strcmp(word2,(LinestyleStuff.data)[i].text)) {
	  GC_change_linestyle((LinestyleStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,"linewidth"))
      GC_change_linewidth(atoi(word2),FALSE);
    else if (!strcmp(word1,CapstyleStuff.choice.text)) {
      for (i=0;i<NUM_CAPSTYLES;++i) {
	if (!strcmp(word2,(CapstyleStuff.data)[i].text)) {
	  GC_change_capstyle((CapstyleStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,JoinstyleStuff.choice.text)) {
      for (i=0;i<NUM_JOINSTYLES;++i) {
	if (!strcmp(word2,(JoinstyleStuff.data)[i].text)) {
	  GC_change_joinstyle((JoinstyleStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,FillstyleStuff.choice.text)) {
      for (i=0;i<NUM_FILLSTYLES;++i) {
	if (!strcmp(word2,(FillstyleStuff.data)[i].text)) {
	  GC_change_fillstyle((FillstyleStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,FillruleStuff.choice.text)) {
      for (i=0;i<NUM_FILLRULES;++i) {
	if (!strcmp(word2,(FillruleStuff.data)[i].text)) {
	  GC_change_fillrule((FillruleStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,ArcmodeStuff.choice.text)) {
      for (i=0;i<NUM_ARCMODES;++i) {
	if (!strcmp(word2,(ArcmodeStuff.data)[i].text)) {
	  GC_change_arcmode((ArcmodeStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,"planemask"))
      GC_change_planemask((unsigned long) atoi(word2),FALSE);
    else if (!strcmp(word1,"dashlist"))
      GC_change_dashlist(atoi(word2),FALSE);
    else if (!strcmp(word1,"font"))
      GC_change_font(word2,FALSE);
    else if (!strcmp(word1,"foreground"))
      GC_change_foreground((unsigned long) atoi(word2),FALSE);
    else if (!strcmp(word1,"background"))
      GC_change_background((unsigned long) atoi(word2),FALSE);
    else if (!strcmp(word1,"percent"))
      change_percent(atoi(word2), FALSE);
    else fprintf(stderr,"Ack... %s %s\n",word1,word2);
  }
}

#ifdef notdef
void
interpret(const char *instring)
{
  FILE *inend;

  print_if_recording(instring);
  yyin = outend;
  inend = fdopen(fildes[1],"w");
  fprintf(inend,"%s",instring);
  fclose(inend);
  yyparse();
}
#endif

#define select_correct_button(which,number) \
  select_button(GCdescs[(which)],(number));

/* GC_change_blahzee(foo,feedback)
** ---------------------
** Changes the blahzee field in xgc's GC to foo.  If feedback is TRUE,
** changes the display to reflect this (makes it look like the user
** selected the button, or typed in the text, or whatever).
*/

void
GC_change_function(int function, Boolean feedback)
{
  XSetFunction(X.dpy,X.gc,function);
  X.gcv.function = function;
  if (feedback) select_correct_button(CFunction,function);
}

void
GC_change_foreground(unsigned long foreground, Boolean feedback)
{
  char text[40];

  XSetForeground(X.dpy,X.miscgc,foreground);
  XCopyPlane(X.dpy,X.stipple,X.tile,X.miscgc,0,0,tile_width,tile_height,0,0,1);
  XSetForeground(X.dpy,X.gc,foreground);
  X.gcv.foreground = foreground;
  XSetTile(X.dpy,X.gc,X.tile);
  XSetTile(X.dpy,X.miscgc,X.tile);
  if (feedback) {
    snprintf(text, sizeof text, "%lu",foreground);
    change_text(foregroundtext,text);
  }
}

void
GC_change_background(unsigned long background, Boolean feedback)
{
  char text[40];

  XSetBackground(X.dpy,X.miscgc,background);
  XCopyPlane(X.dpy,X.stipple,X.tile,X.miscgc,0,0,tile_width,tile_height,0,0,1);
  XSetBackground(X.dpy,X.gc,background);
  X.gcv.background = background;
  XSetTile(X.dpy,X.gc,X.tile);
  XSetTile(X.dpy,X.miscgc,X.tile);

  /* Update the background of the test window NOW. */

  XSetWindowBackground(X.dpy,XtWindow(test),background);
  XClearWindow(X.dpy,XtWindow(test));

  if (feedback) {
    snprintf(text, sizeof text, "%lu",background);
    change_text(backgroundtext,text);
  }
}

void
GC_change_linewidth(int linewidth, Boolean feedback)
{
  char text[40];

  X.gcv.line_width = linewidth;
  XChangeGC(X.dpy,X.gc,GCLineWidth,&X.gcv);
  if (feedback) {
    snprintf(text, sizeof text, "%d",linewidth);
    change_text(linewidthtext,text);
  }
}

void
GC_change_linestyle(int linestyle, Boolean feedback)
{
  X.gcv.line_style = linestyle;
  XChangeGC(X.dpy,X.gc,GCLineStyle,&X.gcv);
  if (feedback) select_correct_button(CLinestyle,linestyle);
}

void
GC_change_capstyle(int capstyle, Boolean feedback)
{
  X.gcv.cap_style = capstyle;
  XChangeGC(X.dpy,X.gc,GCCapStyle,&X.gcv);
  if (feedback) select_correct_button(CCapstyle,capstyle);
}

void
GC_change_joinstyle(int joinstyle, Boolean feedback)
{
  X.gcv.join_style = joinstyle;
  XChangeGC(X.dpy,X.gc,GCJoinStyle,&X.gcv);
  if (feedback) select_correct_button(CJoinstyle,joinstyle);
}

void
GC_change_fillstyle(int fillstyle, Boolean feedback)
{
  XSetFillStyle(X.dpy,X.gc,fillstyle);
  X.gcv.fill_style = fillstyle;
  if (feedback) select_correct_button(CFillstyle,fillstyle);
}

void
GC_change_fillrule(int fillrule, Boolean feedback)
{
  XSetFillRule(X.dpy,X.gc,fillrule);
  X.gcv.fill_rule = fillrule;
  if (feedback) select_correct_button(CFillrule,fillrule);
}

void
GC_change_arcmode(int arcmode, Boolean feedback)
{
  XSetArcMode(X.dpy,X.gc,arcmode);
  X.gcv.arc_mode = arcmode;
  if (feedback) select_correct_button(CArcmode,arcmode);
}

/* GC_change_dashlist(dashlist)
** ----------------------------
** Now this one's a bit tricky.  dashlist gets passed in as an int, but we
** want to change it to an array of chars, like the GC likes it.
** For example:
**     119 => XXX_XXX_ => [3,1,3,1]
*/

void
GC_change_dashlist(int dashlist, Boolean feedback)
{
  char dasharray[DASHLENGTH];	/* what we're going to pass to XSetDashes */
  int dashnumber = 0;		/* which element of dasharray we're currently
				   modifying */
  int i;			/* which bit of the dashlist we're on */
  int state = 1;		/* whether the list bit we checked was
				   on (1) or off (0) */

  /* Initialize the dasharray */

  for (i = 0; i < DASHLENGTH; ++i) dasharray[i] = 0;

  if (dashlist == 0) return;	/* having no dashes at all is bogus */

  /* XSetDashes expects the dashlist to start with an on bit, so if it
  ** doesn't, we keep on rotating it until it does */

  while (!(dashlist&1)) dashlist /= 2;

  /* Go through all the bits in dashlist, and update the dasharray
  ** accordingly */

  for (i = 0; i < DASHLENGTH; ++i) {
    /* the following if statements checks to see if the bit we're looking
    ** at as the same on or offness as the one before it (state).  If
    ** so, we increment the length of the current dash. */

    if (((dashlist&1<<i) && state) || (!(dashlist&1<<i) && !state))
      ++dasharray[dashnumber];
    else {
      state = state^1;		/* reverse the state */
      ++dasharray[++dashnumber]; /* start a new dash */
    }
  }

  XSetDashes(X.dpy,X.gc,0,dasharray,dashnumber+1);
  X.gcv.dashes = dashlist;

  if (feedback) update_dashlist(dashlist);
}

void
GC_change_planemask(unsigned long planemask, Boolean feedback)
{
  XSetPlaneMask(X.dpy,X.gc,planemask);
  X.gcv.plane_mask = planemask;
  if (feedback) update_planemask((long)planemask);
}

void
change_test(int test, Boolean feedback)
{
  X.test = test;
  if (feedback) select_button(testchoicedesc,test);
}

void
GC_change_font(char *str, Boolean feedback)
{
  int num_fonts;		/* number of fonts that match the string */

  XListFonts(X.dpy,str,1,&num_fonts); /* see if the font exists */

  if (num_fonts) {
    XSetFont(X.dpy,X.gc,XLoadFont(X.dpy,str));
    if (feedback) change_text(fonttext,str);
  }
}

void
change_percent(int percent, Boolean feedback)
{
  /* Make sure that percent is valid */

  if (percent < 1 || percent > 100) return;

  X.percent = (float) percent / 100.0;

  if (feedback) update_slider(percent);
}
