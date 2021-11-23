#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <string>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <limits.h>
#include <fcntl.h>
#include <map>
#include <stack>
#include <signal.h>
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
  int reverted = 0;
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
  ~WriteToFile();
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

class Pipe : public IO
{
public:
  enum PipeType
  {
    NO_PIPE_FOUND = 0,
    STREAM_STDOUT = 1,
    STREAM_STDERR = 2
  };
  PipeType type;
  static Pipe::PipeType getPipeType(std::string cmd_s)
  {
    if (cmd_s.find("|&") != std::string::npos)
    {
      return Pipe::STREAM_STDERR;
    }
    else if (cmd_s.find("|") != std::string::npos)
    {
      return Pipe::STREAM_STDOUT;
    }
    return Pipe::NO_PIPE_FOUND;
  }
  int my_pipe[2];
  int std_target;
  int is_father;
  Pipe(PipeType type);
  ~Pipe();
  void config() override;
  void revert() override;
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
  static IO *getPipe(Pipe::PipeType type)
  {
    return new Pipe(type);
  }
};

class Command
{

public:
  char *args[COMMAND_MAX_ARGS];
  int numOfArgs;
  IO *IOConfig = nullptr;
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

class ChangeDirCommand : public BuiltInCommand
{
  // TODO: Add your data members public:
public:
  char **old_pwd = nullptr;
  ChangeDirCommand(const char *cmd_line, char **last_pwd);
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

class JobEntry
{
public:
  enum JobStatus
  {
    STOPPED = 0,
    FOREGROUND = 1,
    BACKGROUND = 2
  };
  int jid;
  pid_t pid;
  Command *cmd;
  JobStatus status;
  time_t time;

  static bool is_alive(pid_t pid);
  bool operator<(const JobEntry &job2)
  {
    return this->jid < job2.jid;
  }
};

class JobsList
{
public:
  std::map<int, JobEntry> jobsList;
  int last_jit_stopped;
  JobsList() = default;
  ~JobsList() = default;
  void addJob(JobEntry job);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry *getLastJob();
  JobEntry *getLastStoppedJob();
  // TODO: Add extra methods or modify exisitng ones as needed
};
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
  int N = 10;
  HeadCommand(const char *cmd_line);
  virtual ~HeadCommand() {}
  void execute() override;
};

class ExternalCommand;

class SmallShell
{
private:
  SmallShell();

public:
  std::string name;
  char *old_pwd = nullptr;
  JobsList jobList;
  ExternalCommand *current_command = nullptr;
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
  static void exec_util(Command *cmd)
  {
    if (cmd != nullptr)
    {
      cmd->prepare();
      cmd->execute();
      cmd->cleanup();
    }
  }
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

class PipedCommands : public Command
{
  Command *cmd1 = nullptr;
  Command *cmd2 = nullptr;

public:
  PipedCommands(const char *cmd_line, SmallShell *shell);
  ~PipedCommands();
  void execute() override;
};


class ExternalCommand : public Command
{
public:
  JobEntry job;
  SmallShell *shell;
  const char *cmd_line;
  bool is_father;
  bool is_fg;
  ExternalCommand(const char *cmd_line, SmallShell *shell);
  virtual ~ExternalCommand() {}
  void execute() override;
};


#endif //SMASH_COMMAND_H_
