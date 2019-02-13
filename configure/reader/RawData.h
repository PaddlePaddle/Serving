/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: RawData.h,v 1.4 2009/02/13 15:25:31 yingxiang Exp $ 
 * 
 **************************************************************************/



/**
 * @file RawData.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/18 22:52:32
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/


#ifndef  __RAWDATA_H_
#define  __RAWDATA_H_

#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <vector>
#include "utils/cc_exception.h"
#include "butil/logging.h"

namespace comcfg{
	class RawData{
		public:

			int load(const char * filename);
			char * getRebuildBuffer(size_t size);
			int rebuild();
			char * dump(size_t *);
			size_t lineNum();
			size_t size();
			const char * getLine(int idx);
			RawData();
			~RawData();
		protected:
			std::vector <size_t> _line;
			size_t _lnum;
			char * _file;
			char * _file_original;
			size_t _fsize;
			size_t _bufsize;
	};
}
















#endif  //__RAWDATA_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
