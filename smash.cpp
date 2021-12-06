#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
#include "Exception.h"

int main(int argc, char *argv[])
{
    if (signal(SIGTSTP, ctrlZHandler) == SIG_ERR)
    {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR)
    {
        perror("smash error: failed to set ctrl-C handler");
    }


    struct sigaction new_alarm_action;
    new_alarm_action.sa_handler = &alarmHandler;
    new_alarm_action.sa_flags = SA_RESTART;
    if (sigaction(SIGALRM, &new_alarm_action,NULL) == -1)
    {
        perror("smash error: failed to set alarm handler");
    }


    //TODO: setup sig alarm handler

    SmallShell &smash = SmallShell::getInstance("smash");
    while (true)
    {
        try
        {
            smash.jobList.removeFinishedJobs();
            std::cout << smash.name;
            std::string cmd_line;
            std::getline(std::cin, cmd_line);
            smash.executeCommand(cmd_line.c_str());
        }
        catch (Exception &e)
        {
            e.handle();

            if (!smash.isMaster)
            {
                exit(0);
            }
        }
        catch (...)
        {
           perror("hello! You have a bug :)\n");
        }
    }
    return 0;
}