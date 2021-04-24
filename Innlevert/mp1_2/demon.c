#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mount.h>

int main()
{
	mount("none", "/proc", "proc", 0, NULL);

	if(getppid() == 1)
	{
		int logg = open("/var/log/debug.log", O_CREAT | O_WRONLY | O_APPEND, 0666);
                dup2(logg, 2);

		fprintf(stderr, "PID:\t%d", getpid());
		fprintf(stderr, "PPID:\t%d", getppid());

		char *args[]={"tjener", NULL};
		execvp(args[0], args);

		return 0;
	}

	if (0 == fork())
	{
		setsid();

		signal(SIGHUP, SIG_IGN);

		if (0 == fork())
		{
			close(0);
			close(1);
			close(2);

			int logg = open("/var/log/debug.log", O_CREAT | O_WRONLY | O_APPEND, 0666);
			dup2(logg, 2);

			char *args[]={"tjener", NULL};
			execvp(args[0], args);

		}
		else
		{
			exit(0);
		}

	}
	else
	{
		exit(0);
	}
	return 0;
}
