/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STRING = 258,
     NUMBER = 259,
     RUN = 260,
     FUNCTION = 261,
     FUNCTIONTYPE = 262,
     TEST = 263,
     TESTTYPE = 264,
     LINESTYLE = 265,
     LINESTYLETYPE = 266,
     CAPSTYLE = 267,
     CAPSTYLETYPE = 268,
     JOINSTYLE = 269,
     JOINSTYLETYPE = 270,
     ROUND = 271,
     SOLID = 272,
     FILLSTYLE = 273,
     FILLSTYLETYPE = 274,
     FILLRULE = 275,
     FILLRULETYPE = 276,
     ARCMODE = 277,
     ARCMODETYPE = 278,
     FOREGROUND = 279,
     BACKGROUND = 280,
     LINEWIDTH = 281,
     PLANEMASK = 282,
     DASHLIST = 283,
     PERCENT = 284,
     FONT = 285
   };
#endif
/* Tokens.  */
#define STRING 258
#define NUMBER 259
#define RUN 260
#define FUNCTION 261
#define FUNCTIONTYPE 262
#define TEST 263
#define TESTTYPE 264
#define LINESTYLE 265
#define LINESTYLETYPE 266
#define CAPSTYLE 267
#define CAPSTYLETYPE 268
#define JOINSTYLE 269
#define JOINSTYLETYPE 270
#define ROUND 271
#define SOLID 272
#define FILLSTYLE 273
#define FILLSTYLETYPE 274
#define FILLRULE 275
#define FILLRULETYPE 276
#define ARCMODE 277
#define ARCMODETYPE 278
#define FOREGROUND 279
#define BACKGROUND 280
#define LINEWIDTH 281
#define PLANEMASK 282
#define DASHLIST 283
#define PERCENT 284
#define FONT 285




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 18 "gram.y"
{
  int num;
  char *ptr;
}
/* Line 1489 of yacc.c.  */
#line 114 "gram.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

