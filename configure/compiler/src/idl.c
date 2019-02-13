/* -*- c++ -*-
   copy[write] by dirlt(zhang_yan@baidu.com)
   date time:Wed Nov 19 23:43:27 CST 2008
   file name:idl.c */


/**
 * @file idl.c
 * @author zhang_yan@baidu.com
 * @date Wed Nov 19 23:43:27 CST 2008
 * @version
 * @brief the implementation of internal IDL compiler,contains some basic routines
 */

#include <stdio.h>
#include <assert.h>
#include "idl_conf_if.h"
#include "idl.h"
#include "idl_lex.h"
/* the already known keywords */
const int ECHO_BUFF_SIZE = 2048;
static const char *type_keywords[]={
  "raw",
  "char",
  "uchar",
  "int8_t",
  "uint8_t",
  "int16_t",
  "uint16_t",
  "int32_t",
  "uint32_t",
  "int64_t",
  "uint64_t",
  "float",
  "double",
  "string",
};
int is_keyword_type(meta_t &type)
{
  int size=sizeof(type_keywords)/sizeof(const char*);
  int i;
  for(i=0;i<size;i++){
    if(type.data==type_keywords[i])return 1;
  }
  return 0;
}

meta_t *alloc_meta(idl_t *idl)
{
  assert(idl!=NULL);
  meta_t *meta=new meta_t;
  idl->rc_meta_list.push_back(meta);
  return meta;
}
cf_t *alloc_cf(idl_t *idl)
{
  assert(idl!=NULL);
  cf_t *cf=new cf_t;
  idl->rc_cf_list.push_back(cf);
  return cf;
}
var_t *alloc_var(idl_t *idl)
{
  assert(idl!=NULL);
  var_t *var=new var_t;
  idl->rc_var_list.push_back(var);
  return var;
}
group_t *alloc_group(idl_t *idl)
{
  assert(idl!=NULL);
  group_t *group=new group_t;
  idl->rc_group_list.push_back(group);
  return group;
}

void add_idl_error(idl_t *idl,meta_t &errinfo)
{
  assert(idl!=NULL);
  idl->err_list.push_back(errinfo);
  return ;
}

void add_idl_var(idl_t *idl,var_t *var)
{
  assert(idl!=NULL);
  if(var==NULL)return ;

  meta_t &name=var->name;
  meta_t &type=var->type;
  var_map_t &var_map=idl->var_map;
  var_list_t &var_list=idl->var_list;
  group_map_t &group_map=idl->group_map;

  /* 1.we have to assure the var doesn't exist in 
     the config right now */
  if(var_map.find(name.data)!=var_map.end()){/* exists */
    meta_t errinfo;
    char tmp[ECHO_BUFF_SIZE];
    errinfo.file=name.file;
    errinfo.lineno=name.lineno;

    var_t *before=var_map.find(name.data)->second;
    snprintf(tmp,sizeof(tmp),"var '%s' already exists at %s:%d",
	    name.data.c_str(),
	    before->name.file.c_str(),
	    before->name.lineno);
    errinfo.data=tmp;
    add_idl_error(idl,errinfo);
    return ;
  }

  /* 2.we have to assure the if the typename is not 
     keyword type,it should exist in the idl */  
  if(!is_keyword_type(type) && 
     group_map.find(type.data)==group_map.end()){
      meta_t errinfo;
      char tmp[ECHO_BUFF_SIZE];
      errinfo.file=type.file;
      errinfo.lineno=type.lineno;
      snprintf(tmp,sizeof(tmp),"type '%s' doesn't exist",
	      type.data.c_str());
      errinfo.data=tmp;
      add_idl_error(idl,errinfo);
  }
  var_map[name.data]=var;
  var_list.push_back(var);
  return ;
}

void add_idl_group(idl_t *idl,group_t *grp)
{
  assert(idl!=NULL);
  if(grp==NULL)return ;

  meta_t &name=grp->name;
  group_map_t &group_map=idl->group_map;
  group_list_t &group_list=idl->group_list;
  /* 1.the group has already exists in the idl */
  if(group_map.find(name.data)!=group_map.end()){
    group_t *group=group_map.find(name.data)->second;
    meta_t &before=group->name;

    meta_t errinfo;
    char tmp[ECHO_BUFF_SIZE];
    errinfo.file=name.file;
    errinfo.lineno=name.lineno;
    snprintf(tmp,sizeof(tmp),"groups '%s' already exists at %s:%d",
	    name.data.c_str(),
	    before.file.c_str(),
	    before.lineno);
    errinfo.data=tmp;
    add_idl_error(idl,errinfo);
    return ;
  } 
  group_map[(grp->name).data]=grp;
  group_list.push_back(grp);
  return ;
}


void assemble_idl_cf(cf_t *cf,meta_t *fn,meta_list_t *arg_list)
{
  assert(cf!=NULL);
  meta_vec_t arg_vec;
  cf->func=(*fn);
  cf->arg_list=(*arg_list);
  meta_list_t::iterator arg_it;
  for(arg_it=arg_list->begin();arg_it!=arg_list->end();++arg_it){
    arg_vec.push_back(*arg_it);
  }
  cf->arg_vec=arg_vec;
  return ;
}

void assemble_idl_var(idl_t *idl,var_t *var,meta_t *id,meta_t *type,cf_list_t *cf_list)
{
  assert(idl!=NULL);
  assert(var!=NULL);
  assert(id!=NULL);
  assert(type!=NULL);
  assert(cf_list!=NULL);

  cf_map_t cf_map;
  var->name=(*id);
  var->type=(*type);
  var->cf_list=(*cf_list);
  cf_list_t::iterator cf_it;
  for(cf_it=cf_list->begin();cf_it!=cf_list->end();++cf_it){
    cf_t *cf=*cf_it;
    if(cf_map.find(cf->func.data)!=cf_map.end()){
      char tmp[ECHO_BUFF_SIZE];
      meta_t errinfo;
      cf_t *before_cf=cf_map[cf->func.data];
      errinfo.file=cf->func.file;
      errinfo.lineno=cf->func.lineno;
      snprintf(tmp,sizeof(tmp),"constraint function '%s' has declared before at %s:%d",
	      before_cf->func.data.c_str(),
	      before_cf->func.file.c_str(),
	      before_cf->func.lineno);
      errinfo.data=tmp;
      add_idl_error(idl,errinfo);
    }else{
      cf_map[cf->func.data]=cf;
    }
  }
  var->cf_map=cf_map;
  return ;
}

void assemble_idl_group(idl_t *idl,group_t *group,meta_t *id,var_list_t *var_list)
{
  assert(idl!=NULL);
  assert(group!=NULL);
  assert(id!=NULL);
  assert(var_list!=NULL);

  group_map_t &group_map=idl->group_map;
  group->name=(*id);
  group->parent_name=(*id);
  group->var_list=(*var_list);

  var_map_t &var_map=group->var_map;
  var_list_t::iterator it;
  for(it=var_list->begin();it!=var_list->end();++it){
    var_t *each_var=*it;
    /* 1. if the filed in the group 
       is duplicate */
    if(var_map.find((each_var->name).data)!=var_map.end()){
      var_t *before=var_map.find((each_var->name).data)->second;
      meta_t errinfo;
      char tmp[ECHO_BUFF_SIZE];
      errinfo.file=each_var->name.file;
      errinfo.lineno=each_var->name.lineno;
      snprintf(tmp,sizeof(tmp),"field '%s' already exists group '%s' at %s:%d",
	      (each_var->name).data.c_str(),
	      (id->data).c_str(),
	      before->name.file.c_str(),
	      before->name.lineno);
      errinfo.data=tmp;
      add_idl_error(idl,errinfo);
      continue;
    }

    meta_t type=each_var->type;
    /* 2.we have to assure the if the typename is not 
       keyword type,it should exist in the idl */  
    if(!is_keyword_type(type) && 
       group_map.find(type.data)==group_map.end()){
      meta_t errinfo;
      char tmp[ECHO_BUFF_SIZE];
      errinfo.file=type.file;
      errinfo.lineno=type.lineno;
      snprintf(tmp,sizeof(tmp),"type '%s' doesn't exist",
	      type.data.c_str());
      errinfo.data=tmp;
      add_idl_error(idl,errinfo);
      continue;
    }
    var_map[(each_var->name).data]=each_var;
  }
  return ;
}

void overwrite_idl_group(idl_t *idl,group_t *group,meta_t *new_id,meta_t *old_id,var_list_t *var_list)
{
  assert(idl!=NULL);
  assert(group!=NULL);
  assert(new_id!=NULL);
  assert(old_id!=NULL);
  assert(var_list!=NULL);

  group_map_t &group_map=idl->group_map;
  
  /* 1. if the old id doesn't exist in group,
     we make a group by calling 'assmeble_idl_group' */
  if(group_map.find(old_id->data)==group_map.end()){
    meta_t errinfo;
    char tmp[ECHO_BUFF_SIZE];
    errinfo.file=old_id->file;
    errinfo.lineno=old_id->lineno;
    snprintf(tmp,sizeof(tmp),"group '%s' to be overwrited doesn't exist",
	    old_id->data.c_str());
    errinfo.data=tmp;
    add_idl_error(idl,errinfo);
    return assemble_idl_group(idl,group,new_id,var_list);
  }else{
    /* copy the old conf cons */
    (*group)=(*group_map[old_id->data]);
    group->name=(*new_id);

    /* find the topest parent */
    meta_t *tmp_id=old_id;
    while(!((tmp_id->data)==(group_map[tmp_id->data]->name.data))){
      tmp_id=&(group_map[tmp_id->data]->name);
    } 
    group->parent_name=(*tmp_id);

    /* set the map */
    var_map_t &var_map=group->var_map;
    var_list_t::iterator it;
    for(it=var_list->begin();it!=var_list->end();++it){
      var_t *each_var=*it;
      if(var_map.find((each_var->name).data)==var_map.end()){
	/* 2. should overwrite the old_id group fileds */
	meta_t errinfo;
	char tmp[ECHO_BUFF_SIZE];
	errinfo.file=each_var->name.file;
	errinfo.lineno=each_var->name.lineno;
	snprintf(tmp,sizeof(tmp),"field '%s' doesn't exist in group '%s'",
		each_var->name.data.c_str(),
		old_id->data.c_str());
	errinfo.data=tmp;
	add_idl_error(idl,errinfo);
	continue;
      }


      meta_t type=each_var->type;
      /* 2.we have to assure the if the typename is not 
	 keyword type,it should exist in the idl */  
      if(!is_keyword_type(type) && 
	 group_map.find(type.data)==group_map.end()){
	meta_t errinfo;
	char tmp[ECHO_BUFF_SIZE];
	errinfo.file=type.file;
	errinfo.lineno=type.lineno;
	snprintf(tmp,sizeof(tmp),"type '%s' doesn't exist",
		type.data.c_str());
	errinfo.data=tmp;
	add_idl_error(idl,errinfo);
	continue;
      }

      var_t *new_var=alloc_var(idl);
      (*new_var)=(*each_var);
      var_map[(each_var->name).data]=new_var;
    }

    /* reorganize the var list */
    var_list_t old_var_list=group->var_list;
    var_list_t new_var_list;
    for(it=old_var_list.begin();it!=old_var_list.end();++it){
      var_t *each_var=*it;
      if(var_map.find((each_var->name).data)!=var_map.end()){	
	var_t *new_var=var_map[(each_var->name).data];
	new_var_list.push_back(new_var);
      }
    }
    group->var_list=new_var_list;	  
    return ;
  }
}

extern int cfgidlparse(yyscan_t scanner,meta_t *loc,idl_t *idl);
namespace confIDL{
/* the inteface provied to the user */
void print_idl(idl_t *idl)
{
  assert(idl!=NULL);
  group_list_t &group_list=idl->group_list;
  group_list_t::iterator group_it;

  printf("idl groups:\n");
  for(group_it=group_list.begin();group_it!=group_list.end();++group_it){
    group_t &group=**group_it;
    printf("\tgroup:%s,overwrited from %s\n",\
	   group.name.data.c_str(),\
	   group.parent_name.data.c_str());

    var_list_t &var_list=group.var_list;
    var_list_t::iterator var_it;
    for(var_it=var_list.begin();var_it!=var_list.end();++var_it){
      var_t &var=**var_it;
      printf("\t\tvar:%s,type:%s\n",var.name.data.c_str(),\
	     var.type.data.c_str());
      
      cf_list_t &cf_list=var.cf_list;
      cf_list_t::iterator cf_it;
      for(cf_it=cf_list.begin();cf_it!=cf_list.end();++cf_it){
	cf_t &cf=**cf_it;
	printf("\t\t\tfunc:%s\n",cf.func.data.c_str());	
	
	meta_list_t &arg_list=cf.arg_list;
	meta_list_t::iterator arg_it;
	for(arg_it=arg_list.begin();arg_it!=arg_list.end();++arg_it){
	  meta_t &arg=**arg_it;
	  printf("\t\t\t\targ:%s\n",arg.data.c_str());
	}
      }
    }
  }
  
  printf("idl vars:\n");
  var_list_t &var_list=idl->var_list;
  var_list_t::iterator var_it;
  for(var_it=var_list.begin();var_it!=var_list.end();++var_it){  
    var_t &var=**var_it;
    printf("\tvar:%s,type:%s\n",var.name.data.c_str(),\
	   var.type.data.c_str());
    
    cf_list_t &cf_list=var.cf_list;
    cf_list_t::iterator cf_it;
    for(cf_it=cf_list.begin();cf_it!=cf_list.end();++cf_it){
      cf_t &cf=**cf_it;
      printf("\t\tfunc:%s\n",cf.func.data.c_str());	
      
      meta_list_t &arg_list=cf.arg_list;
      meta_list_t::iterator arg_it;
      for(arg_it=arg_list.begin();arg_it!=arg_list.end();++arg_it){
	meta_t &arg=**arg_it;
	printf("\t\t\targ:%s\n",arg.data.c_str());
      }
    }
  }
  
  printf("idl error:\n");
  err_list_t &err_list=idl->err_list;
  err_list_t::iterator err_it;
  for(err_it=err_list.begin();err_it!=err_list.end();++err_it){
    meta_t &err=*err_it;
    printf("\t%s:%d:%s\n",
	   err.file.c_str(),
	   err.lineno,
	   err.data.c_str());
  }
  return ;
}

idl_t *alloc_idl()
{
  idl_t *idl =  new idl_t;
  idl->cmdreserve = 0;
  return idl;
}
void free_idl(idl_t *idl)
{
  if(idl==NULL)return ;
  meta_list_t &rc_meta_list=idl->rc_meta_list;
  meta_list_t::iterator meta_it;
  for(meta_it=rc_meta_list.begin();meta_it!=rc_meta_list.end();++meta_it)
    delete *meta_it;

  cf_list_t &rc_cf_list=idl->rc_cf_list;
  cf_list_t::iterator cf_it;
  for(cf_it=rc_cf_list.begin();cf_it!=rc_cf_list.end();++cf_it)
    delete *cf_it;

  var_list_t &rc_var_list=idl->rc_var_list;
  var_list_t::iterator var_it;
  for(var_it=rc_var_list.begin();var_it!=rc_var_list.end();++var_it)
    delete *var_it;

  group_list_t &rc_grp_list=idl->rc_group_list;
  group_list_t::iterator grp_it;
  for(grp_it=rc_grp_list.begin();grp_it!=rc_grp_list.end();++grp_it)
    delete *grp_it;

  cf_list_p &cflp=idl->parse_list;
  cf_list_p::iterator cflp_it;
  for(cflp_it=cflp.begin();cflp_it!=cflp.end();++cflp_it)
    delete *cflp_it;

  var_list_p &varlp=idl->parse_var_list;
  var_list_p::iterator varlp_it;
  for(varlp_it=varlp.begin();varlp_it!=varlp.end();++varlp_it)
    delete *varlp_it;

  meta_list_p &mtlp=idl->parse_meta_list;
  meta_list_p::iterator mtlp_it;
  for(mtlp_it=mtlp.begin();mtlp_it!=mtlp.end();++mtlp_it)
    delete *mtlp_it;

  delete idl;
  return ;
}

void load_idl(const char *file,
	      idl_t *idl)
{
  meta_t loc;
  FILE *fin;
  yyscan_t scanner;

  /* haven't allocate the space */
  if(idl==NULL)
    return ;
  /* init the lexical process */
  loc.file=file;
  loc.lineno=1;

  fin=fopen(file,"r");
  if(fin==NULL){
    /* can't open the file */
    meta_t errinfo;
    char tmp[ECHO_BUFF_SIZE];
    snprintf(tmp,sizeof(tmp),"can't open file '%s'",file);
    errinfo.file=file;
    errinfo.lineno=0;
    errinfo.data=tmp;
    add_idl_error(idl,errinfo);
    idl->retval=1;
    return ;
  }
  
  cfgidllex_init(&scanner);
  cfgidlset_in(fin,scanner);
  cfgidlparse(scanner,&loc,idl);
  cfgidllex_destroy(scanner);
  fclose(fin);

  /* set the return value */
  idl->retval=((idl->err_list).empty())?0:1;
  return ;
}
};
