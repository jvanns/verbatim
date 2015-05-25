/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_UTILITY_EXCEPTION_HPP
#define VERBATIM_UTILITY_EXCEPTION_HPP

// libstdc++
#include <string>
#include <exception>

// libc
#include <cstdarg> // For va_list et al

namespace verbatim {
namespace utility {

class Exception : public std::exception
{
    public:
        /* Member functions/methods */
        Exception(const Exception &e);
        Exception& operator= (const Exception &e);

        const char* what() const throw() { return message.c_str(); }
    protected:
        /* Member variables/attributes */
        int error; // Copy of errno
        va_list args; // The argument list
        std::string message; // The resulting error message
        std::string (*errno2str)(int error); // strerror() etc.

        /* Member functions/methods */
        Exception();
        virtual ~Exception() throw() {}

        void init(const std::string &from, int error, const char *format);
};

// Exceptions
struct LockError : public Exception
{
    LockError(const std::string &from, int error, const char *format, ...);
};

struct FileError : public Exception
{
    FileError(const std::string &from, int error, const char *format, ...);
};

struct LogicError : public Exception
{
    LogicError(const std::string &from, int error, const char *format, ...);
};

struct ThreadError : public Exception
{
    ThreadError(const std::string &from, int error, const char *format, ...);
};

struct SignalError : public Exception
{
    SignalError(const std::string &from, int error, const char *format, ...);
};

struct ProcessError : public Exception
{
    ProcessError(const std::string &from, int error, const char *format, ...);
};

struct ValueError : public Exception
{
    ValueError(const std::string &from, int error, const char *format,...);
};

struct StreamError : public Exception
{
    StreamError(const std::string &from, int error, const char *format,...);
};

struct ConversionError : public Exception
{
    ConversionError(const std::string &from, int error, const char *format,...);
};

} // utility
} // verbatim

#endif

