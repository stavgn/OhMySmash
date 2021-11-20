#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <exception>
#include <string>
#include <stdio.h>
#include <unistd.h>

class Exception : public std::exception
{

protected:
    std::string err_msg;

public:
    Exception() = default;
    Exception(std::string msg) : err_msg(msg) {}
    virtual void handle() 
    {
        std::string msg = "smash error: " + err_msg;
        std::cerr << msg.c_str() << '\n';
    }
};

class SysCallException : public Exception
{
public:
    SysCallException(std::string msg) : Exception(msg) {}
    void handle() override
    {
        std::string msg = "smash error: " + err_msg + " failed";
        perror(msg.c_str());
    }
};

#endif //EXCEPTION_H_