#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <exception>
#include <string>
#include <stdio.h>
#include <unistd.h>

class Exception : public std::exception
{
    Exception() = default;
    Exception(std::string msg) : msg(msg) {}

protected:
    std::string msg;

public:
    virtual void handle() {}
};

class SysCallException : public Exception
{
public:
    void handle() override
    {
        msg = "smash error: " + msg + "failed";
        perror(msg.c_str());
    }
};

#endif //EXCEPTION_H_