/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Exception.hpp"

// libc
#include <stdio.h> // For vsnprintf
#include <string.h> // For strerror
#include <stdint.h> // For uint16_t

// libstdc++
using std::string;

namespace {

inline
string
default_errno2str(const int error)
{
    return error == 0 ? "" : strerror(error);
}

} // anonymous

namespace verbatim {
namespace utility {

Exception::Exception() : error(0),
                         message("Exception thrown from "),
                         errno2str(default_errno2str)
{
}

Exception::Exception(const Exception &e)
{
    *this = e;
}

Exception&
Exception::operator= (const Exception &e)
{
    if (this != &e) {
        error = e.error;
        message = e.message;
    }

    return *this;
}

void
Exception::init(const string &from, int e, const char *format)
{
    static const uint16_t buffer_size = 1024;
    char buffer[buffer_size];

    memset(buffer, '\0', buffer_size);
    vsnprintf(buffer, buffer_size, format, args);
    va_end(args);

    message += (from + ":\n** " + buffer);

    if ((error = e) && errno2str) {
        const string s(errno2str(e));

        if (!s.empty())
            message += "\n** External 'strerror' function suggests: " + s;
    }
}

// Exceptions
LockError::LockError(const string &from, int error, const char *format, ...)
{
    va_start(args, format);
    init(from, error, format); // va_end happens in here
}

FileError::FileError(const string &from, int error, const char *format, ...)
{
    va_start(args, format);
    init(from, error, format); // va_end happens in here
}

LogicError::LogicError(const string &from, int error, const char *format, ...)
{
    va_start(args, format);
    init(from, error, format); // va_end happens in here
}

ThreadError::ThreadError(const string &from, int error, const char *format, ...)
{
    va_start(args, format);
    init(from, error, format); // va_end happens in here
}

SignalError::SignalError(const string &from, int error, const char *format, ...)
{
    va_start(args, format);
    init(from, error, format); // va_end happens in here
}

ProcessError::ProcessError(const string &from,
                           int error,
                           const char *format,
                           ...)
{
    va_start(args, format);
    init(from, error, format); // va_end happens in here
}

ValueError::ValueError(const string &from,
                       int error,
                       const char *format,
                       ...)
{
    va_start(args, format);
    init(from, error, format); // va_end happens in here
}

StreamError::StreamError(const string &from,
                         int error,
                         const char *format,
                         ...)
{
    va_start(args, format);
    init(from, error, format); // va_end happens in here
}

ConversionError::ConversionError(const string &from,
                                 int error,
                                 const char *format,
                                 ...)
{
    va_start(args, format);
    init(from, error, format); // va_end happens in here
}

} // utility
} // verbatim
