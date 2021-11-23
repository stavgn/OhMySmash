#include "Commands.h"

using namespace std;

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
  return _rtrim(_ltrim(s));
}

void _split(const char *cmd_line, std::string term, std::string &str1, std::string &str2)
{
  string cmd_s = string(cmd_line);
  str1 = cmd_s.substr(0, cmd_s.find_first_of(term));
  str2 = cmd_s.substr(cmd_s.find_first_of(term) + 1);
}

int _parseCommandLine(const char *cmd_line, char **args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    memset(args[i], 0, s.length() + 1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

int is_piped_command(string cmd_s)
{
  return cmd_s.find("|") != std::string::npos || cmd_s.find("|&") != std::string::npos;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell(std::string name)
{
  updateShellName(name);
}

SmallShell::~SmallShell()
{
  free(old_pwd);
  // TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line)
{
  // For example:
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (is_piped_command(cmd_s))
  {
    return new PipedCommands(cmd_line, this);
  }
  else if (firstWord.compare("chprompt") == 0)
  {
    return new ChangePromptCommand(cmd_line, this);
  }
  else if (firstWord.compare("pwd") == 0)
  {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0)
  {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0)
  {
    return new ChangeDirCommand(cmd_line, &old_pwd);
  }
  else if (firstWord.compare("head") == 0)
  {
    return new HeadCommand(cmd_line);
  }

  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  // TODO: Add your implementation here
  // for example:
  Command *cmd = CreateCommand(cmd_line);
  SmallShell::exec_util(cmd);
}

void SmallShell::updateShellName(std::string name)
{
  this->name = _trim(name) + "> ";
}

Command::Command(const char *cmd_line)
{
  numOfArgs = _parseCommandLine(cmd_line, args);
  IOConfig = IOFactory::getIO(args, numOfArgs);
}

void Command::prepare()
{
  if (IOConfig != nullptr)
  {
    IOConfig->config();
  }
}

void Command::cleanup()
{
  if (IOConfig != nullptr)
  {
    IOConfig->revert();
  }
}

Command::~Command()
{
  for (int i = 0; i < numOfArgs - 1; i++)
  {
    delete args[i];
  }
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line)
{
}

ChangePromptCommand::ChangePromptCommand(const char *cmd_line, SmallShell *shell) : BuiltInCommand(cmd_line)
{
  this->shell = shell;
}

void ChangePromptCommand::execute()
{
  if (numOfArgs <= 1)
  {
    this->shell->updateShellName("smash");
    return;
  }
  this->shell->updateShellName(args[1]);
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
{
}

void GetCurrDirCommand::execute()
{
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL)
  {
    cout << cwd << "\n";
  }
  else
  {
    throw(SysCallException(std::string("getcwd")));
  }
}

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
{
}

void ShowPidCommand::execute()
{
  printf("smash pid is %d\n", getpid());
}
ChangeDirCommand::ChangeDirCommand(const char *cmd_line, char **last_pwd) : BuiltInCommand(cmd_line)
{
  old_pwd = last_pwd;
}

void ChangeDirCommand::execute()
{
  if (numOfArgs == 1)
  {
    return;
  }
  if (numOfArgs > 2)
  {
    throw(Exception("cd: too many arguments"));
  }
  std::string arg(args[1]);
  std::string new_path;
  if (arg.compare("-") == 0)
  {
    if (*old_pwd == nullptr)
    {
      throw(Exception("cd: OLDPWD not set"));
    }
    else
    {
      new_path = string(*old_pwd);
    }
  }
  else
  {
    new_path = arg;
  }

  if (*old_pwd == nullptr) // first use of cd
  {
    *old_pwd = (char *)malloc(PATH_MAX);
  }

  if (getcwd(*old_pwd, PATH_MAX) == NULL) // update old_pwd
  {
    throw(SysCallException(std::string("getcwd")));
  }

  if (chdir(new_path.c_str()) < 0)
  {
    throw(SysCallException("chdir"));
  }
}

HeadCommand::HeadCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
{
}

void HeadCommand::execute()
{
  int path_arg = 1;
  if (numOfArgs < 2)
  {
    throw(Exception("head: not enough argmuments"));
  }
  if ((numOfArgs >= 3) && (args[1][0] == '-'))
  {
    N = stoi(string(args[1] + 1)); // Takes -N and, cuts the '-' and convert to int
    path_arg = 2;
  }
  fstream file;
  file.open(args[path_arg], fstream::in);
  if (file.fail() == 1)
  {
    throw(SysCallException("open"));
  }
  string line;

  for (int i = 0; (i < N) && (!file.eof()); i++)
  {
    getline(file, line);
    if (file.bad() == 1)
    {
      throw(SysCallException("read"));
    }
    if (file.eof())
    {
      cout << line << flush;
    }
    else
    {
      cout << line << endl;
    }
    if (cout.bad())
    {
      throw(SysCallException("write"));
    }
  }
  file.close();
}

WriteToFile::WriteToFile(std::string filename) : IO()
{
  this->filename = filename;
};

void CreateOrOverWriteToFile::config()
{
  int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
  int stdout = dup(1);
  close(1);
  dup(fd);
  this->fd = fd;
  this->stdout = stdout;
}

void CreateOrAppendToFile::config()
{
  int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
  lseek(fd, 0, SEEK_END);
  int stdout = dup(1);
  close(1);
  dup(fd);
  this->fd = fd;
  this->stdout = stdout;
}

void WriteToFile::revert()
{
  close(1);
  dup(stdout);
  close(stdout);
  close(fd);
  reverted = 1;
}

PipedCommands::PipedCommands(const char *cmd_line, SmallShell *shell) : Command(cmd_line)
{
  std::string cmd_line1;
  std::string cmd_line2;
  Pipe::PipeType type = Pipe::getPipeType(string(cmd_line));

  switch (type)
  {
  case Pipe::STREAM_STDOUT:
    _split(cmd_line, "|", cmd_line1, cmd_line2);
    break;
  case Pipe::STREAM_STDERR:
    _split(cmd_line, "|&", cmd_line1, cmd_line2);
  default:
    break;
  }

  IOConfig = IOFactory::getPipe(type);
  cmd1 = shell->CreateCommand(cmd_line1.c_str());
  cmd2 = shell->CreateCommand(cmd_line2.c_str());
}

PipedCommands::~PipedCommands()
{
  delete cmd1;
  delete cmd2;
}

void PipedCommands::execute()
{
  Pipe *PipeIO = dynamic_cast<Pipe *>(IOConfig);
  if (PipeIO->is_father)
  {
    SmallShell::exec_util(cmd1);
    wait(NULL);
  }
  else
  {
    SmallShell::exec_util(cmd2);
    exit(0);
  }
}

Pipe::Pipe(PipeType type) : type(type)
{
  pipe(my_pipe);
}

void Pipe::config()
{
  is_father = fork() != 0;
  if (is_father)
  {
    int target_fd = type == Pipe::STREAM_STDOUT ? 1 : 2;
    std_target = dup(type);
    dup2(my_pipe[1], target_fd);
    close(my_pipe[0]);
    close(my_pipe[1]);
  }
  else
  {
    setpgrp();
    dup2(my_pipe[0], 0);
    close(my_pipe[0]);
    close(my_pipe[1]);
  }
}

void Pipe::revert()
{
  if (is_father)
  {
    close(type);
    dup(std_target);
    close(std_target);
  }
  reverted = 1;
}

WriteToFile::~WriteToFile()
{
  if (!reverted)
    revert();
}
Pipe::~Pipe()
{
  if (!reverted)
    revert();
}

void JobsList::addJob(JobEntry job)
{
  removeFinishedJobs();
  if (jobsList.empty())
  {
    job.jid = 1;
  }
  else
  {
    JobEntry max_job = jobsList.rbegin()->second;
    job.jid = max_job.jid + 1;
  }
  job.time = time(NULL);
  jobsList[job.jid] = job;
}

void JobsList::printJobsList()
{
  removeFinishedJobs();
  for (auto i = jobsList.cbegin(); i != jobsList.cend(); ++i)
  {
    JobEntry cur_job = i->second;
    cout << "[" << cur_job.jid << "] ";
    cout << cur_job.cmd << " : ";
    cout << cur_job.pid << " ";
    cout << difftime(time(NULL), cur_job.time);
    if (cur_job.status == JobEntry::STOPPED)
    {
      cout << " (stopped)";
    }
    cout << endl;
  }
}

ExternalCommand::ExternalCommand(const char *cmd_line, SmallShell *shell) : Command(cmd_line)
{
  this->shell = shell;
  this->cmd_line = cmd_line;
  if (_isBackgroundComamnd(cmd_line))
  {
    is_fg = false;
    job.status = JobEntry::BACKGROUND;
  }
  else
  {
    is_fg = true;
    job.status = JobEntry::FOREGROUND;
  }

  pid_t pid = fork();
  if (pid < 0)
  {
    throw SysCallException("fork");
  }
  else if (pid == 0)
  {
    is_father = false;
  }
  else
  {
    is_father = true;
    job.pid = pid;
  }
}

void ExternalCommand::execute()
{
  if (is_father)
  {
    if (is_fg)
    {
      waitpid(job.pid, NULL);
    }
    else
    {
      shell->jobList.addJob(job);
    }
    return;
  }
  //// need to be fixed!!

  // in case of child
  string exe_args = "-c \"" + string(cmd_line);
  execv("/bin/bash");
}

JobEntry *JobsList::getLastStoppedJob()
{
  return &jobsList[last_jit_stopped];
}
JobEntry *JobsList::getLastJob()
{
  auto it = jobsList.rbegin();
  return &it->second;
}

void JobsList::removeJobById(int jobId)
{
  jobsList.erase(jobId);
}

JobEntry *JobsList::getJobById(int jobId)
{
  return &jobsList[jobId];
}

void JobsList::removeFinishedJobs()
{
  std::stack<int> to_be_deleted;
  std::map<int, JobEntry>::iterator it;
  for (it = jobsList.begin(); it != jobsList.end(); ++it)
  {
    if (it->second.status == JobEntry::STOPPED)
    {
      to_be_deleted.push(it->second.jid);
    }
  }

  while (!to_be_deleted.empty())
  {
    int jip = to_be_deleted.top();
    jobsList.erase(jip);
    to_be_deleted.pop();
  }
}

void JobsList::killAllJobs()
{
  std::stack<int> to_be_killed;
  std::map<int, JobEntry>::iterator it;
  for (it = jobsList.begin(); it != jobsList.end(); ++it)
  {
    kill(it->second.pid, 9);
  }
}