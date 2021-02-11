#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
	if (0 == fork())
	{
		setsid();

		signal(SIGHUP, SIG_IGN);

		if (0 == fork())
		{
			close(0);
			close(1);
			close(2);

			int file = open("logg.txt", O_WRONLY, 0666);
        		dup2(file, 2);

			char *args[]={"./tjener", NULL};
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
