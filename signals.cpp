#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num)
{
  // TODO: Add your implementation
  printf("smash: got ctrl-Z\n");
  SmallShell &smash = SmallShell::getInstance("smash");
  if (smash.current_command == nullptr)
  {
    return;
  }
  ExternalCommand &current_cmd = *(smash.current_command);
  if ((current_cmd.is_fg == true) && (JobEntry::is_alive(current_cmd.job.pid)))
  {
    DO_SYS(kill(current_cmd.job.pid,SIGSTOP));
    current_cmd.job.status = JobEntry::STOPPED;
    current_cmd.is_fg = false;
    smash.jobList.addJob(current_cmd.job);
    printf("smash: process %d was stopped\n",(int)current_cmd.job.pid);
  }
}

void ctrlCHandler(int sig_num)
{
  // TODO: Add your implementation
}

void alarmHandler(int sig_num)
{
  // TODO: Add your implementation
}
