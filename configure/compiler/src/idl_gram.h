/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     KW_OVERWRITE = 258,
     KW_STRUCT = 259,
     KW_RAW = 260,
     KW_CHAR = 261,
     KW_UCHAR = 262,
     KW_INT8 = 263,
     KW_UINT8 = 264,
     KW_INT16 = 265,
     KW_UINT16 = 266,
     KW_INT32 = 267,
     KW_UINT32 = 268,
     KW_INT64 = 269,
     KW_UINT64 = 270,
     KW_FLOAT = 271,
     KW_DOUBLE = 272,
     KW_STRING = 273,
     INTEGER = 274,
     ID = 275,
     LITERAL_STRING = 276,
     DOUBLE = 277,
     VAR_CMD = 278
   };
#endif
#define KW_OVERWRITE 258
#define KW_STRUCT 259
#define KW_RAW 260
#define KW_CHAR 261
#define KW_UCHAR 262
#define KW_INT8 263
#define KW_UINT8 264
#define KW_INT16 265
#define KW_UINT16 266
#define KW_INT32 267
#define KW_UINT32 268
#define KW_INT64 269
#define KW_UINT64 270
#define KW_FLOAT 271
#define KW_DOUBLE 272
#define KW_STRING 273
#define INTEGER 274
#define ID 275
#define LITERAL_STRING 276
#define DOUBLE 277
#define VAR_CMD 278




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 40 "idl.gram"
typedef union YYSTYPE {
  meta_t *meta;
  var_t *var;
  group_t *grp;
  cf_list_t *cf_list;
  meta_list_t *meta_list;
  var_list_t *var_list;
  cf_t *cf;
} YYSTYPE;
/* Line 1275 of yacc.c.  */
#line 93 "idl_gram.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





