/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: cfgflag.cpp,v 1.3 2009/11/10 04:50:30 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file init.cpp
 * @author xiaowei(com@baidu.com)
 * @date 2009/02/12 13:57:10
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/

#include "cfgflag.h"
#include "cfgext.h"

namespace comcfg
{

const char *DEFHELP = 
	" -d:     conf_dir\n"
	" -f:     conf_file\n"
	" -r:	  range_file\n"
	" -h:     show this help page\n"
	" -t:     check config file syntax\n"
	" -g:     generate config sample file\n"
	" -v:     show version infomation\n ";

void Flag::default_version()
{
	std::cout<<"call default_version function, you must reload it"<<std::endl;
}

void Flag::default_help()
{
	printf ("%s", DEFHELP);
}

static int rfind(const char *s, char flag) {
	if (s == NULL) return -1;
	int z = strlen(s);
	do {
		--z;
	} while (z>=0 && s[z] != flag);
	return z;
}

Flag::Flag(fun_t ver, fun_t help)
{
	_modname = "";
	_cfpath = "";
	_cffile = "";
	_cfrange = "";
	_version = ver;
	_help = help;
}

Flag::~Flag() 
{
}

bsl::string Flag::get_defrange(const bsl::string &file)
{
	bsl::string str=file;
	if (file.size() > 5 && file.substr(file.size()-5) == ".conf") {
		str = file.substr(0,file.size()-5);
	}
	str.append(".range");
	return str;
}

bsl::string Flag::get_defconf(const bsl::string &mod)
{
	int pos = comcfg::rfind(mod.c_str(), '/');
	bsl::string cf;
	if (pos < 0) {
		cf = bsl::string(mod.c_str()).append(".conf");
	} else {
		cf = bsl::string(mod.c_str()+pos+1).append(".conf");
	}
	return bsl::string("conf/").append(cf);
}

int Flag::init(int argc, char **argv, const char *optstr)
{
	int ret = 0;
	try {
		if (optstr == NULL) {
			ret = _opt.init(argc, argv, bsl::string("f:d:r:gtvh").c_str());
		} else {
			ret = _opt.init(argc, argv, bsl::string("f:d:r:gtvh").append(optstr).c_str());
		}
		if (ret != 0) return ret;

		_modname = argv[0];

		if (_opt.hasOption("d")) {
			_cfpath = _opt["d"].to_cstr();
		} else if (_opt.hasOption("dir")) {
			_cfpath = _opt["dir"].to_cstr();
		} else {
			_cfpath = "";
		}

		if (_opt.hasOption("f")) {
			_cffile = _opt["f"].to_cstr();
		} else if (_opt.hasOption("conf")) {
			_cffile = _opt["conf"].to_cstr();
		} else {
			_cffile = "";
		}

		if (_opt.hasOption("r")) {
			_cfrange = _opt["r"].to_cstr();
		} else {
			_cfrange = "";
		}

		if (_opt.hasOption("v") || _opt.hasOption("version")) {
			_version();
			exit (0);
		}
		if (_opt.hasOption("h") || _opt.hasOption("help")) {
			_help();
			exit (0);
		}
		if (_opt.hasOption("t")) {
			ret = 0;
			try {
				ret = this->loadconfig();
			} catch (bsl::Exception &e) {
				LOG(WARNING) << "check configure error : exception " << e.what() << ", at " << e.stack();
				ret = -1;
			}
			if (ret != 0) {
				LOG(WARNING) << "check configure[" << this->_cfpath.c_str() << "/" << this->_cffile.c_str() << "] error";
			} else {
				LOG(INFO) << "check configure[" << _cfpath.c_str() << "/" << _cffile.c_str() << "] success";
			}
			exit (0);
		}
		if (_opt.hasOption("g")) {
			bsl::string file = "";
			try {
				auto_def();
				file = comcfg::autoConfigGen(fullpath(_cfpath, _cfrange).c_str());
			} catch (bsl::Exception &e) {
				LOG(WARNING) << "auto gen configure error for " << _cfpath.c_str() << "/" << _cffile.c_str() << ":\nExp " << e.what() << ", " << e.name() << ", " << e.stack();
				exit(0);
			}
			bsl::string path = _cfpath;
			path.append("/").append(_cffile);
			FILE *fp = fopen(path.c_str(), "w");
			if (fp == NULL) {
				LOG(WARNING) << "auto gen configure error for can't open file [" << path.c_str() << "]";
				exit(0);
			}
			fwrite(file.c_str(), 1, file.size(), fp);
			fclose(fp);
			LOG(INFO) << "auto configure " << path.c_str() << " success";
			exit (0);
		}

		return 0;
	} catch (bsl::Exception &e) {
		return -1;
	}
}

bsl::string Flag::fullpath(const bsl::string &path, const bsl::string &file)
{
	bsl::string full = path;
	return full.append("/").append(file);
}

bool Flag::file_exist(const char *fn)
{
	FILE *fp = fopen(fn, "r");
	if (fp) {
		fclose(fp);
		return true;
	}
	return false;
}

int Flag::loadconfig()
{
	try {
		auto_def();
		const char *r = NULL;
		if (file_exist(fullpath(_cfpath, _cfrange).c_str())) {
			r = _cfrange.c_str();
		}
		if (r != NULL) {
			LOG(INFO) << "configure.load(" << _cfpath.c_str() << ", " << _cffile.c_str() << ", " << r << "); check range: " << _cfrange.c_str() << " exist";
		} else {
			LOG(INFO) << "configure.load(" << _cfpath.c_str() << ", " << _cffile.c_str() << ", NULL); check range: " << _cfrange.c_str() << " noexist";
		}
		int ret = _cfg.load(_cfpath.c_str(), _cffile.c_str(), r);
		if (ret != 0) {
			return ret;
		}
	} catch (bsl::Exception &e) {
		return -1;
	}
	return 0;
}

int Flag::loadconfig(const char *path, const char *conf) 
{
	if (path == NULL) {
		_cfpath = "";
	} else {
		_cfpath = path;
	}
	if (conf == NULL) {
		_cffile = "";
	} else {
		_cffile = conf;
	}
	return loadconfig();
}

void Flag::set_confpath(const char *path, const char *file, const char *range)
{
	if (path) { 
		set_cfpath(path); 
	} else { 
		set_cfpath(""); 
	}
	if (file) { 
		set_cffile(file); 
	} else { 
		set_cffile(""); 
	}
	if (range) { 
		set_cfrange(range); 
	} else { 
		set_cfrange(""); 
	}
}

void Flag::auto_def()
{
	if (_cffile.size() == 0) {
		_cffile = get_defconf(_modname);
	}
	if (_cfpath.size() == 0) {
		int pos = rfind(_cffile.c_str(), '/');
		if (pos >= 0) {
			_cfpath = _cffile.substr(0, pos+1);
			_cffile = _cffile.substr(pos+1);
		} else {
			_cfpath = "./";
		}
	}
	if (_cfrange.size() == 0) {
		_cfrange = get_defrange(_cffile);
	}
}

}

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
