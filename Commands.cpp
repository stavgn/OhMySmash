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

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell(std::string name)
{
  updateShellName(name);
}

SmallShell::~SmallShell()
{
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

  if (firstWord.compare("chprompt") == 0)
  {
    return new ChangePromptCommand(cmd_line, this);
  }
  else if (firstWord.compare("pwd") == 0)
  {
    return new GetCurrDirCommand();
  }

  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  // TODO: Add your implementation here
  // for example:
  Command *cmd = CreateCommand(cmd_line);
  if (cmd != nullptr)
    cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void SmallShell::updateShellName(std::string name)
{
  this->name = _trim(name) + "> ";
}

Command::Command(const char *cmd_line)
{
  numOfArgs = _parseCommandLine(cmd_line, args);
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

void GetCurrDirCommand::execute()
{
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL)
  {
    cout << cwd << "\n";
  }
}