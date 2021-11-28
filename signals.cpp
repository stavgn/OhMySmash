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
  if ((!JobEntry::is_alive(smash.current_fg_job.pid)) || (smash.current_fg_job.status != JobEntry::FOREGROUND))
  {
    cout << smash.name << flush;
    return;
  }
  DO_SYS(kill(smash.current_fg_job.pid, SIGSTOP));
  smash.current_fg_job.status = JobEntry::STOPPED;
  smash.jobList.addJob(smash.current_fg_job);
  printf("smash: process %d was stopped\n", (int)smash.current_fg_job.pid);
}

void ctrlCHandler(int sig_num)
{
  printf("smash: got ctrl-C\n");
  SmallShell &smash = SmallShell::getInstance("smash");
   if ((!JobEntry::is_alive(smash.current_fg_job.pid)) || (smash.current_fg_job.status != JobEntry::FOREGROUND))
  {
    cout << smash.name << flush;
    return;
  }
  DO_SYS(kill(smash.current_fg_job.pid, SIGINT));
  printf("smash: process %d was killed\n", (int)smash.current_fg_job.pid);
}

void alarmHandler(int sig_num)
{
  // TODO: Add your implementation
}
