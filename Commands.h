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
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define WHITESPACE " \t\n\r\f\v"

class Command
{

public:
  char *args[21];
  int numOfArgs;
  Command() = default;
  Command(const char *cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
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
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
public:
  ShowPidCommand() {}
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

class JobsList
{
public:
  class JobEntry
  {
    // TODO: Add your data members
  };
  // TODO: Add your data members
public:
  JobsList();
  ~JobsList();
  void addJob(Command *cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry *getLastJob(int *lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
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
