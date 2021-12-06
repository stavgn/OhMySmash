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
    return;
  }
  DO_SYS(kill(smash.current_fg_job.pid, SIGINT));
  printf("smash: process %d was killed\n", (int)smash.current_fg_job.pid);
}

void alarmHandler(int sig_num)
{
  // FILE *fd = fopen("/tmp/myfile.txt", "w");
  // fprintf(fd, "smash: got an alarm\n");
  // fflush(fd);
  // fclose(fd);
  cout <<"smash: got an alarm" << endl << flush;
  SmallShell &smash = SmallShell::getInstance("smash");
  JobEntry &job = (smash.timedJobList.getFirstJob()).eCommand->job;
  smash.timedJobList.removFirstJob();
  if ((JobEntry::is_alive(job.pid)))
  {
    DO_SYS(kill(job.pid, SIGINT));
    cout << "smash: " << job.cmd_line << " timed out!" << endl << flush;
  }

  if(smash.timedJobList.empty())
  {
    return;
  }
  const TimedJobEntry &next_job = (smash.timedJobList.getFirstJob());
  if (next_job.time_left() == 0) //if the next alarm should go now
  {
    DO_SYS(kill(getpid(), SIGALRM));
    return;
  }

  alarm(next_job.time_left());

}
