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
  if ((smash.current_fg_job == nullptr) || !JobEntry::is_alive(smash.current_fg_job->pid))
  {
    cout << smash.name << flush;
    return;
  }
  JobEntry &current_fg_job = *(smash.current_fg_job);
  DO_SYS(kill(current_fg_job.pid, SIGSTOP));
  current_fg_job.status = JobEntry::STOPPED;
  smash.jobList.addJob(current_fg_job);
  printf("smash: process %d was stopped\n", (int)current_fg_job.pid);
  smash.current_fg_job = nullptr;
}

void ctrlCHandler(int sig_num)
{
  printf("smash: got ctrl-C\n");
  SmallShell &smash = SmallShell::getInstance("smash");
  if ((smash.current_fg_job == nullptr) || !JobEntry::is_alive(smash.current_fg_job->pid))
  {
    cout << smash.name << flush;
    return;
  }
  JobEntry &current_fg_job = *(smash.current_fg_job);
  DO_SYS(kill(current_fg_job.pid, SIGINT));
  printf("smash: process %d was killed\n", (int)current_fg_job.pid);
  smash.current_fg_job = nullptr;
}

void alarmHandler(int sig_num)
{
  // TODO: Add your implementation
}
