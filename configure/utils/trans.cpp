/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: trans.cpp,v 1.8 2009/11/10 04:50:30 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file trans.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/24 15:17:20
 * @version $Revision: 1.8 $ 
 * @brief 
 *  
 **/
#include "cc_utils.h"

namespace comcfg{

	bool Trans :: isInteger(const str_t & str){
		for(int i = 0; i < (int)str.size(); ++i){
			if(isdigit(str[i]) || (str[i] == '-' && i == 0) ||
					(str[i] == '+' && i == 0)){
				continue;
			}
			else{
				return false;
			}
		}
		return true;
	}

	//Float״̬�������ĳ���ַ����Ƿ�Ϊfloat
	//float��������£��ڶ���Ϊ״̬��: 
	//   [+-][digit][.][digit][Ee][+-][digit] '\0'  ->  true false
	// 0  1     2    3    4    5   6     7               -1    -2
	enum{digit = 0, sig=1, dot=2, E=3, end=4};
	static int g_float_table[8][5] = {
		//digit sig dot E end
		{2, 1, 3, -2, -2}, // 0
		{2, -2, 3, -2, -2},// 1
		{2, -2, 3, 5, -1},//2
		{4, -2, -2, -2, -2},//3
		{4, -2, -2, 5, -1}, //4
		{7, 6, -2, -2, -2},//5
		{7, -2, -2, -2, -2},//6
		{7, -2, -2, -2, -1} //7
	};
	//��������g_float_table[2]Ϊ����
	//��ǰ����״̬2 (digit)������������֣���Ȼ����״̬2.������������ţ�����(-2)
	//��������㣬��Ϊ״̬3�����������ĸE����Ϊ״̬5�����������β����ʱ�Ѿ�����isFloatΪtrue(-1)
	bool floatAutoMachine(int status, const str_t & str, int nextpos){
		if(status == -1){
			return true;
		}else if(status == -2){
			return false;
		}
		char nextchar = (int)str.size() > nextpos ? str[nextpos] : '\0';
		int next;
		if(isdigit(nextchar)){
			next = digit;
		}else if(nextchar == '+' || nextchar == '-'){
			next = sig;
		}else if(nextchar == '.'){
			next = dot;
		}else if(nextchar == 'E' || nextchar == 'e'){
			next = E;
		}else if(nextchar == '\0'){
			next = end;
		}else{
			return false;
		}
		return floatAutoMachine(g_float_table[status][next], str, nextpos+1);
	}

	bool Trans :: isFloat(const str_t & str){
		return floatAutoMachine(0, str, 0);
	}

	static str_t getFirstPart(const str_t & str){
		for(int i = 0; i < (int)str.size(); ++i){
			if(isspace(str[i]) || str[i] == '#'){
				return str.substr(0, i);
			}
		}
		return str;
	}

#if 0
	int Trans :: str2double(const str_t & __str, double * buf){
		if(! __str.size()){
			ul_writelog(Log::notice(), "Config : Null value");
			return NULLVALUE;
		}
		str_t str = getFirstPart(__str);
		if(isFloat(str)){
			if( sscanf(str.c_str(), "%lf", buf) == 1 ){
				ul_writelog(Log::trace(), "Read value [%s] as double/float = [%g]", __str.c_str(), *buf);
				return 0;
			}
		}
		ul_writelog(Log::notice(), "Config : Format error");
		return FORMATERROR;
	}
#endif
	int Trans :: str2double(const str_t & __str, double * buf){
		if(! __str.size()){
			LOG(INFO) << "Config : Null value";
			return NULLVALUE;
		}
		str_t str = getFirstPart(__str);
		char *endptr = NULL;
		errno = 0;
		*buf = strtod(str.c_str(), &endptr);
		if(errno == ERANGE){
			LOG(INFO) << "Config : str2double : out of range";
			return OUTOFRANGE;
		}
		if(*endptr != '\0'){
			LOG(INFO) << "Config : str2double : Format error char=[" << *endptr << "]";
			return FORMATERROR;
		}
		return 0;
	}
#if 0		
	static const str_t max_uint = "18446744073709551615";
	static bool uint64_range(str_t str){
		int i;
		for(i = 0; i < (int)str.size(); ++i){
			if(str[i] != '0'){
				break;
			}
		}
		if(i){
			str = str.substr(i);
		}
		if(str.size() > max_uint.size() ||
				(str.size() == max_uint.size() && max_uint < str)){
			return false;
		}
		return true;
	}
#endif
	int Trans :: str2uint64(const str_t & __str, unsigned long long * buf){
		if(! __str.size()){
			LOG(INFO) << "Config : Null value";
			return NULLVALUE;
		}
		str_t str = getFirstPart(__str);
		char *endptr = NULL;
		if(str[0] == '-'){
			//strtoull��һ�����⣬�Ὣ-1ת����0xFFFFFFFF
			return FORMATERROR;
		}
		errno = 0;
		*buf = strtoull(str.c_str(), &endptr, 0);
		if(errno == ERANGE){
			LOG(INFO) << "Config : str2uint64 : Out of range";
			return OUTOFRANGE;
		}
		if(*endptr != '\0'){
			LOG(INFO) << "Config : str2uint64 : Format error char=[" << *endptr << "]";
			return FORMATERROR;
		}
		return 0;
	}

	int Trans :: str2int64(const str_t & __str, long long * buf){
		if(! __str.size()){
			LOG(INFO) << "Config : Null value";
			return NULLVALUE;
		}
		str_t str = getFirstPart(__str);
		char *endptr = NULL;
		errno = 0;
		*buf = strtoll(str.c_str(), &endptr, 0);
		if(errno == ERANGE){
			LOG(INFO) << "Config : str2int64 : Out of range";
			return OUTOFRANGE;
		}
		if(*endptr != '\0'){
			LOG(INFO) << "Config : str2int64 : " << __str.c_str() << " Format error char=[" << *endptr << "]";
			return FORMATERROR;
		}
		return 0;
	}
#if 0
	//�����ӿ�
	int Trans :: str2int(const str_t & __str, Trans :: Integer * buf){
		if(! __str.size()){
			ul_writelog(Log::notice(), "Config : Null value");
			return NULLVALUE;
		}
		str_t str = getFirstPart(__str);
		if(isInteger(str)){
			if(str[0] == '-'){
				if(! uint64_range(str.substr(1))){
					ul_writelog(Log::notice(), "Config : Out of range");
					return OUTOFRANGE;
				}
				buf->negative = true;
				if( sscanf(str.c_str(), "%llu", &buf->val) == 1){
					ul_writelog(Log::debug(), "Read value [%s] as Integer = [-%llu]", __str.c_str(), buf->val);
					return 0;
				}
			}
			else{
				if(! uint64_range(str)){
					ul_writelog(Log::notice(), "Config : Out of range");
					return OUTOFRANGE;
				}
				buf->negative = false;
				if( sscanf(str.c_str(), "%llu", &buf->val) == 1){
					ul_writelog(Log::debug(), "Read value [%s] as Integer = [%llu]", __str.c_str(), buf->val);
					return 0;
				}
			}
		}
		ul_writelog(Log::notice(), "Config : Format error");
		return FORMATERROR;
	}
#endif
	/**
	 * \0	���ַ�(NULL)	00H/0
	 * \n	���з�(LF)	0AH/10
	 * \r	�س���(CR)	0DH/13
	 * \t	ˮƽ�Ʊ��(HT)	09H/9
	 * \v	��ֱ�Ʊ�(VT)	0B/11
	 * \a	����(BEL) 	07/7
	 * \b	�˸��(BS)	08H/8
	 * \f	��ҳ��(FF)	0CH/12
	 * \��	������	27H/39
	 * \"	˫����	22H/34
	 * \\	��б��	5CH/92
	 * \ddd 	�����ַ� 	��λ�˽���
	 * \xhh 	�����ַ� 	��λʮ������
	 */
	static int hex2int(char c){
		if(c >= 'A' && c <= 'F'){
			return int(c - 'A' + 10);
		}
		if(c >= 'a' && c <= 'f'){
			return int(c - 'a' + 10);
		}
		if(c >= '0' && c <= '9'){
			return int(c - '0');
		}
		return -255;
	}
	int Trans :: str2str(const str_t & str, str_t * buf){
		if(str.size() == 0 || str[0] != '\"'){
			*buf = str;
			return 0;
		}
		else{
			*buf = "";
			for(int i = 1; i < (int)str.size(); ++i){
				if( str[i] == '\"'){
					return 0;
				}
				else if(str[i] == '\\'){
					++i;
					if(i >= (int)str.size()){
						return FORMATERROR;
					}
					switch(str[i]){
						case '0':
							buf->push_back('\0');break;
						case 'n':
							buf->push_back('\n');break;
						case 'r':
							buf->push_back('\r');break;
						case 't':
							buf->push_back('\t');break;
						case 'v':
							buf->push_back('\v');break;
						case 'a':
							buf->push_back('\a');break;
						case 'b':
							buf->push_back('\b');break;
						case 'f':
							buf->push_back('\f');break;
						case '\'':
							buf->push_back('\'');break;
							case '\"':
								buf->push_back('\"');break;
						case '\\':
							buf->push_back('\\');break;
						case 'x':
						default:
							if(i+2 >= (int)str.size()){
								return FORMATERROR;
							}
							if(str[i] == 'x'){
								int c = hex2int(str[i+1]) * 16 + hex2int(str[i+2]);
								if(c < 0){
									return FORMATERROR;
								}
								else{
									buf->push_back(char(c));
								}
							}
							else if(str[i] >= '0' && str[i] < '8'){
								int c = int(str[i] - '0') * 64 + int(str[i+1] - '0') * 8 + int(str[i+2] - '0');
								if(c > 255){
									return FORMATERROR;
								}
								else{
									buf->push_back(char(c));
								}
							}
							else{
								return FORMATERROR;
							}
							i += 2;
					}
				}
				else{
					buf->push_back(str[i]);
				}
			}
		}
		return FORMATERROR;
	}

	str_t Trans :: cutComment(const str_t & str){
		for(int i = 0; i < (int)str.size(); ++i){
			if(str[i] == '#'){
				return str.substr(0, i);
			}
		}
		return str;
	}

}















/* vim: set ts=4 sw=4 sts=4 tw=100 */
