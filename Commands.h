#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <string>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <limits.h>
#include <fcntl.h>
#include "Exception.h"
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define WHITESPACE " \t\n\r\f\v"

#define DO_SYS(syscall)                 \
  do                                    \
  {                                     \
    /* safely invoke a system call */   \
    if ((syscall) == -1)                \
    {                                   \
      throw SysCallException(#syscall); \
    }                                   \
  } while (0)

class IO
{
public:
  IO() = default;
  ~IO() = default;
  virtual void config() = 0;
  virtual void revert() = 0;
};

class WriteToFile : public IO
{
protected:
  std::string filename;
  int fd;
  int stdout;
  WriteToFile(std::string filename);
  ~WriteToFile() = default;
  void revert() override;
};

class CreateOrOverWriteToFile : public WriteToFile
{
public:
  CreateOrOverWriteToFile(std::string filename) : WriteToFile(filename) {}
  ~CreateOrOverWriteToFile() = default;
  void config() override;
};

class CreateOrAppendToFile : public WriteToFile
{
public:
  CreateOrAppendToFile(std::string filename) : WriteToFile(filename) {}
  ~CreateOrAppendToFile() = default;
  void config() override;
};

class AppendToFile : public IO
{
};
class IOFactory
{
  // TODO: Add your data members
public:
  IOFactory();
  ~IOFactory();
  static IO *getIO(char **args, int numOfArgs)
  {
    if (std::string(args[numOfArgs - 2]) == ">")
    {
      return new CreateOrOverWriteToFile(args[numOfArgs - 1]);
    }
    else if (std::string(args[numOfArgs - 2]) == ">>")
    {
      return new CreateOrAppendToFile(args[numOfArgs - 1]);
    }

    return nullptr;
  }
};

class Command
{

public:
  char *args[COMMAND_MAX_ARGS];
  int numOfArgs;
  IO *IOConfig;
  Command() = default;
  Command(const char *cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  virtual void prepare();
  virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command
{
public:
  BuiltInCommand() = default;
  BuiltInCommand(const char *cmd_line);
  virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command
{
public:
  ExternalCommand(const char *cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command
{
  // TODO: Add your data members
public:
  PipeCommand(const char *cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command
{
  // TODO: Add your data members
public:
  explicit RedirectionCommand(const char *cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand
{
  // TODO: Add your data members public:
  ChangeDirCommand(const char *cmd_line, char **plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand
{
public:
  GetCurrDirCommand(const char *cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
public:
  ShowPidCommand(const char *cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand
{
  // TODO: Add your data members public:
  QuitCommand(const char *cmd_line, JobsList *jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class JobsCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  JobsCommand(const char *cmd_line, JobsList *jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  KillCommand(const char *cmd_line, JobsList *jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  ForegroundCommand(const char *cmd_line, JobsList *jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  BackgroundCommand(const char *cmd_line, JobsList *jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class HeadCommand : public BuiltInCommand
{
public:
  HeadCommand(const char *cmd_line);
  virtual ~HeadCommand() {}
  void execute() override;
};

class SmallShell
{
private:
  SmallShell();

public:
  std::string name;
  void updateShellName(std::string name);
  Command *CreateCommand(const char *cmd_line);
  SmallShell(std::string name);
  SmallShell(SmallShell const &, std::string name) = delete; // disable copy ctor
  void operator=(SmallShell const &) = delete;               // disable = operator
  static SmallShell &getInstance(std::string name)           // make SmallShell singleton
  {
    static SmallShell instance = SmallShell(name); // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char *cmd_line);
  // TODO: add extra methods as needed
};

class ChangePromptCommand : public BuiltInCommand
{
public:
  SmallShell *shell = nullptr;
  virtual ~ChangePromptCommand(){};
  ChangePromptCommand(const char *cmd_line, SmallShell *shell);
  void execute() override;
};

#endif //SMASH_COMMAND_H_
