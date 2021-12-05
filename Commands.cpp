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

void _split(const char *cmd_line, std::string term, std::string &str1, std::string &str2)
{
  std::string cmd_s = std::string(cmd_line);
  str1 = cmd_s.substr(0, cmd_s.find_first_of(term));
  str2 = cmd_s.substr(cmd_s.find_first_of(term) + 1);
};

bool _isBackgroundCommand(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

string _removeBackgroundSign(const char *cmd_line)
{
  string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return str;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return str;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  str[idx] = ' ';
  // truncate the command line string up to the last non-space character
  str[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
  return str;
}

int is_piped_command(string cmd_s)
{
  return cmd_s.find("|") != std::string::npos || cmd_s.find("|&") != std::string::npos;
}

bool _is_a_number(const char *c, int offset)
{
  std::string s = string(c + offset);
  try
  {
    stoi(s);
  }
  catch (...)
  {
    return false;
  }
  return true;
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
  else if (firstWord.compare("jobs") == 0)
  {
    return new JobsCommand(cmd_line, &jobList);
  }
  else if (cmd_s.length() == 0)
  {
    return nullptr;
  }
  else if (firstWord.compare("kill") == 0)
  {
    return new KillCommand(cmd_line, &jobList);
  }
  else if (firstWord.compare("fg") == 0)
  {
    return new ForegroundCommand(cmd_line, &jobList, this);
  }
  else if (firstWord.compare("bg") == 0)
  {
    return new BackgroundCommand(cmd_line, &jobList);
  }
  else if (firstWord.compare("quit") == 0)
  {
    return new QuitCommand(cmd_line, &jobList);
  }
  else if (firstWord.compare("timeout") == 0)
  {
    return new TimedCommand(cmd_line, this);
  }
  else
  {
    return new ExternalCommand(cmd_line, this);
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

Command::Command(const char *cmd_line) : cmd_line(cmd_line)
{
  numOfArgs = _parseCommandLine(cmd_line, args);
  IOConfig = IOFactory::getIO(args, &numOfArgs, cmd_line);
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

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(_removeBackgroundSign(cmd_line).c_str())
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
  char temp_pwd[PATH_MAX];
  if (getcwd(temp_pwd, PATH_MAX) == NULL) // update old_pwd
  {
    throw(SysCallException(std::string("getcwd")));
  }

  if (chdir(new_path.c_str()) < 0)
  {
    throw(SysCallException("chdir"));
  }
  strcpy(*old_pwd, temp_pwd);
}

HeadCommand::HeadCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
{
}

void HeadCommand::execute()
{
  int path_arg = 1;
  if (numOfArgs < 2)
  {
    throw(Exception("head: not enough arguments"));
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

void JobsCommand::execute()
{
  jobList->printJobsList();
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

PipedCommands::PipedCommands(const char *cmd_line, SmallShell *shell) : Command(cmd_line), shell(shell)
{
  std::string cmd_line1;
  std::string cmd_line2;
  Pipe::PipeType type = Pipe::getPipeType(string(cmd_line));
  Pipe *PipeIO = dynamic_cast<Pipe *>(IOConfig);
  PipeIO->loadShell(shell);

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

void Pipe::loadShell(SmallShell *shell)
{
  this->shell = shell;
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
    shell->isMaster = false;
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
  else if (job.jid == 0)
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
    cout << cur_job.cmd_line << " : ";
    cout << cur_job.pid << " ";
    cout << difftime(time(NULL), cur_job.time) << " secs";
    if (cur_job.status == JobEntry::STOPPED)
    {
      cout << " (stopped)";
    }
    cout << endl;
  }
}

void JobsList::printJobsList2()
{
  for (auto i = jobsList.cbegin(); i != jobsList.cend(); ++i)
  {
    JobEntry cur_job = i->second;
    cout << cur_job.jid << ": ";
    cout << cur_job.cmd_line;
    cout << endl;
  }
}

ExternalCommand::ExternalCommand(const char *cmd_line, SmallShell *shell) : Command(cmd_line)
{
  this->shell = shell;
  this->cmd_line = cmd_line;
  job.cmd_line = string(cmd_line);
  if (_isBackgroundCommand(cmd_line))
  {
    is_fg = false;
    job.status = JobEntry::BACKGROUND;
  }
  else
  {
    is_fg = true;
    job.status = JobEntry::FOREGROUND;
  }
}

void ExternalCommand::execute()
{
  if (shell->isMaster)
  {
    pid_t pid = fork();
    if (pid < 0)
    {
      throw SysCallException("fork");
    }
    else if (pid == 0)
    {
      setpgrp();
      is_father = false;
    }
    else
    {
      is_father = true;
      job.pid = pid;
    }
    if (is_father)
    {
      if (is_fg)
      {
        shell->current_fg_job = job;
        waitpid(job.pid, NULL, WSTOPPED);
      }
      else
      {
        shell->jobList.addJob(job);
      }
      return;
    }
  }

  // in case of child
  string bash_pth("/bin/bash");
  string bash_flag("-c");
  string execline = _removeBackgroundSign(cmd_line);
  string args = bash_flag + execline;
  execl(bash_pth.c_str(), bash_pth.c_str(), bash_flag.c_str(), execline.c_str(), (char *)NULL);
  throw SysCallException("exec");
}

JobEntry *JobsList::getLastStoppedJob()
{
  auto it = jobsList.rbegin();
  for (; it != jobsList.rend(); it++)
  {
    if (it->second.status == JobEntry::STOPPED)
    {
      return &(it->second);
    }
  }
  return nullptr;
}
JobEntry *JobsList::getLastJob()
{
  auto it = jobsList.rbegin();
  return jobsList.empty() ? nullptr : &it->second;
}

void JobsList::removeJobById(int jobId)
{
  jobsList.erase(jobId);
}

JobEntry *JobsList::getJobById(int jobId)
{
  removeFinishedJobs();
  auto it = jobsList.find(jobId);

  return it == jobsList.end() ? nullptr : &jobsList[jobId];
}

bool JobEntry::is_alive(int ProcessId)
{
  // Wait for child process, this should clean up defunct processes
  waitpid(ProcessId, nullptr, WNOHANG);
  // kill failed let's see why..
  if (kill(ProcessId, 0) == -1)
  {
    // First of all kill may fail with EPERM if we run as a different user and we have no access, so let's make sure the errno is ESRCH (Process not found!)
    if (errno != ESRCH)
    {
      return true;
    }
    return false;
  }
  // If kill didn't fail the process is still running
  return true;
}
bool JobEntry::is_stopped(pid_t pid, JobEntry::JobStatus status)
{
  int wait_status;
  if (!JobEntry::is_alive(pid))
  {
    return false;
  }
  pid_t result = waitpid(pid, &wait_status, WUNTRACED | WNOHANG | WCONTINUED);
  if (result == -1)
  {
    throw SysCallException("waitpid");
  }

  if (((status == JobEntry::STOPPED) && !WIFCONTINUED(wait_status)) || WIFSTOPPED(wait_status))
  {
    return true;
  }

  return false;
}

void JobsList::removeFinishedJobs()
{
  std::stack<int> to_be_deleted;
  std::map<int, JobEntry>::iterator it;
  for (it = jobsList.begin(); it != jobsList.end(); ++it)
  {
    if (!JobEntry::is_alive(it->second.pid))
    {

      to_be_deleted.push(it->second.jid);
    }
    if (JobEntry::is_stopped(it->second.pid, it->second.status))
    {
      it->second.status = JobEntry::STOPPED;
    }
    else
    {
      it->second.status = JobEntry::BACKGROUND;
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

KillCommand::KillCommand(const char *cmd_line, JobsList *jobsList) : BuiltInCommand(cmd_line)
{
  this->jobsList = jobsList;
}

bool KillCommand::validate()
{
  if (numOfArgs != 3 || (!_is_a_number(args[2], 0)))
  {
    return false;
  }

  std::string arg2 = string(args[1]);
  if (arg2[0] == '-' && _is_a_number(args[1], 1))
  {
    int signum = stoi(string(args[1]).substr(1).c_str());
    return signum >= 1 && signum <= 31;
  }
  return false;
}

void KillCommand::execute()
{
  if (!validate())
  {
    throw Exception("kill: invalid arguments");
  }
  JobEntry *target_job = jobsList->getJobById(stoi(args[2]));
  if (target_job == nullptr)
  {
    throw Exception("kill: job-id " + string(args[2]) + " does not exist");
  }
  int signum = stoi(string(args[1]).substr(1).c_str());
  DO_SYS(kill(target_job->pid, signum));
  cout << "signal number " << signum << " was sent to pid " << target_job->pid << endl;
}

ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobsList, SmallShell *shell) : BuiltInCommand(cmd_line)
{
  this->shell = shell;
  this->jobsList = jobsList;
}

bool ForegroundCommand::validate()
{
  return numOfArgs == 1 || (numOfArgs == 2 && _is_a_number(args[1], 0));
}

void ForegroundCommand::execute()
{
  if (!validate())
  {
    throw Exception("fg: invalid arguments");
  }
  JobEntry *target_job;
  if (numOfArgs == 2)
  {
    target_job = jobsList->getJobById(stoi(string(args[1])));
    if (target_job == nullptr)
    {
      throw Exception("fg: job-id " + string(args[1]) + " does not exist");
    }
  }
  else
  {
    target_job = jobsList->getLastJob();
    if (target_job == nullptr)
    {
      throw Exception("fg: jobs list is empty");
    }
  }

  cout << target_job->cmd_line << " : " << target_job->pid << endl;
  jobsList->removeJobById(target_job->jid);
  DO_SYS(kill(target_job->pid, SIGCONT));
  target_job->status = JobEntry::FOREGROUND;
  shell->current_fg_job = *target_job;
  waitpid(target_job->pid, NULL, WSTOPPED);
}

BackgroundCommand::BackgroundCommand(const char *cmd_line, JobsList *jobsList) : BuiltInCommand(cmd_line)
{
  this->jobsList = jobsList;
}

bool BackgroundCommand::validate()
{
  return numOfArgs == 1 || (numOfArgs == 2 && _is_a_number(args[1], 0));
}

void BackgroundCommand::execute()
{
  if (!validate())
  {
    throw Exception("bg: invalid arguments");
  }
  JobEntry *target_job;
  if (numOfArgs == 2)
  {
    target_job = jobsList->getJobById(stoi(string(args[1])));
    if (target_job == nullptr)
    {
      throw Exception("bg: job-id " + string(args[1]) + " does not exist");
    }
    else if (target_job->status != JobEntry::STOPPED)
    {
      throw Exception("bg: job-id " + string(args[1]) + " is already running in the background");
    }
  }
  else
  {
    target_job = jobsList->getLastStoppedJob();
    if (target_job == nullptr)
    {
      throw Exception("bg: there is no stopped jobs to resume");
    }
  }

  cout << target_job->cmd_line << " : " << target_job->pid << endl;
  DO_SYS(kill(target_job->pid, SIGCONT));
  target_job->status = JobEntry::BACKGROUND;
}

QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobsList) : BuiltInCommand(cmd_line)
{
  this->jobsList = jobsList;
}

void QuitCommand::execute()
{
  if (numOfArgs == 2 and string(args[1]) == "kill")
  {
    jobsList->removeFinishedJobs();
    cout << "smash: sending SIGKILL signal to " << jobsList->jobsList.size() << " jobs:" << endl;
    jobsList->printJobsList2();
    jobsList->killAllJobs();
  }
  exit(0);
}

TimedJobEntry::TimedJobEntry(unsigned int time_left, ExternalCommand *command) : alarm_time(time_left), eCommand(command)
{
  insert_time = time(NULL);
}

double TimedJobEntry::time_left() const
{
  return alarm_time - difftime(time(NULL), insert_time);
}

void TimedJobs::addJob(unsigned int timeKey, TimedJobEntry &job)
{
  timedList.push(job);
}

const TimedJobEntry &TimedJobs::getFirstJob()
{
  return timedList.top();
}
void TimedJobs::removFirstJob()
{
  timedList.pop();
}

bool TimedJobs::empty()
{
  return timedList.empty();
}

TimedCommand::TimedCommand(const char *cmd_line, SmallShell *shell) : Command(cmd_line)
{
  if (numOfArgs < 3)
  {
    throw(Exception("timeout: not enough arguments"));
  }
  if (!_is_a_number(args[1], 0))
  {
    throw(Exception("timeout: invalid arguments"));
  }
  this->shell = shell;
  this->timeJobs = &shell->timedJobList;
}

void TimedCommand::execute()
{
  string external_cmd_line("");
  for (int i = 2; i < numOfArgs; i++)
  {
    external_cmd_line += " " + string(args[i]);
  }
  ExternalCommand *commnad = new ExternalCommand(external_cmd_line.c_str(), shell);
  commnad->job.cmd_line = string(cmd_line);
  TimedJobEntry *job = new TimedJobEntry(stoi(args[1]), commnad);
  if (timeJobs->empty() || (job->time_left() < timeJobs->getFirstJob().time_left()))
  {
    alarm(job->alarm_time);
  }
  timeJobs->addJob(job->alarm_time, *job);
  commnad->execute();
}

bool TimedJobEntry::operator<(const TimedJobEntry &job2) const
{
  return (time_left()) > (job2.time_left());
}
