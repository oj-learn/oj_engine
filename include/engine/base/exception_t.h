#pragma once

#include "iguana/iguana/msgpack.hpp"
#include <cxxabi.h>
#include <exception>
#include <string>


/*---------------------------------------------------------------------------------
sig_t
---------------------------------------------------------------------------------*/
struct sig_ex_t {
    //-----------------------------------------------------------------------------
    //注册段错误的处理函数
    //-----------------------------------------------------------------------------
    static void installSegvHandler();
    //-----------------------------------------------------------------------------
    static void fillStackTrace(std::string& stackOut);
    //-----------------------------------------------------------------------------
    static std::string demangle(const char* symbol);
};


/*---------------------------------------------------------------------------------
只是打印try cach 中的信息，示例
  try
  {
    fun();
  }
  catch (const exception_t& ex)
  {
    printf("reason: %s\n", ex.what());
    printf("stack trace: %s\n", ex.stackTrace());
  }
---------------------------------------------------------------------------------*/
class exception_t : public std::exception {
public:
    //-----------------------------------------------------------------------------
    std::string message_;  //异常信息
    std::string stack_;    //栈trace
    int16_t     error_code_;
    std::string error_message_;

public:
    //-----------------------------------------------------------------------------
    exception_t() noexcept {}
    //-----------------------------------------------------------------------------
    exception_t(int16_t hc, std::string what);
    //-----------------------------------------------------------------------------
    explicit exception_t(const char* what);
    //-----------------------------------------------------------------------------
    explicit exception_t(const std::string& what);
    //-----------------------------------------------------------------------------
    virtual ~exception_t() throw();
    //-----------------------------------------------------------------------------
    virtual const char* what() const throw();
    //-----------------------------------------------------------------------------
    const char* stackTrace() const throw();
    //-----------------------------------------------------------------------------
};

REFLECTION(exception_t, error_code_, error_message_);


// template <typename t>
// std::string tname()
// {
//     auto tn        = typeid(t).name();
//     char buff[256] = { 0 };
//     auto t1        = abi::__cxa_demangle(tn, buff, sizeof(buff), nullptr);
// }
