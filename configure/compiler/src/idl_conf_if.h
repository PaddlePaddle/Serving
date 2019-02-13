/* -*- c++ -*- 
   copy[write] by dirlt(zhang_yan@baidu.com)
   date time:Wed Nov 19 23:22:32 CST 2008
   file name:idl_conf_if.h */

#ifndef _IDL_CONF_IF_H_
#define _IDL_CONF_IF_H_

/**
 * @file idl_conf_if.h
 * @author zhang_yan@baidu.com
 * @date Wed Nov 19 23:22:32 CST 2008
 * @version
 * @brief the IDL conf_if provided to the user
 */

#include <string>
#include <vector>
#include <list>
#include <map>
#include "bsl/containers/string/bsl_string.h"

namespace confIDL{
/**
* @brief the meta data structure
*/
  typedef bsl::string idl_str_t;
  struct meta_t
  {
    idl_str_t file;/**< the meta data in which file */
    int lineno;/**< the meta data in which line */
    idl_str_t data;/**< what is the data in it?? */
  };
  typedef std::list<meta_t*> meta_list_t;
  typedef std::vector<meta_t *>meta_vec_t;
  typedef std::list<meta_t> err_list_t;

/**
* @brief constraint function structure
* detail using it to describe the constraint function
*/
  struct cons_func_t{
    meta_t func;/**< constraint function name */
    meta_list_t arg_list;/**< constraint function arguments(in idl_str_t form) */
    meta_vec_t arg_vec;/**< constraint function arguments(in idl_str_t form) */
  };
  typedef cons_func_t cf_t;
  typedef std::list<cf_t*> cf_list_t;
  typedef std::map<idl_str_t,cf_t *>cf_map_t;

  typedef std::list<cf_list_t*> cf_list_p;
  typedef std::list<meta_list_t*> meta_list_p;

/**
* @brief variable structure
* detail using it to descibel the variable or the fields in the class
*/
  struct var_t{
    meta_t name;
    meta_t type;
    cf_list_t cf_list;
    cf_map_t cf_map;
  };
  typedef std::list<var_t*>var_list_t;
  typedef std::map<idl_str_t,var_t*>var_map_t;

  typedef std::list<var_list_t*> var_list_p;

/**
* @brief struct structure
* detail using it to describle the class or the strucut or some 
* complex object 
*/
  struct group_t{
    meta_t name;/**< the struct name */
    meta_t parent_name;/**< the struct which this struct is overwrited from */
    var_list_t var_list;
    var_map_t var_map;
  };
  typedef std::map<idl_str_t,group_t*>group_map_t;
  typedef std::list<group_t*>group_list_t;
/**
* @brief IDL struct
* detail contain all the information collected from the IDL file
* also including the resource pool 
*/
  struct idl_t
  {
    group_map_t group_map;/**< structs in the IDL */
    var_map_t var_map;/**< the variables in the IDL */
    group_list_t group_list; 
    var_list_t var_list;
    
    /* resource pool */
    meta_list_t rc_meta_list;/**< the allocated meta data  */
    cf_list_t rc_cf_list;/**< the allocated constraint functions  */
    var_list_t rc_var_list;/**< the allocated variables */
    group_list_t rc_group_list;/**< the allocated groups*/
  
	int cmdreserve;
    int retval;/**< return value of parsing the IDL file */
    err_list_t err_list;/**< the generated detail error messages when we parse the IDL file */

	cf_list_p parse_list; /**< the allocated cflist in parsing*/
	meta_list_p parse_meta_list; /**< the allocated metalist in parsing*/
	var_list_p parse_var_list; /**< the allocated varlist in parsing*/
  };


  /**
   * @brief allocated a IDL struct
   * @return return the allocated  IDL  struct
   * @see idl_t
   * @author zhang_yan@baidu.com
   * @date Wed Nov 19 23:28:47 CST 2008
  */
  extern confIDL::idl_t *alloc_idl();

  /**
   * @brief deallocated the IDL struct
   * @param idl the IDl struct allocated by alloc_idl() this function
   * @see alloc_idl
   * @author zhang_yan@baidu.com
   * @date Wed Nov 19 23:29:36 CST 2008
  */
  extern void free_idl(confIDL::idl_t *idl);

  /**
   * @brief load the IDL structu from the file
   * @param file the IDL file
   * @param  idl the IDL strucut we want to load into
   * @see alloc_idl free_idl
   * @author zhang_yan@baidu.com
   * @date Wed Nov 19 23:30:52 CST 2008
  */
  extern void load_idl(const char *file,confIDL::idl_t *idl);

  /**
   * @brief print the IDL struct in the indented form
   * @param idl the IDL struct to be printed
   * @see load_idl
   * @author zhang_yan@baidu.com
   * @date Wed Nov 19 23:30:55 CST 2008
  */
  extern void print_idl(confIDL::idl_t *idl);

};

#endif

