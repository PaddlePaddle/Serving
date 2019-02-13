/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ConfigUnit.h,v 1.19 2010/04/13 09:59:41 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file ConfigUnit.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/11/26 20:53:09
 * @version $Revision: 1.19 $ 
 * @brief 
 *  
 **/

 
#ifndef  __CONFIGUNIT_H_
#define  __CONFIGUNIT_H_

#include "utils/cc_utils.h"
#include "bsl/var/IVar.h"
#include "bsl/var/Ref.h"
#include "bsl/ResourcePool.h"
#include "bsl/containers/hash/bsl_hashmap.h"
#include <vector>

namespace comcfg{
	enum{
		CONFIG_UNIT_TYPE = 0,
		CONFIG_GROUP_TYPE,
		CONFIG_ARRAY_TYPE,
		CONFIG_ERROR_TYPE
	};
	class Reader;
	class ConfigGroup;
	class ConfigUnit{
		public:
			/**
			 * @brief 方括号重载（三个函数），支持以方括号获取相应字段
			 * 			支持字符串、bsl::string和int下标。int下标Array专用。
			 * 			如果出错，返回一个ConfigError类型的常量
			 * 			可以用 conf[""].selfType() != CONFIG_ERROR_TYPE来判断
			 *
			 * @return  const ConfigUnit& [] 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:18:13
			**/
			virtual const ConfigUnit & operator[] (const char *) const;
			virtual const ConfigUnit & operator[] (const str_t &) const;
			virtual const ConfigUnit & operator[] (int) const;

			virtual  ConfigUnit & operator[] (const char *) ;
			virtual  ConfigUnit & operator[] (const str_t &) ;
			virtual  ConfigUnit & operator[] (int) ;
			virtual  ConfigUnit & operator= (ConfigUnit & unit) ;
			
			/**
			 * @brief 将一个ErrCode转化为一个对应的字符串信息
			 * 			ErrCode内部就是一个int, 0为OK，非0为出错信息
			 *
			 * @return  const char* 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:21:07
			**/
			virtual const char * seeError(const ErrCode &) const;
			virtual ~ConfigUnit();

			/**
			 * @brief 从一个字段中获取特定类型的数据
			 * 			XXX to_XXX() 表示以XXX类型获取该配置项
			 *
			 * @param [out] errCode   : ErrCode* 出错信息
			 * 				errCode不为NULL时，错误号将会写入*errCode
			 * 				当此处errCode为默认值NULL时，出错将会以异常的形式抛出
			 * 				抛出的异常为：ConfigException或其子类
			 * @return  char 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:22:07
			**/
			virtual char to_char(ErrCode * errCode = NULL)const;
			virtual unsigned char to_uchar(ErrCode * errCode = NULL)const;
			virtual int16_t to_int16(ErrCode * errCode = NULL)const;
			virtual u_int16_t to_uint16(ErrCode * errCode = NULL)const;
			virtual int to_int32(ErrCode * errCode = NULL)const;
			virtual u_int32_t to_uint32(ErrCode * errCode = NULL)const;
			virtual long long to_int64(ErrCode * errCode = NULL)const;
			virtual unsigned long long to_uint64(ErrCode * errCode = NULL)const;
			virtual float to_float(ErrCode * errCode = NULL)const;
			virtual double to_double(ErrCode * errCode = NULL)const;
			virtual bsl_string to_bsl_string(ErrCode * errCode = NULL)const;
			//to_std_string : 返回值为std::string，如果是双引号开头的串，这里的字符串是经过转义的，\'\x41\' -> 'A'
			//virtual std_string to_std_string(ErrCode * errCode = NULL)const;
			//to_raw_string : 未经过转义处理的字符串
			virtual str_t to_raw_string(ErrCode * errCode = NULL)const;
			//获取C风格的字符串（常量）
			virtual const char * to_cstr(ErrCode * errCode = NULL)const;

			/**
			 * @brief 带有默认值的to_XXX系列函数
			 *        返回值与to_xxx系列完全相同。
			 *        唯一的不同是：当errCode不为0（即发生错误）, 返回def默认值
			 *
			 * @param [out] errCode   : ErrCode* 出错信息
			 * @param [in] def   : const &  默认值
			 * @return  
			 * @author zhang_rui 
			 * @date 2009/03/11 22:37:46
			**/
			virtual char to_char(ErrCode * errCode, const char & def)const;
			virtual unsigned char to_uchar(ErrCode * errCode, const unsigned char & def)const;
			virtual int16_t to_int16(ErrCode * errCode, const int16_t & def)const;
			virtual u_int16_t to_uint16(ErrCode * errCode, const u_int16_t & def)const;
			virtual int to_int32(ErrCode * errCode, const int & def)const;
			virtual u_int32_t to_uint32(ErrCode * errCode, const u_int32_t & def)const;
			virtual long long to_int64(ErrCode * errCode, const long long & def)const;
			virtual unsigned long long to_uint64(ErrCode * errCode, const unsigned long long & def)const;
			virtual float to_float(ErrCode * errCode, const float & def)const;
			virtual double to_double(ErrCode * errCode, const double & def)const;
			virtual bsl_string to_bsl_string(ErrCode * errCode, const bsl_string & def)const;
			virtual str_t to_raw_string(ErrCode * errCode, const str_t & def)const;
			//获取C风格的字符串（常量）
			virtual const char * to_cstr(ErrCode * errCode, const char * def)const;



			/**
			 * @brief 与to_XXX功能相同的函数。不过get_XXX系列为C风格
			 * 			传入为用于放值的buffer，返回值是错误号
			 *
			 * @param [out] valueBuf   : char*
			 * @return  ErrCode 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:25:52
			**/
			virtual ErrCode get_char(char * valueBuf)const;
			virtual ErrCode get_uchar(unsigned char * valueBuf)const;
			virtual ErrCode get_int16(int16_t * valueBuf)const;
			virtual ErrCode get_uint16(u_int16_t * valueBuf)const;
			virtual ErrCode get_int32(int * valueBuf)const;
			virtual ErrCode get_uint32(u_int32_t * valueBuf)const;
			virtual ErrCode get_int64(long long * valueBuf)const;
			virtual ErrCode get_uint64(unsigned long long * valueBuf)const;
			virtual ErrCode get_float(float * valueBuf)const;
			virtual ErrCode get_double(double * valueBuf)const;
			virtual ErrCode get_bsl_string(bsl_string * valueBuf)const;
			//virtual ErrCode get_std_string(std_string * valueBuf) const;
			virtual ErrCode get_raw_string(str_t * valueBuf) const;
			//获取C风格的字符串，将字符串填入valueBuf，len是valueBuf长度，自动补0，最多填充len-1个字符
			virtual ErrCode get_cstr(char * valueBuf, size_t len) const;


			/**
			 * @brief 带有默认值的ge_XXX系列函数
			 *        返回值与get_xxx系列完全相同。
			 *        唯一的不同是：当返回值不为0（即发生错误）valueBuf中使用def默认值
			 *
			 * @param [out] valueBuf   : char*  传出的值
			 * @param [in] def   : const char&  默认值
			 * @return  ErrCode 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/11 22:37:46
			**/
			virtual ErrCode get_char(char * valueBuf, const char & def)const;
			virtual ErrCode get_uchar(unsigned char * valueBuf, const unsigned char & def)const;
			virtual ErrCode get_int16(int16_t * valueBuf, const int16_t & def)const;
			virtual ErrCode get_uint16(u_int16_t * valueBuf, const u_int16_t & def)const;
			virtual ErrCode get_int32(int * valueBuf, const int & def)const;
			virtual ErrCode get_uint32(u_int32_t * valueBuf, const u_int32_t & def)const;
			virtual ErrCode get_int64(long long * valueBuf, const long long & def)const;
			virtual ErrCode get_uint64(unsigned long long * valueBuf, const unsigned long long & def)const;
			virtual ErrCode get_float(float * valueBuf, const float & def)const;
			virtual ErrCode get_double(double * valueBuf, const double & def)const;
			virtual ErrCode get_bsl_string(bsl_string * valueBuf, const bsl_string & def)const;
			virtual ErrCode get_raw_string(str_t * valueBuf, const str_t & def)const;
			virtual ErrCode get_cstr(char * valueBuf, size_t len, const char * def)const;

			/**
			 * @brief 将自身转化为一个IVar
			 *
			 * 		  会抛异常
			 *
			 * @param [in/out] vpool   : VarPool* IVar句柄表，所以新分配的IVar都注册到VarPool
			 *                                    当需要销毁我创建的IVar时，只需要vpool->destroy();
			 * @param [in] errCode   : ErrCode* 错误号，0为正确，其余表示出错
			 * @return  IVar& 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/11 22:00:48
			**/
			virtual bsl::var::IVar& to_IVar(bsl::ResourcePool* vpool, ErrCode* errCode = NULL)const;
			/**
			 * @brief 获取自己的类型
			 *
			 * @return  int 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:35:27
			**/
			virtual int selfType()const{
				return CONFIG_UNIT_TYPE;
			}
			
			/**
			 * @brief 获取元素个数
			 *
			 * @return  size_t 个数
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 02:00:28
			**/
			virtual size_t size()const{
				return 1;
			}

			/**
			 * @brief 长路径Unit的直接获取
			 *
			 * @param [in] path   : const str_t 长路径，例如 group.sub.key
			 * @return  const ConfigUnit& 长路径获取的ConfigUnit
			 * @date 2009/07/16 15:50:47
			**/
			virtual const ConfigUnit & deepGet(const str_t path) const;

			/**
			 * @brief 访问不存在的key后，给出不存在的key的具体路径。比如 group.sub.nokey
			 *
			 * @return  const char*  不存在的key的路径
			 * @author zhang_rui
			 * @date 2009/07/17 10:20:09
			**/
			const char * getErrKeyPath() const;

			/**
			 * @brief 获取子unit。ConfigUnit直接返回error unit
			 *
			 * @return  ConfigUnit &
			 * @author zhang_rui
			**/
			virtual const ConfigUnit & get_sub_unit(int index) const;
			/**
			 * @brief 比较是否相等
			 *
			 * @return  int 0表示相等 -1不相等
			 * @author zhang_rui
			**/
			virtual int equals(const ConfigUnit & conf) const;

			//以下函数的参数中，except为0则用错误号表示错误，
			//except为1则用异常表示错误
			/**
			 * @brief 增加一个unit, 连接到子unit，不做深拷贝
			 *
			 * @param [in] key   : 新增unit的key
			 * @param [in] value : 新增unit的value
			 * @param [in] objtype : unit类型。
			 *    CONFIG_UNIT_TYPE表示unit;
			 *    CONFIG_GROUP_TYPE表示group;
			 *    CONFIG_ARRAY_TYPE表示array;
			 * @return  int 0 成功，其他是错误号
			 * @author zhang_rui
			**/
			virtual int add_unit(const bsl_string & key, const bsl_string& value, 
				const int objtype=CONFIG_UNIT_TYPE, int except=0, ConfigUnit ** ref=NULL);

			/**
			 * @brief 复制一个unit, 深拷贝
			 *
			 *  将unit的内容追加到本对象下面。
			 * 类型需要匹配，只能从group复制到group，array复制到array
			 * group要检查key是否重复，不重复则追加
			 * array直接追加
			 * @return  int 0 成功，其他是错误号
			 * @author zhang_rui
			**/
			virtual int append_unit(const ConfigUnit & unit, int except=0);

			/**
			 * @brief 复制一个unit, 深拷贝
			 *
			 *  将unit的内容复制到本对象。
			 * 类型需要匹配，只能从group复制到group，array复制到array
			 * @return  int 0 成功，其他是错误号
			 * @author zhang_rui
			**/
			virtual int copy_unit(const ConfigUnit & unit, int except=0);
			
			/**
			 * @brief 删除一个unit
			 *
			 * key : 要删除的unit的key
			 * @return  int 0 成功，其他是错误号
			 * @author zhang_rui
			**/
			virtual int del_unit(const bsl_string & key, int except=0);
			/**
			 * @brief 修改unit中value的值
			 *
			 * @return  int 0 成功，其他是错误号
			 * @author zhang_rui
			**/
			virtual int set_value(const bsl_string & value, int except=0);
			/**
			 * @brief 遍历load()完成后的comcfg::Configure结构，使用用户提供的callback函数对每个unit进行操作
			 * @note 深度优先搜索，子节点的访问无先后顺序
			 * @param [in] cb_fn: 用户提供的callback函数
			 * @param [in] cb_arg: 用户callback函数所需要的参数
			 * 
			 * @return  int 0 成功，其他是错误号
			 * @author linjieqiong
			**/
			typedef int (* cb_tr)(ConfigUnit *, void *);
			virtual int traverse_unit(cb_tr cb_fn, void *cb_arg);
			/**
			 * @brief 类似于traverse_unit()，区别是遍历一个group时按照CONFIG_ERROR_TYPE, CONFIG_UNIT_TYPE, CONFIG_ARRAY_TYPE, CONFIG_GROUP_TYPE的顺序访问子节点
			 *
			 * @param [in] cb_fn: 用户提供的callback函数
			 * @param [in] cb_arg: 用户callback函数所需要的参数
			 *
			 * @return  int 0 成功，其他是错误号
			 *
			 * @version 1.2.9
			 * @author linjieqiong
			 * @date 2013/04/28 16:25:47
			 */
			virtual int traverse_unit_order(cb_tr cb_fn, void *cb_arg);

			//-----------------从这里往下的东西用户不需要关心-----------------

			//构造函数，
			ConfigUnit();
			ConfigUnit(const str_t& __key, const str_t& __value, const Reader * __cur_reader = NULL, ConfigGroup * father = NULL);

			//调试打印时用，缩进
			void pindent(int ind)const{
				while(ind--){
					printf("    ");
				}
			}
			//打印
			virtual void print(int indent = 0)const{
				pindent(indent);
				printf("=[%s], _value=[%s]\n", _key.c_str(), _value.c_str());
			}


			//for debug or writelog
			virtual str_t info() const{
				//const int INFO_BUF_SIZE = 256;
				//char buf[INFO_BUF_SIZE] = "";
				str_t buf;
				buf.appendf("[File:%s Line:%d Key:%s Value:%s]", 
						_at_file ? _at_file : "NULL", _at_line, _key.c_str(), _value.c_str());
				return buf;
			}
			//for child class
			virtual ConfigUnit * relativeSection(ConfigUnit *, str_t, int* ){
				return NULL;
			}
			inline const char * getFileName(){
				return _at_file;
			}
			inline int getFileLine(){
				return _at_line;
			}
			inline void setConstrainted(const bool bCons){
				_bConstrainted = bCons;
			}
			inline bool getConstrainted()const{
				return _bConstrainted;
			}
			static ConfigUnit* get_err_unit();
			virtual void get_full_path(bsl::string &path, const int get_global = 0) const;
			inline virtual const bsl::string & get_key_name() const{
				return _key;
			}
			inline ConfigGroup * getFather()const{
				return _father;
			}
		protected:
			inline void setFilePosition(const char *cur_file = "NULL", const int cur_line = -1){
				_at_file = cur_file;
				_at_line = cur_line;
			}
			inline void setFileLine(const int cur_line = -1){
				_at_line = cur_line;
			}
			virtual void popError(ErrCode *, ErrCode) const;
			void setErrorKeyPath(str_t str)const;
            #if __GNUC__ == 4 && __GNUC_MINOR__ >= 4 
            const ConfigUnit & deepGetSegment(const str_t segment) const; 
            #else
            const ConfigUnit & ConfigUnit :: deepGetSegment(const str_t segment) const; 
            #endif
			
			static const str_t g_unknown;
			str_t _key;
			str_t _value;
			str_t _cstr; //专门的一个被解析后的string，只有当value以"为开头时才起作用
			int _cstr_err;
			str_t _vstring;
			const char * _at_file;
			int _at_line;
			ConfigGroup * _father;
			bool _bConstrainted;		
			//mutable str_t _to_def_str;
			ConfigUnit * create_unit(const bsl_string & key, const bsl_string& value, 
				int objtype, ConfigGroup * father);

			/**
			 * @brief 清除内容
			 *
			 * @return  void
			 * @author zhang_rui
			**/
			virtual void clear();
			
		public:
			/**
			 * @brief 复制一个unit, 深拷贝
			 *
			 *  将unit的内容复制到本对象下面。
			 * 类型需要匹配，只能从group复制到group，array复制到array
			 * group要检查key是否重复，不重复则追加
			 * array直接追加
			 * @return  int 0 成功，其他是错误号
			 * @author zhang_rui
			**/
			virtual int _append_unit(const ConfigUnit & unit,int check=1, int except=0);
		private:
			int init_unit(const str_t& __key, const str_t& __value);
			//访问类型为type的子节点，如果子节点是group/array，递归访问
			int traverse_sub_type(cb_tr cb_fn, void *cv_arg, const int type);
	};

}









#endif  //__CONFIGUNIT_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
