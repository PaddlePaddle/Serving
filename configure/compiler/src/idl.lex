/* -*- c++ -*-
   copy[write] by dirlt(zhang_yan@baidu.com)
   date time:Mon Nov 17 12:06:46 CST 2008
   file name:idl.lex */
%{
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "idl.h"
#include "idl_gram.h"

#define MAX_TEMP_BUFFER 2048
  
#define YY_DECL int cfgidllex(YYSTYPE *yylval,yyscan_t yyscanner,meta_t *loc,idl_t *idl)
  static void do_keyword(YYSTYPE *yylval,yyscan_t cfgidlscanner,meta_t *loc,idl_t *idl);
  static int do_special_cmd(YYSTYPE *yylval,yyscan_t cfgidlscanner,meta_t *loc,idl_t *idl);
  static void do_meta_data(YYSTYPE *yylval,yyscan_t cfgidlscanner,meta_t *loc,idl_t *idl);
  static void do_comment(yyscan_t cfgidlscanner,meta_t *loc);
  static void update(yyscan_t cfgidlscanner,meta_t *loc, idl_t *idl);
%}

/* we don't need to handle file more than one */
%option noyywrap 
/* give out warnings */
%option warn
/* using 8 bit a character */
%option 8bit

/* we want idllex this class */
%option prefix="cfgidl"
/* output file is idl_lex.c */
%option outfile="idl_lex.c"
/* header file is idl_lex.h */
%option header-file="idl_lex.h"
%option reentrant

/* the letter is so generous except
   (),whitespace and ; */
letter [a-zA-Z_]
digit [0-9]

%%

"#" {do_comment(yyscanner,loc);}
"raw" {do_keyword(yylval,yyscanner,loc,idl);return KW_RAW;}
"char" {do_keyword(yylval,yyscanner,loc,idl);return KW_CHAR;}
"uchar" {do_keyword(yylval,yyscanner,loc,idl);return KW_UCHAR;}
"int8_t" {do_keyword(yylval,yyscanner,loc,idl);return KW_INT8;}
"uint8_t" {do_keyword(yylval,yyscanner,loc,idl);return KW_UINT8;}
"int16_t" {do_keyword(yylval,yyscanner,loc,idl);return KW_INT16;}
"uint16_t" {do_keyword(yylval,yyscanner,loc,idl);return KW_UINT16;}
"int32_t" {do_keyword(yylval,yyscanner,loc,idl);return KW_INT32;}
"uint32_t" {do_keyword(yylval,yyscanner,loc,idl);return KW_UINT32;}
"int64_t" {do_keyword(yylval,yyscanner,loc,idl);return KW_INT64;}
"uint64_t" {do_keyword(yylval,yyscanner,loc,idl);return KW_UINT64;}
"float" {do_keyword(yylval,yyscanner,loc,idl);return KW_FLOAT;}
"double" {do_keyword(yylval,yyscanner,loc,idl);return KW_DOUBLE;}
"string" {do_keyword(yylval,yyscanner,loc,idl);return KW_STRING;}
"struct" {do_keyword(yylval,yyscanner,loc,idl);return KW_STRUCT;}
"overwrite" {do_keyword(yylval,yyscanner,loc,idl);return KW_OVERWRITE;}

\/\/[^\n]* {
	if (do_special_cmd(yylval,yyscanner,loc,idl) != 0) {
		return VAR_CMD;
	}
}
\"[^\"]*\" {do_meta_data(yylval,yyscanner,loc,idl);return LITERAL_STRING;}
[+-]?({digit})+ {do_meta_data(yylval,yyscanner,loc,idl);return INTEGER;}
[+-]?((({digit})+)|(\.({digit})+)|((({digit})+)\.(({digit})+)))([Ee]({digit})+)? {do_meta_data(yylval,yyscanner,loc,idl);return DOUBLE;}
{letter}({letter}|{digit})* {do_meta_data(yylval,yyscanner,loc,idl);return ID;}
[ \t\n\f]+ {update(yyscanner,loc, idl);}
. {return yytext[0];}

%%
void do_keyword(YYSTYPE *yylval,yyscan_t scanner,meta_t *loc,idl_t *idl)
{
  char *text=cfgidlget_text(scanner);
#ifdef DEBUG
  printf("call %s,%s\n",__func__,text);
#endif
  meta_t *meta=alloc_meta(idl);
  meta->file=loc->file;
  meta->lineno=loc->lineno;
  meta->data=text;
  yylval->meta=meta;
}

void do_meta_data(YYSTYPE *yylval,yyscan_t scanner,meta_t *loc,idl_t *idl)
{
  char *text=cfgidlget_text(scanner);
  int leng=cfgidlget_leng(scanner);
#ifdef DEBUG
  printf("call %s,%s\n",__func__,text);
#endif
  meta_t *meta=alloc_meta(idl);
  meta->file=loc->file;
  meta->lineno=loc->lineno;
  meta->data=cfgidlget_text(scanner);
  yylval->meta=meta;

  int i;
  for(i=0;i<leng;i++){
    if(text[i]=='\n')
      loc->lineno++;
  }
  return ;
}

int do_special_cmd(YYSTYPE *yylval,yyscan_t scanner,meta_t *loc,idl_t *idl)
{
  char *text = cfgidlget_text(scanner);
  meta_t *meta=alloc_meta(idl);
  meta->file=loc->file;
  meta->lineno=loc->lineno;
  meta->data=text;
  yylval->meta=meta;

  loc->lineno++;

  int ret = idl->cmdreserve;
  idl->cmdreserve = 0;
  return ret;
}

  
static int cdirective(char *tmp,int *lineno,char **filename)
{
  int i,j;
  if(tmp[0]!='#' || tmp[1]!=' ' || !isdigit(tmp[2]))return 0;
  i=2;
  while(isdigit(tmp[i]))i++;
  tmp[i++]=0;
  *lineno=atoi(tmp+2);

  j=i+1;
  if(tmp[i]!='"')return 0;
  i++;
  while(tmp[i]!='"')i++;
  tmp[i]=0;
  *filename=tmp+j;

  return 1;
}

void do_comment(yyscan_t scanner,meta_t *loc)
{
  char ch;
  char tmp[MAX_TEMP_BUFFER];
  int i;

  i=0;
  if(cfgidlget_text(scanner)[0]=='#'){
    tmp[0]='#';
    i=1;
  }
  while(1){
    ch=yyinput(scanner);
    if(ch=='\n' || ch==EOF)break;
    if(i < (int)sizeof(tmp) - 1) {
		tmp[i++]=ch;
	}
  }
  tmp[i]=0;
  loc->lineno++;

  char *tmp_file;
  int tmp_lineno;
  if(cdirective(tmp,&tmp_lineno,&tmp_file)==1){
    loc->lineno=tmp_lineno;
    loc->file=tmp_file;
#ifdef DEBUG
      printf("new file:%s,lineno:%d\n",
	     loc->file.c_str(),
	     loc->lineno);
#endif
  }
  return ;
}

void update(yyscan_t scanner,meta_t *loc, idl_t *idl)
{
  int i;
  int leng=cfgidlget_leng(scanner);
  char *text=cfgidlget_text(scanner);
  int cnt = 0;
  for(i=0;i<leng;i++){
    if(text[i]=='\n') {
      loc->lineno++;
	  //printf("update[%d]: %c\n", i, text[i]);
	  ++ cnt;
	  }
  }
  if (cnt > 0) {
  	idl->cmdreserve = 0;
  }
  return ;
}
