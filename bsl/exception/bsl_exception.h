/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_exception.h,v 1.12 2009/10/14 08:24:58 chenxm Exp $ 
 * 
 **************************************************************************/

/**
 * @file bsl_exception.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/07/25 17:40:02
 * @version $Revision: 1.12 $ 
 * @brief 
 *  
 **/

#ifndef  __BSL_EXCEPTION_H__
#define  __BSL_EXCEPTION_H__

#include<exception>
#include<cstdio>
#include<iostream>
#include<typeinfo>
#include<cstdarg>
#include"bsl/AutoBuffer.h"
#include"bsl/exception/stack_trace.h"

/**
* @brief �쳣ת��־�Ķ�ӦWARNING��־����
*  
* Ĭ��ʹ��UB_LOG_WARNING
* ���ʹ��������־��������ȷ����ӿ���UB_LOG_WARNINGһ��
*/
#ifndef __BSL_LOG_WARNING__
#define __BSL_LOG_WARNING__ UB_LOG_WARNING
#endif

/**
* @brief �쳣ת��־�Ķ�ӦFATAL��־����
*  
* Ĭ��ʹ��UB_LOG_FATAL
* ���ʹ��������־��������ȷ����ӿ���UB_LOG_FATALһ��
*/
#ifndef __BSL_LOG_FATAL__   
#define __BSL_LOG_FATAL__   UB_LOG_FATAL
#endif

/**
* @brief �����׳��쳣λ�õĺ�
*  
* �ú�ʵ������һ��bs::ExceptionArg������������Ƿǳ���ȫ��
*/
#define BSL_EARG \
    bsl::ExceptionArg( __PRETTY_FUNCTION__, __FILE__, __LINE__ )


namespace bsl {

    enum exception_level_t{
        EXCEPTION_LEVEL_SILENT     = -999,
        EXCEPTION_LEVEL_DEBUG      = 0,
        EXCEPTION_LEVEL_TRACE      = 100,
        EXCEPTION_LEVEL_NOTICE     = 200,
        EXCEPTION_LEVEL_WARNING    = 400,
        EXCEPTION_LEVEL_FATAL      = 800,
        EXCEPTION_LEVEL_CORE_DUMPED= 999,

        EXCEPTION_LEVEL_DEFAULT    = EXCEPTION_LEVEL_DEBUG,

    };

    static const char * const EXCEPTION_LEVEL_SILENT_CSTRING    = "EXCEPTION_LEVEL_SILENT";
    static const char * const EXCEPTION_LEVEL_DEBUG_CSTRING     = "EXCEPTION_LEVEL_DEBUG";
    static const char * const EXCEPTION_LEVEL_TRACE_CSTRING     = "EXCEPTION_LEVEL_TRACE";
    static const char * const EXCEPTION_LEVEL_NOTICE_CSTRING    = "EXCEPTION_LEVEL_NOTICE";
    static const char * const EXCEPTION_LEVEL_WARNING_CSTRING   = "EXCEPTION_LEVEL_WARNING";
    static const char * const EXCEPTION_LEVEL_FATAL_CSTRING     = "EXCEPTION_LEVEL_FATAL";
    static const char * const EXCEPTION_LEVEL_CORE_DUMPED_CSTRING="EXCEPTION_LEVEL_CORE_DUMPED";
    static const char * const EXCEPTION_LEVEL_DEFAULT_CSTRING   = EXCEPTION_LEVEL_DEBUG_CSTRING;
    static const char * const EXCEPTION_LEVEL_UNKNOWN_CSTRING   ="EXCEPTION_LEVEL_KNOWN";

    const char * to_cstring(exception_level_t level);

    struct ExceptionArg {
        ExceptionArg( const char * function, const char * file, int line )
            :_function(function), _file(file), _line(line){}

        const char *_function;
        const char *_file;
        int         _line;

    };

    template<typename ExceptionT, typename RealBaseExceptionT>
        class BasicException: public RealBaseExceptionT{
        public:

            template<typename ValueT>
            ExceptionT& push( ValueT value ){
                this->_msg.push(value);
                check_type();
                return static_cast<ExceptionT&>(*this);
            }

            template<typename Value1T, typename Value2T>
            ExceptionT& push( Value1T value1, Value2T value2 ){
                this->_msg.push(value1, value2);
                check_type();
                return static_cast<ExceptionT&>(*this);
            }

            ExceptionT& pushf( const char * format, ... )__attribute__ ((format (printf, 2, 3) ));

            ExceptionT& vpushf( const char * format, va_list ap ){
                this->_msg.vpushf(ap);
                check_type();
                return static_cast<ExceptionT&>(*this);
            }

            ExceptionT& push( const ExceptionArg& __arg ){
                this->_arg = __arg;
                check_type();
                return static_cast<ExceptionT&>(*this);
            }

            ExceptionT& push( exception_level_t __level ){
                this->_level = __level;
                if ( this->_level >= EXCEPTION_LEVEL_CORE_DUMPED ){
                    this->core_dump();
                }
                check_type();
                return static_cast<ExceptionT&>(*this);
            }

            template<typename ValueT>
            ExceptionT& operator << ( ValueT value ){
                this->_msg<<value;
                check_type();
                return static_cast<ExceptionT&>(*this);
            }

            ExceptionT& operator << ( const ExceptionArg& __arg ){
                this->_arg = __arg;
                check_type();
                return static_cast<ExceptionT&>(*this);
            }

            ExceptionT& operator << ( exception_level_t __level ){
                this->_level = __level;
                check_type();
                return static_cast<ExceptionT&>(*this);
            }

        private:
            virtual void check_type(){
                if ( &typeid(ExceptionT) != &typeid(*this) ){
                    this->_msg.push("WARNING: invalid definition of ").push(static_cast<ExceptionT*>(this)->name());
                    if ( this->_level < EXCEPTION_LEVEL_WARNING ){
                        this->_level = EXCEPTION_LEVEL_WARNING;
                    }
                }
            }


        };

        
    //write this way because of a bug of g++ 2.96
    template<typename ExceptionT, typename RealBaseExceptionT>
        inline ExceptionT& BasicException<ExceptionT,RealBaseExceptionT>::pushf( const char * format, ... ){
            va_list ap;
            va_start( ap, format );
            this->_msg.vpushf( format, ap );
            va_end(ap);
            check_type();
            return static_cast<ExceptionT&>(*this);
        }

    /**
    * @brief BSL��һ���쳣��������
    *  
    *  Ĭ�ϳ�ʼ��ʱ����AutoBuffer�����һ����ʼ���������Լ��ٶ�̬�ڴ������������������㣬AutoBuffer���Զ���������
    *  ��Exception����Ա�����Ϊ�ڱ��׳�ʱ��/�򱻽�ס�����������׳�ʱ����Ļ/��־�д�ӡ������Ϣ��ջ��Ϣ���Ա���Ժʹ������
    */
    class ExceptionBase: public ::std::exception {
    public:
        typedef ::std::exception base_type;
        ExceptionBase()
            : base_type(), _level(EXCEPTION_LEVEL_DEFAULT), 
            _stack(DEFAULT_STACK_CAPACITY), _name(DEFAULT_NAME_CAPACITY), _msg(DEFAULT_WHAT_CAPACITY), _all(0), _arg(DEFAULT_FUNCTION, DEFAULT_FILE, DEFAULT_LINE){
            }

        ExceptionBase(const ExceptionBase& other)
            : base_type(other), _level(other._level), 
            _stack(other._stack.capacity()), _name(other._name.capacity()), _msg(other._msg.capacity()), _arg(other._arg) {
                _stack.transfer_from( other._stack );
                _name.transfer_from( other._name );
                _msg.transfer_from( other._msg );
            }

        virtual ~ExceptionBase() throw() { }

    public:
        static const size_t DEFAULT_MSG_CAPACITY = 64;
        static const size_t DEFAULT_STACK_CAPACITY = 64;
        static const size_t DEFAULT_NAME_CAPACITY = 32;
        static const size_t DEFAULT_WHAT_CAPACITY = 256;
        static const char * const DEFAULT_FILE;
        static const char * const DEFAULT_FUNCTION;
        static const int DEFAULT_LINE = 0;
    protected:
        exception_level_t   _level;
        mutable AutoBuffer  _stack;
        mutable AutoBuffer  _name;
        mutable AutoBuffer  _msg;
        mutable AutoBuffer  _all;
        ExceptionArg        _arg;

    };

    class Exception: public BasicException<Exception, ExceptionBase>{
    public:

        typedef BasicException<Exception, ExceptionBase> base_type;

        /**
         * @brief Ĭ�Ϲ��캯��
         *
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:28:12 / modified by zhujianwei at 2010/12/27
        **/
        Exception()
            :base_type(){
                stack_trace(_stack, _s_stack_trace_level, 1, MAX_STACK_TRACE_LEVEL,  _s_line_delimiter);
            }

        /**
         * @brief ��������
         *
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:28:43
        **/
        virtual ~Exception() throw() { }

        /**
         * @brief �����쳣����Я����ȫ����Ϣ
         *
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/08 15:09:40
        **/
        virtual const char * what() const throw() {
            return _msg.c_str();
        }

        /**
         * @brief ����Exception���������
         *
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:28:25
        **/
        const char * name() const  {
            if ( _name.empty() ){
                demangle( _name, typeid(*this).name() );
            }
            return _name.c_str();
        }

        /**
         * @brief �����쳣�׳�ʱ��ջ������Ϣ��
         *
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:28:57
        **/
        const char * stack() const {
            return _stack.c_str();
        }

        /**
         * @brief �����쳣�׳����ڵĺ�����
         *
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:29:34
        **/
        const char * function() const {
            return _arg._function;
        }

        /**
         * @brief �����쳣�׳����ڵ��ļ���
         *
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:30:07
        **/
        const char * file() const {
            return _arg._file;
        }

        /**
         * @brief �����쳣�׳����ڵ��к�
         *
         * @return  int 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:31:35
        **/
        int line() const {
            return _arg._line;
        }

        /**
         * @brief �����쳣�����ؼ���
         *
         * @return  int 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:32:00
        **/
        int level() const {
            return _level;
        }

        /**
         * @brief �����쳣���ؼ����C����ַ���
         *
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:32:16
        **/
        const char * level_str() const {
            return to_cstring( _level );
        }

        /**
         * @brief ���ذ���������������Ϣ���ַ���
         *
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/09/27 16:15:01
        **/
        const char * all() const;
        
    public:
        /**
         * @brief ���õ��쳣�׳�ʱ��ջ���ٵĲ���
         *
         * ���û�����ù���Ĭ��ΪDEFAULT_STACK_TRACE_LEVEL
         * ��������ǰ��ջ���ٲ���
         * ע�������Խӿڣ��������ܻ�ı�
         *
         * @param [in] on   : bool
         * @return  size_t 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:32:40
        **/
        static size_t set_stack_trace_level(size_t level_){
            size_t old = _s_stack_trace_level;
            _s_stack_trace_level = level_ ;
            return old;
        }

        /**
         * @brief ���÷��з�
         *
         * Ĭ�Ϸ��з�Ϊ"<CR>"��Ŀ����Ϊ�˷�ֹ����־ʱ\nӰ���ء��û��ɸ�����Ҫ�趨���з����������Ҫ��أ���Ϊ"\n"����
         * ע�������Խӿڣ��������ܻ�ı�
         *
         * @param [in] delimiter   : const char*
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/14 17:12:12
        **/
        static void set_line_delimiter( const char * delimiter ){
            snprintf( _s_line_delimiter, _S_LINE_DELIMITER_SIZE, "%s", delimiter );
        }

        /**
         * @brief ��ȡ�����õķ��з�
         *
         * ע�������Խӿڣ��������ܻ�ı�
         *
         * @param [in] delimiter   : const char*
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/14 17:16:26
        **/
        static const char *get_line_delimiter(){
            return _s_line_delimiter;
        }

        /**
        * @brief Ĭ�ϴ�ӡջ���ٵĲ���
        *  
        *  ע�������Խӿڣ��������ܻ�ı�
        */
        static const size_t DEFAULT_STACK_TRACE_LEVEL = 10;

        /**
        * @brief �����Ը��ٵ��Ĳ���
        *  
        *  ע�������Խӿڣ��������ܻ�ı�
        */
        static const size_t MAX_STACK_TRACE_LEVEL     = 100;


    protected:
        
        /**
         * @brief �ֶ���core�����쳣����ΪCORE_DUMPʱ������
         *
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:40:03
        **/
        static void core_dump();
        
    private:
        static const char * const DEFAULT_LINE_DELIMITER;
        static const size_t       _S_LINE_DELIMITER_SIZE    = 10;
        static size_t _s_stack_trace_level;
        static char _s_line_delimiter[_S_LINE_DELIMITER_SIZE];
    };

    /**
    * @brief ��׼���쳣(std::exception������)�İ����ࡣ
    *  
    *  �ʺ���ʹ���˻����쳣�ı�׼�������װ�쳣�á�
    */
    class StdException: public BasicException<StdException, Exception>{
    public:
        typedef BasicException<StdException, Exception> base_type;
        StdException( ::std::exception& e )
            :base_type(){
            demangle( _name, typeid(e).name() );
            _msg.push(e.what());
        }
    };

    /**
    * @brief δ֪�쳣
    *  
    *  Ӧ�þ�������ʹ�ø��쳣��������ʹ���������쳣ԭ����쳣��
    */
    class UnknownException: public BasicException<UnknownException, Exception>{};

    /**
    * @brief ����ת��ʧ���쳣
    *  
    *  ���쳣��ʾ�����˲�֧�ֵ�����ת��
    */
    class BadCastException: public BasicException<BadCastException, Exception>{};

    /**
    * @brief ����Խ���쳣
    *  
    *  ���쳣��ʾ����Խ��
    */
    class OutOfBoundException: public BasicException<OutOfBoundException, Exception>{};

    /**
    * @brief ���������쳣
    *  
    *  ���쳣��ʾ���Ҽ�������
    */
    class KeyNotFoundException: public BasicException<KeyNotFoundException, Exception>{};

    /**
    * @brief ���Ѵ����쳣
    *  
    *  ���쳣��ʾ���Ҽ��Ѿ�����
    */
    class KeyAlreadyExistException: public BasicException<KeyAlreadyExistException, Exception>{};

    /**
    * @brief ���붯̬�ڴ�ʧ���쳣
    *  
    *  ���쳣��ʾ��̬�ڴ�����ʧ��
    */
    class BadAllocException: public BasicException<BadAllocException, Exception>{};

    /**
    * @brief ���������쳣
    *  
    *  ���쳣��ʾ���ú����Ĳ������Ϸ��������ʹ����������������������Ϸ�ԭ����쳣�Ļ�����������ʹ�ñ��쳣
    */
    class BadArgumentException: public BasicException<BadArgumentException, Exception>{};

    /**
    * @brief ��ָ���쳣
    *  
    *  ���쳣��ʾ�ڲ��ܽ��ܿ�ָ��ĵط������˿�ָ��
    */
    class NullPointerException: public BasicException<NullPointerException, Exception>{};

    /**
    * @brief ����ĸ�ʽ�ַ����쳣
    *  
    *  ���쳣��ʾ��ʽ�ַ������Ϸ���
    */
    class BadFormatStringException: public BasicException<BadFormatStringException, Exception>{};

    /**
    * @brief δ��ʼ���쳣
    *  
    *  ���쳣��ʾʹ����δ����ʼ���Ķ���
    */
    class UninitializedException: public BasicException<UninitializedException, Exception>{};

    /**
    * @brief δʵ���쳣
    *  
    *  ���쳣��ʾ������δ��ʵ�ֵ�API
    */
    class NotImplementedException: public BasicException<NotImplementedException, Exception>{};

    /**
    * @brief ��Ч�����쳣
    *  
    *  ���쳣��ʾִ������Ч�Ĳ���
    */
    class InvalidOperationException: public BasicException<InvalidOperationException, Exception>{};

    /**
    * @brief ������쳣
    *  
    *  ���쳣��ʾ��ֵ���ܽ��ܵ����ֵ��Ҫ��
    */
    class OverflowException: public BasicException<OverflowException, Exception>{};

    /**
    * @brief ������쳣
    *  
    *  ���쳣��ʾ��ֵ���ܽ��ܵ���Сֵ��ҪС
    */
    class UnderflowException: public BasicException<UnderflowException, Exception>{};

    /**
    * @brief ���﷨������ʧ���쳣
    *  
    *  ���쳣��ʾ�﷨����ʧ��
    */
    class ParseErrorException: public BasicException<ParseErrorException, Exception>{};

    /**
    * @brief IO�쳣
    *  
    */
    class IOException : public bsl::BasicException< IOException, bsl::Exception >{};

    /**
    * @brief �Ҳ����ļ��쳣
    *  
    *  
    */
    class FileNotFoundException : public bsl::BasicException< FileNotFoundException, IOException >{};

    /**
    * @brief �ļ���Ϣ�쳣
    *  
    *  
    */
    class FstatException : public bsl::BasicException< FstatException, IOException >{};

    /**
    * @brief mmap�쳣
    *  
    *  
    */
    class MmapException : public bsl::BasicException< MmapException, IOException >{};

    /**
    * @brief ��̬�����쳣
    *  
    *  
    */
    class DynamicLinkingException : public bsl::BasicException< DynamicLinkingException, IOException >{};

    /**
    * @brief ����ʧ���쳣
    *  
    *  
    */
    class AssertionFailedException : public bsl::BasicException< AssertionFailedException, bsl::Exception >{};


}//namespace bsl 
#endif  //__BSL_EXCEPTION_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
