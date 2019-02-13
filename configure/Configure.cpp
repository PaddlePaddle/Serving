/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
<<<<<<< Configure.cpp
 * $Id: Configure.cpp,v 1.21 2010/04/13 09:59:41 scmpf Exp $ 
=======
 * $Id: Configure.cpp,v 1.21 2010/04/13 09:59:41 scmpf Exp $ 
>>>>>>> 1.18
 * 
 **************************************************************************/



/**
 * @file Configure.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/21 18:09:35
<<<<<<< Configure.cpp
 * @version $Revision: 1.21 $ 
=======
 * @version $Revision: 1.21 $ 
>>>>>>> 1.18
 * @brief 
 *  
 **/

#include "Configure.h"
#include "reader/Reader.h"
#include "constraint/Constraint.h"
#include "utils/cc_utils.h"
#include "idl_conf_if.h"
#include "bsl/var/Dict.h"
#include "bsl/var/Array.h"

namespace comcfg{

	using bsl::var::IVar;

	//extern Reader * g_cur_reader;
	int Configure :: load(const char * path, const char * conf, const char * range, int /*version*/){
		return load_1_0(path, conf, range);
	}

	int Configure :: load_ex(const char * path, const char * conf, const char * range,
			int dupLevel, int /*version*/)
	{
		if(dupLevel < GROUP_DUP_LEVEL0 || dupLevel > GROUP_DUP_LEVEL3){
			LOG(WARNING) << "Configure.load_ex : parameter @dupLevel is wrong.";
			return ERROR;
		}
		_dupLevel = dupLevel;
		if(_dupLevel != GROUP_DUP_LEVEL0 ){
			_dupMap.create(256);
		}
		int ret = load_1_0(path, conf, range);
		return ret!=0 ? ret : printGroup();
	}

	int Configure :: load_ex2(const char * path, const char * conf, const char * range, int /*version*/)
	{
		int ret = load_1_0(path, conf, range);
		if(ret == 0 && range != NULL && *range != 0){
			return checkConstraint();
		}
		return ret;
	}

	int Configure :: load_1_0(const char * path, const char * conf, const char *range){
		int ret = 0;
		if(path == NULL || (conf == NULL && range == NULL)){
			LOG(WARNING) << "Configure.load : path or filename error...";
			return ERROR;
		}
		if(0 == strcmp(path, "")) {
			_path = ".";    //set as current dir
		}
		else {
			_path = path;
		}
		if(_readers.size() != 0){
			LOG(WARNING) << "Configure.load : Configure can't be reused, create a new object for load.";
			return ERROR;
		}
#if 0
		if(conf != NULL){
			Reader * reader = new Reader();
			_cur_reader = reader;
			_cur_level = 0;
			_readers.push_back(reader);
			ret = reader->read(conf, this);
			_cur_reader = NULL;
		}
#endif
		size_t idx = 0;
		pushSubReader(conf, 0);//config文件进入队列，因为可能有$include，按广度优先处理
		while(idx < _readers.size()){
			ReaderNode & r = (*_readers[idx]);
			_cur_reader = r.reader;
			_cur_level = r.level;
			_section = this;//切换到GLOBAL
			_depth = 0;
			_at_file = r.filename.c_str();
			ret = r.reader->read(_at_file, this);
			if(ret != 0){
				LOG(WARNING) << "Reader configure file [" << r.filename.c_str() << "] error. Stop.";
				return CONFIG_ERROR;
			}
			++idx;
		}

		if(range != NULL && 0 != *range){
			str_t range_file;
			range_file.append(_path).append("/").append(range);
			if(check_once(range_file.c_str()) != 0){
				return CONSTRAINT_ERROR;
			}
		}
		if(ConstraintLibrary :: getInstance() -> checkGlobalConstraint(this) != 0){
			return CONSTRAINT_ERROR;
		}
		return 0;
	}
	void Configure :: pushSubReader(const char * conf, int level){
		if(conf == NULL){
			return;
		}
		if(level < 0){
			level = _cur_level + 1;
		}
		LOG(INFO) << "Config $include : " << conf << ", MAX_DEPTH=" << MAX_INCLUDE_DEPTH << ", level=" << level;
		if(level > MAX_INCLUDE_DEPTH){
			LOG(WARNING) << "Config error: Max $include level is " << MAX_INCLUDE_DEPTH << " [" << _cur_reader->_cur_file.c_str() << ":" << _cur_reader->_cur_line << "]";
			return;
		}
		ReaderNode * newNode = new ReaderNode();
		_readers.push_back(newNode);
		ReaderNode & r = *(_readers[_readers.size() - 1]);
		r.reader = new Reader();
		r.filename.append(_path).append("/").append(conf);
		r.level = level;
		LOG(INFO) << "Config : $include : " << r.filename.c_str() << ", level=" <<  r.level;
	}

	char * Configure :: getRebuildBuffer(size_t __size){
		if(_readers.size() == 0){
			_path = ".";
			pushSubReader(REBUILD_CONF, 0);
		}
		else{
			LOG(WARNING) << "getRebuildBuffer : This Configure Object can't be re-used. Create a new one.";
			return NULL;
		}
		if(_readers.size() == 0){
			return NULL;
		}
		//return _readers[0].reader->getRebuildBuffer(__size);
		return _readers[0]->reader->getRebuildBuffer(__size);
	}

	int Configure :: rebuild(){
		if(_readers.size() == 0){
			return -1;
		}
		//_cur_reader = _readers[0].reader;
		_cur_reader = _readers[0]->reader;
		_cur_level = 0;
		int ret = _cur_reader->rebuild(this);
		_cur_reader = NULL;
		return ret;
	}

	char * Configure :: dump(size_t * __size){
		//多个子文件合并的时候，需要在子文件的头部添加 [GLOBAL]
		const char * global = "\n[" CONFIG_GLOBAL "]\n";
		size_t len = strlen(global);
		if(_dump_buffer != NULL){
			if(__size){
				*__size = _dump_size;
			}
			return _dump_buffer;
		}
		if(_readers.size() == 0){
			return NULL;
		}
		_dump_size = 0;
		std::vector <const char *> bufs;
		std::vector <size_t> rsize;
		for(size_t i = 0; i < _readers.size(); ++i){
			size_t tmpsize = 0;
			//char * p = _readers[i].reader->dump(&tmpsize);
			char * p = _readers[i]->reader->dump(&tmpsize);
			rsize.push_back(tmpsize);
			bufs.push_back(p);
			//LOG(INFO) << "dump file : %s, size=%zu", _readers[i].filename.c_str(), tmpsize);
			LOG(INFO) << "dump file : " << _readers[i]->filename.c_str() << ", size=" << tmpsize;
			if(p == NULL){
				return NULL;
			}
			_dump_size += tmpsize;
			if(i){
				_dump_size += len;
			}
		}

		size_t bufsize = ((_dump_size >> 12) + 1) << 12;
		_dump_buffer = (char *)malloc(bufsize);
		if(_dump_buffer == NULL){
			LOG(WARNING) << "Can't build dump buffer";
			return NULL;
		}

		size_t pos = 0;
		for(size_t i = 0; i < _readers.size(); ++i){
			if(i){
				memcpy(_dump_buffer + pos, global, len);
				pos += len;
			}
			memcpy(_dump_buffer + pos, bufs[i], rsize[i]);
			pos += rsize[i];
		}
		if(_dump_size != pos){
			LOG(WARNING) << "What's wrong??? _dump_size(" << _dump_size << ") != real size("<< pos << ")"; 
			return NULL;
		}
		_dump_buffer[_dump_size] = '\0';
		if(__size){
			*__size = _dump_size;
		}
		return _dump_buffer;
	}

	struct dump_conf{
		enum{
			RC_COMMENT = 0,
			RC_NO_COMMENT,
			RC_POSITION,
		};
		int rc_type;
		std::vector<Configure :: ReaderNode *> *files;
		void *father;
		str_t dumpedstr;
	};

	int cb_dumpConf(ConfigUnit *unit, void * dumpconf){
		int unit_type = unit->selfType();
		//以下两类节点是在内存中新建的，在配置文件中无对应节点
		if(unit_type == CONFIG_ARRAY_TYPE || unit_type == CONFIG_ERROR_TYPE){
			return 0;
		}
		struct dump_conf * conf = static_cast<struct dump_conf *>(dumpconf);
		str_t group_path;

		//父节点发生变化，重新获取其路径
		if(unit_type == CONFIG_UNIT_TYPE && conf->father != unit->getFather()){
			conf->father = unit->getFather();
			unit->getFather()->get_full_path(group_path);
			conf->dumpedstr.appendf("[%s]\n", group_path.c_str());
		}
		//从当前配置行往上获取该配置的注释信息
		if(conf->rc_type != dump_conf :: RC_NO_COMMENT){
			str_t comment;
			std::vector<Configure :: ReaderNode *> & ReaderNodes = *conf->files;
			for(size_t i=0; i<ReaderNodes.size(); ++i){
				if(strcmp(unit->getFileName(), ReaderNodes[i]->filename.c_str())){
					continue;
				}
				Reader * & rd = ReaderNodes[i]->reader;
				//该配置行的注释在原配置文件中的起始行号
				int line = unit->getFileLine() - 2;
				rd->getCommentUpwards(comment, line);
			}
			conf->dumpedstr.append(comment);
		}
		//dump该配置的在原配置文件中的位置
		if(conf->rc_type == dump_conf :: RC_POSITION){
			conf->dumpedstr.appendf("#[%s:%d]\n", unit->getFileName(), unit->getFileLine());
		}
		//dump该配置项
		if(unit_type == CONFIG_UNIT_TYPE){
			conf->dumpedstr.appendf("%s : %s\n", unit->get_key_name().c_str(), unit->to_cstr());
		}
		else{//CONFIG_GROUP_TYPE, traverse_unit()是深搜, 接下来会遍历子节点
			conf->father = unit;
			unit->get_full_path(group_path);
			conf->dumpedstr.appendf("[%s]\n", group_path.c_str());
		}
		return 0;
	}

	int Configure :: dump_ex(char *buf, const size_t bufsize,  const int restoreComment){
		if(buf == NULL || bufsize <= 0 || restoreComment < 0 || restoreComment > 2){
			LOG(WARNING) << "wrong parameter(s) of dump_ex().";
			return -1;
		}
		struct dump_conf conf = {restoreComment, &_readers, this, str_t(),};
		conf.dumpedstr.reserve(bufsize);
		
		traverse_unit_order(cb_dumpConf, &conf);
		
		size_t rsize = conf.dumpedstr.size() + 1;
		if(rsize > bufsize){
			LOG(WARNING) << "@bufsize is too small. at least " << rsize << " is required.";
			return 0 - rsize;
		}
		memcpy(buf, conf.dumpedstr.c_str(), rsize);
		return rsize;
	}
	
	void Configure :: changeSection(str_t str){
		LOG(INFO) << "Configure: This is [" << this->getName() <<  "], current section[" << _section->getName() << "], depth=" << _depth << ", father[" << (_section->getFather()?_section->getFather()->getName():"NULL") << "]"; 
		if(str[0] != '.'){
			_section = this;
			_depth = 0;
		}
		else{
			int i;
			for(i = 0; i < (int)str.size(); ++i){
				if(str[i] != '.'){
					break;
				}
			}

			if((int)i > _depth){
				LOG(WARNING) << "Configure : Section Error [" << str.c_str() << "] Ignored.";
				throw ConfigException();
			}
			str = str.substr(i);
			//--i;
			i = _depth - i;
			while(i > 0){
				_section = (ConfigGroup *)_section->getFather();
				_depth--;
				i--;
			}
		}

		ConfigGroup * _bak_section = _section;
		LOG(INFO) << "Config : set Section: New[" << str.c_str() << "] -> Father[" << _section->getName() << "]";
		_section = (ConfigGroup *)_section->relativeSection(this, str, &_depth);
		if(_section == NULL){
			_section = _bak_section;
		}
		if(_section && _cur_reader){
			_section->setFilePosition(_cur_reader->_cur_file.c_str(), _cur_reader->_cur_line);
		}
		LOG(INFO) << "Config : Now _section in : [" << (_section?_section->getName():"NULL") << "]";
	}

	void Configure :: pushPair(const str_t& key, const str_t& value){
		ConfigUnit * tmp = new ConfigUnit(key, value, _cur_reader, _section);
		if(_section && (_section->push(key, tmp) != 0)){
			delete tmp;
			free(tmp);
			throw ConfigException();
		}
	}

	time_t Configure :: lastConfigModify(){
		time_t t = time_t(0);
		struct stat st;
		for(int i = 0; i < (int)_readers.size(); ++i){
			//if(_readers[i].level <= MAX_INCLUDE_DEPTH){
			if(_readers[i]->level <= MAX_INCLUDE_DEPTH){
				//const char * f = _readers[i].filename.c_str();
				const char * f = _readers[i]->filename.c_str();
				if(stat(f, &st) == 0){
					if(t < st.st_mtime){
						t = st.st_mtime;
					}
				}
				else{
					LOG(WARNING) << "Check lastConfigModify : I can't stat file [" << f << "]";
					//有一个特例：
					//如果A文件include了B，运行中，我在A中去掉了$include : B，并且删除了B文件，导致我stat失败
					//此时，按理是合法的，配置文件可以被重新读取,lastConfigModify只要返回A的更新时间就可以了
					//不过建议还是不要删除B文件
					//我选择的是返回0，表示有一个文件无法被stat
					return time_t(0);
				}
			}
		}
		return t;
	}


	enum _VarType{
		VAR_ARRAY = 0,
		VAR_DICT,
		VAR_UNIT
	};

	int varType(const IVar& ivar){
		if(ivar.is_array()){
			return VAR_ARRAY;
		}
		else if(ivar.is_dict()){
			return VAR_DICT;
		}
		return VAR_UNIT;
	}

	//----------loadIVar-------------
	str_t seeVarUnit(const IVar & ivar, const char * key, bool isArray);
	str_t seeVarDict(const IVar & ivar, str_t * section);
	str_t seeVar(const IVar & ivar, str_t * section, const char * key = "");

	bool isVarUnit(const IVar & a, int dep = 1){
		if(dep < 0){
			throw ConfigException() << BSL_EARG << "Multi-dimension array is not supported.";
		}
		if(a.is_array()){
			return isVarUnit(a[0], dep-1);
		}
		if(a.is_dict()){
			return false;
		}
		return true;
	}

	str_t seeVarUnit(const IVar & ivar, const char * key, bool isArray){
		//把一个基本的IVar转化为config文本 Key : value
		str_t ret;
		str_t value = ivar.to_string();
		str_t tmp;
		if (ivar.is_int32() || ivar.is_int64() || ivar.is_double()) {
			tmp = value;
		} else {
			str_t strcontent="";
			int yinhao = 0;

			for(int i = 0; i < (int)value.size(); ++i){
				//特殊字符
				if(value[i] < 32 || value[i] >= 127 || value[i] == '\"' || value[i] == '\'' || value[i] == '\\'){
					strcontent.appendf("\\x%02x", value[i]);
					yinhao =1;
				}
				else{
					strcontent.appendf("%c", value[i]);
				}
			}
			if (yinhao) {
				tmp = "\"";
				tmp.append(strcontent);
				tmp.append("\"");
			} else {
				tmp = strcontent;
			}
		}
		ret.appendf("%s%s : %s\n", isArray?"@":"", key, tmp.c_str());
		return ret;
	}

	str_t nextSectionName(const str_t section, const char * key, bool isArray = false){
		str_t nsec;
		if(section == str_t(CONFIG_GLOBAL)){
			nsec = "";
			nsec.appendf("%s%s", isArray ? "@" : "", key);
		}
		else if(strstr(section.c_str(), "@") != NULL){
			nsec = ".";
			for(int i = 0; i < (int)section.size(); ++i){
				if(section[i] == '.'){
					nsec.appendf(".");
				}
			}
			nsec.appendf("%s%s", isArray ? "@" : "", key);
		}
		else{
			nsec = section;
			nsec.appendf("%s%s%s", nsec.size() == 0 ? "" : ".", isArray ? "@" : "", key);
		}
		return nsec;
	}


	str_t seeVarDict(const IVar & ivar, str_t * section){
		//把一个Dict转化为config文本
		LOG(INFO) << "Var " << ivar.to_string().c_str() << " is dict.";
		const IVar & dict = ivar; // no dynamic cast
		bsl::var::Dict::dict_const_iterator iter = dict.dict_begin();
		bsl::var::Dict::dict_const_iterator end  = dict.dict_end();
		str_t bak_section = *section;
		str_t ret;
		ret.appendf("[%s]\n", section->c_str());
		for(; iter != end; ++ iter ){
			if( isVarUnit(iter->value()) ){
				ret.append( seeVar(iter->value(), section, iter->key().c_str()) );
			}
		}
		for(iter = dict.dict_begin(); iter != end; ++ iter ){
			if(! isVarUnit(iter->value()) ){
				ret.append( seeVar(iter->value(), section, iter->key().c_str()) );
				if(*section != bak_section){
					*section = bak_section;
				}
			}
		}
		return ret;
	}


	str_t seeVar(const IVar & ivar, str_t * section, const char * key){
		//把一个IVar类型转为config文本
		str_t ret;
		//str_t bak_section = *section;
		if(varType(ivar) == VAR_ARRAY){
			// 先判断是否为数组
			LOG(INFO) << "Var " << ivar.to_string().c_str() << " is array.";
			if(varType(ivar[0]) == VAR_DICT ){
				*section = nextSectionName(*section, key, true);
			}
			for(int i = 0; i < (int)ivar.size(); ++i){
				if(varType(ivar[i]) == VAR_ARRAY){
					throw ConfigException() << BSL_EARG << "Multi-dimension array is not supported.";
				}
				if(varType(ivar[i]) != varType(ivar[0])){
					throw ConfigException() << BSL_EARG << "array elements should be the same type.";
				}
				if(varType(ivar[i]) == VAR_DICT){
					ret.append( seeVarDict(ivar[i], section) );
				}
				else{
					ret.append( seeVarUnit(ivar[i], key, true) );
				}
			}
		}
		else if(varType(ivar) == VAR_DICT){
			*section = nextSectionName(*section, key);
			ret.append( seeVarDict(ivar, section) );
		}
		else{
			ret.append( seeVarUnit(ivar, key, false) );
		}
		//*section = bak_section;
		return ret;
	}


	int Configure :: loadIVar(const IVar & ivar){
#if 0
		if(ivar == NULL){
			LOG(WARNING) << "loadIVar : ivar = NULL");
			return -1;
		}
#endif
		try{
			if( ! ivar.is_dict()){
				LOG(WARNING) << "loadIVar : ivar should be a dict!";
				return -1;
			}

			str_t section = "";
			str_t s = seeVar(ivar, &section, CONFIG_GLOBAL);
			//printf("====result===== \n%s", s.c_str());
			char * dest = getRebuildBuffer(s.size());
			if(dest == NULL){
				throw ConfigException() << BSL_EARG << "No memory???";
			}
			strcpy(dest, s.c_str());
			return rebuild();
		}catch(bsl::Exception e){
			LOG(WARNING) << "loadIVar : catch exception : [" << e.file() << ":" << e.line() << "]" << e.what();
			return -1;
		}catch(...){
			LOG(WARNING) << "loadIVar : other err...";
			return -1;
		}
		return 0;
	}


	Configure :: Configure(){
		create(CONFIG_GLOBAL);
		_section = this;
		_depth = 0;
		_idl = NULL;
		_cur_reader = NULL;
		_cur_level = -1;
		_dump_buffer = NULL;
		//get_err_unit();
	}

	Configure :: ~Configure(){
		for(size_t i = 0; i < _readers.size(); ++i){
			delete _readers[i]->reader;
			delete _readers[i];
		}
		if(_idl){
			free_idl(_idl);
		}
		if(_dump_buffer){
			free(_dump_buffer);
		}
	}
	
	//callback函数cb_sgConstraint()的参数
	struct cb_cons_arg{
		enum{
			SG_TYPE_SET = 0,	//set _bConstrainted，此处为将其置为false
			SG_TYPE_GET,		//get _bConstrainted，存入vector，为打印做准备
		};
		int sg_type; 
		void *arg;   //callback函数需要的参数，根据作用类型有不同
	};
	//callback函数，用于set/get _bConstrainted
	int cb_sgConstraint(ConfigUnit * unit, void *arg)
	{
		struct cb_cons_arg * cons_arg = (struct cb_cons_arg *)arg;
		if(cons_arg->sg_type == cb_cons_arg::SG_TYPE_SET){
			unit->setConstrainted(*(bool *)cons_arg->arg);
		}
		else if(cons_arg->sg_type == cb_cons_arg::SG_TYPE_GET)
		{
			if(unit->selfType() == CONFIG_UNIT_TYPE && unit->getConstrainted() == false){
				std::vector<ConfigUnit *> *units = (std::vector<ConfigUnit *> *)cons_arg->arg;
				units->push_back(unit);
			}
		}
		return 0;
	}

	int Configure :: check_once(const char * range, int /*version*/) {
		if(range == NULL){
			LOG(WARNING) << "Configure.check_once : range filename error...";
			return ERROR;
		}
		bool bClear = false;
		struct cb_cons_arg cons_arg = {cb_cons_arg::SG_TYPE_SET, &bClear};
		traverse_unit(cb_sgConstraint, (void *)&cons_arg);

		if(_idl){
			free_idl(_idl);
			_idl = NULL;
		}
		_idl = confIDL::alloc_idl();
		load_idl(range, _idl);
		Constraint cons;
		int ret = cons.run(_idl, this);
		return ret==0 ? OK : CONSTRAINT_ERROR;
	}

	int Configure :: checkConstraint()
	{
		std::vector<ConfigUnit *> unconstraintedUnits;
		struct cb_cons_arg cons_arg = {cb_cons_arg::SG_TYPE_GET, &unconstraintedUnits};
		traverse_unit(cb_sgConstraint, (void *)&cons_arg);

		int ret=0, errcode=0;
		for(size_t i=0; i<unconstraintedUnits.size(); ++i){
			LOG(WARNING) << "Configure: in conf file, unconstrainted [key : value] pair: [" <<unconstraintedUnits[i]->get_key_name().c_str() << " : " << unconstraintedUnits[i]->to_cstr(&errcode, "ERROR") << "]";
			ret = CONSTRAINT_ERROR;
		}
		return ret;
	}

	int Configure :: printKeyConstraint(const char *key_path, const char *range)
	{
		if(range != NULL){
			int ret = check_once(range);
			if(ret != 0){
				return ret;
			}
		}
		if(_idl == NULL){
			LOG(WARNING) << 
					"Configure.printKeyConstraint: conf not constrainted, please check_once() or specify a range file";
			return CONSTRAINT_ERROR;
		}
		
		str_t key = str_t(key_path);
		int key_depth = 0;
		confIDL :: group_t *cur_group = NULL;
		for(size_t i = 0; i < key.length();){
			if(key[i] != '.'){
				++ i;
				continue;
			}
			if(i == 0){
				LOG(WARNING) << 
						"Configure.printKeyConstraint: key_path[" << key_path << "] is not well formatted!";
				return CONSTRAINT_ERROR;		
			}				
			str_t group_var = key.substr(0, i);
			key = key.substr(i+1);
			i = 0;
			++ key_depth;

			confIDL :: var_map_t * vmap = (key_depth == 1) ? &_idl->var_map : &cur_group->var_map;
			confIDL :: var_map_t :: iterator vitr = vmap->find(group_var);
			if(vitr == vmap->end()){
				LOG(WARNING) << 
						"Configure.printKeyConstraint: var[" << group_var.c_str() << "] in key_path[" << key_path << "] is not found or not constrainted!";
				return CONSTRAINT_ERROR;		
			}

			str_t group_type = vitr->second->type.data;
			confIDL :: group_map_t :: iterator gitr = _idl->group_map.find(group_type);
			if(gitr == _idl->group_map.end()){
				LOG(WARNING) << 
						"Configure.printKeyConstraint: type[" << group_type.c_str() << "] of var[" << group_var.c_str() << "] in key_path[" << key_path << "] is not found!";
				return CONSTRAINT_ERROR;		
			}
			cur_group = gitr->second;
		}

		confIDL :: var_map_t * vmap = (key_depth == 0) ? &_idl->var_map : &cur_group->var_map;
		confIDL :: var_map_t :: iterator itr = vmap->find(key);
		if(itr == vmap->end()){
			LOG(WARNING) << 
					"Configure.printKeyConstraint: var[" << key.c_str() << "] in key_path[" << key_path << "] is not found or not constrainted!";
			return CONSTRAINT_ERROR;
		}

		LOG(INFO) << "Configure.printKeyConstraint: the constraints of key[" << key_path << "]:";
		confIDL :: cf_list_t &cf_list = itr->second->cf_list;
		confIDL :: cf_list_t :: iterator cf_it;
		for(cf_it=cf_list.begin(); cf_it!=cf_list.end(); ++cf_it){
			confIDL :: cf_t &cf = **cf_it;
			LOG(INFO) << "\t\t\tconstraint function: " << cf.func.data.c_str() << "()"; 
			confIDL :: meta_list_t &arg_list=cf.arg_list;
			confIDL :: meta_list_t :: iterator arg_it;
			for(arg_it=arg_list.begin();arg_it!=arg_list.end();++arg_it){
				confIDL :: meta_t &arg=**arg_it;
				LOG(INFO) << "\t\t\t\tfunction argument: " << arg.data.c_str(); 
			}
		}
		return 0;
	}
}





