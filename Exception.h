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
    virtual void handle();
}

class SysCallExcpetion : public Exception
{
    void handle() override
    {
        perror("smash error: " + msg + "failed");
    }
}

#endif //EXCEPTION_H_