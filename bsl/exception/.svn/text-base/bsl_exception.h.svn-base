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
* @brief 异常转日志的对应WARNING日志函数
*  
* 默认使用UB_LOG_WARNING
* 如果使用其它日志函数，请确认其接口与UB_LOG_WARNING一致
*/
#ifndef __BSL_LOG_WARNING__
#define __BSL_LOG_WARNING__ UB_LOG_WARNING
#endif

/**
* @brief 异常转日志的对应FATAL日志函数
*  
* 默认使用UB_LOG_FATAL
* 如果使用其它日志函数，请确认其接口与UB_LOG_FATAL一致
*/
#ifndef __BSL_LOG_FATAL__   
#define __BSL_LOG_FATAL__   UB_LOG_FATAL
#endif

/**
* @brief 传递抛出异常位置的宏
*  
* 该宏实际上是一个bs::ExceptionArg无名对象，因而是非常安全的
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
    * @brief BSL中一切异常的祖先类
    *  
    *  默认初始化时，各AutoBuffer会分配一个初始的容量，以减少动态内存分配次数，若容量不足，AutoBuffer会自动增加容量
    *  该Exception类可以被设置为在被抛出时和/或被接住并不再重新抛出时向屏幕/日志中打印错误消息及栈信息，以便调试和错误跟踪
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
         * @brief 默认构造函数
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
         * @brief 析构函数
         *
         * @see 
         * @author chenxm
         * @date 2009/02/04 16:28:43
        **/
        virtual ~Exception() throw() { }

        /**
         * @brief 返回异常对象携带的全部信息
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
         * @brief 返回Exception对象的类名
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
         * @brief 返回异常抛出时的栈跟踪信息。
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
         * @brief 返回异常抛出所在的函数名
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
         * @brief 返回异常抛出所在的文件名
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
         * @brief 返回异常抛出所在的行号
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
         * @brief 返回异常的严重级别
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
         * @brief 返回异常严重级别的C风格字符串
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
         * @brief 返回包含有上述所有信息的字符串
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
         * @brief 设置当异常抛出时，栈跟踪的层数
         *
         * 如果没有设置过，默认为DEFAULT_STACK_TRACE_LEVEL
         * 返回设置前的栈跟踪层数
         * 注：试验性接口，将来可能会改变
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
         * @brief 设置分行符
         *
         * 默认分行符为"<CR>"，目的是为了防止打日志时\n影响监控。用户可根据需要设定分行符，如果不需要监控，设为"\n"即可
         * 注：试验性接口，将来可能会改变
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
         * @brief 获取被设置的分行符
         *
         * 注：试验性接口，将来可能会改变
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
        * @brief 默认打印栈跟踪的层数
        *  
        *  注：试验性接口，将来可能会改变
        */
        static const size_t DEFAULT_STACK_TRACE_LEVEL = 10;

        /**
        * @brief 最大可以跟踪到的层数
        *  
        *  注：试验性接口，将来可能会改变
        */
        static const size_t MAX_STACK_TRACE_LEVEL     = 100;


    protected:
        
        /**
         * @brief 手动出core，当异常级别为CORE_DUMP时被调用
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
    * @brief 标准库异常(std::exception的子类)的包裹类。
    *  
    *  适合于使用了会抛异常的标准库组件包装异常用。
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
    * @brief 未知异常
    *  
    *  应该尽量避免使用该异常。尽可能使用能描述异常原因的异常。
    */
    class UnknownException: public BasicException<UnknownException, Exception>{};

    /**
    * @brief 类型转换失败异常
    *  
    *  该异常表示发生了不支持的类型转换
    */
    class BadCastException: public BasicException<BadCastException, Exception>{};

    /**
    * @brief 访问越界异常
    *  
    *  该异常表示访问越界
    */
    class OutOfBoundException: public BasicException<OutOfBoundException, Exception>{};

    /**
    * @brief 键不存在异常
    *  
    *  该异常表示查找键不存在
    */
    class KeyNotFoundException: public BasicException<KeyNotFoundException, Exception>{};

    /**
    * @brief 键已存在异常
    *  
    *  该异常表示查找键已经存在
    */
    class KeyAlreadyExistException: public BasicException<KeyAlreadyExistException, Exception>{};

    /**
    * @brief 申请动态内存失败异常
    *  
    *  该异常表示动态内存申请失败
    */
    class BadAllocException: public BasicException<BadAllocException, Exception>{};

    /**
    * @brief 参数错误异常
    *  
    *  该异常表示调用函数的参数不合法。如果能使用其它更清楚表明参数不合法原因的异常的话，尽量避免使用本异常
    */
    class BadArgumentException: public BasicException<BadArgumentException, Exception>{};

    /**
    * @brief 空指针异常
    *  
    *  该异常表示在不能接受空指针的地方发现了空指针
    */
    class NullPointerException: public BasicException<NullPointerException, Exception>{};

    /**
    * @brief 错误的格式字符串异常
    *  
    *  该异常表示格式字符串不合法。
    */
    class BadFormatStringException: public BasicException<BadFormatStringException, Exception>{};

    /**
    * @brief 未初始化异常
    *  
    *  该异常表示使用了未被初始化的对象
    */
    class UninitializedException: public BasicException<UninitializedException, Exception>{};

    /**
    * @brief 未实现异常
    *  
    *  该异常表示调用了未被实现的API
    */
    class NotImplementedException: public BasicException<NotImplementedException, Exception>{};

    /**
    * @brief 无效操作异常
    *  
    *  该异常表示执行了无效的操作
    */
    class InvalidOperationException: public BasicException<InvalidOperationException, Exception>{};

    /**
    * @brief 上溢出异常
    *  
    *  该异常表示数值比能接受的最大值还要大
    */
    class OverflowException: public BasicException<OverflowException, Exception>{};

    /**
    * @brief 下溢出异常
    *  
    *  该异常表示数值比能接受的最小值还要小
    */
    class UnderflowException: public BasicException<UnderflowException, Exception>{};

    /**
    * @brief （语法）解析失败异常
    *  
    *  该异常表示语法解析失败
    */
    class ParseErrorException: public BasicException<ParseErrorException, Exception>{};

    /**
    * @brief IO异常
    *  
    */
    class IOException : public bsl::BasicException< IOException, bsl::Exception >{};

    /**
    * @brief 找不到文件异常
    *  
    *  
    */
    class FileNotFoundException : public bsl::BasicException< FileNotFoundException, IOException >{};

    /**
    * @brief 文件信息异常
    *  
    *  
    */
    class FstatException : public bsl::BasicException< FstatException, IOException >{};

    /**
    * @brief mmap异常
    *  
    *  
    */
    class MmapException : public bsl::BasicException< MmapException, IOException >{};

    /**
    * @brief 动态链接异常
    *  
    *  
    */
    class DynamicLinkingException : public bsl::BasicException< DynamicLinkingException, IOException >{};

    /**
    * @brief 断言失败异常
    *  
    *  
    */
    class AssertionFailedException : public bsl::BasicException< AssertionFailedException, bsl::Exception >{};


}//namespace bsl 
#endif  //__BSL_EXCEPTION_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
