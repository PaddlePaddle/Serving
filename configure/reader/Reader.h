/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Reader.h,v 1.5 2009/02/13 15:25:31 yingxiang Exp $ 
 * 
 **************************************************************************/



/**
 * @file Reader.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/21 16:09:58
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/


#ifndef  __READER_H_
#define  __READER_H_

#include "RawData.h"
#include "Configure.h"
#include <sys/uio.h>

namespace comcfg{
#define REBUILD_CONF "REBUILD_CONF"
	class Reader{
			RawData _fd;
		public:
			int read(const char * file, Configure * cfg);
			char * getRebuildBuffer(size_t size);
			int rebuild(Configure * cfg);
			char * dump(size_t *);
			Reader();
		private:
			int build(Configure * cfg);
			int push(Configure * cfg, const char * buf, size_t length);
			struct pair_t{
				str_t key;
				str_t value;
			};
			int sectionParser(const char * buf, size_t length, str_t * str);
			int keyParser(const char * buf, size_t length, pair_t * pair);
			int getWord(const char * from, const char * to, str_t * str);
			void getCommentUpwards(str_t &comment, int line);
			int isCommentLine(const char *);//comment line or blank line
			friend class ConfigUnit;
			friend class Configure;
			friend class ConfigGroup;
			friend int cb_dumpConf(ConfigUnit *, void *);
			str_t _cur_file;
			int _cur_line;
			bool _isRebuild;
	};
}
















#endif  //__READER_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
