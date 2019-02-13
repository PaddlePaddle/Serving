/* -*- c++ -*-
   copy[write] by dirlt(zhang_yan@baidu.com)
   date time:Mon Nov 17 17:08:24 CST 2008
   file name:main.cpp */

#include <cstdio>
#include <iostream>
#include "idl_conf_if.h"
using namespace confIDL;

int main(int argc,char **argv)
{
  int i;
  char *file;
  idl_t *idl_data;
  for(i=1;i<argc;i++){
    file=argv[i];
    idl_data=alloc_idl();
    load_idl(file,idl_data);
    fprintf(stderr,"********************IDL file '%s' structure********************\n",file);
    print_idl(idl_data);
    free_idl(idl_data);
  }
  return 0;
}
  
  


