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
        }
    }
    return 0;
}