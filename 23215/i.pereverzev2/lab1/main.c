#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <stdio.h>
#include <ulimit.h>
#include <getopt.h>
#include <stdlib.h>

extern char **environ;

int str_not_int(const char *str)
{
	int len = 0;
	while (*str)
	{
		if (len++ >= 9)
		{
			return 1;
		}
		if (((*str) > '9') || ((*str) < '0'))
		{
			return 1;
		}
		str++;
	}
	return 0;
}

char *arg_split(char *arg)
{
	while ((*arg != '=') && (*arg != 0))
		arg++;
	if (*arg == 0)
		return NULL;
	*arg = 0;
	return arg + 1;
}

int main(int argc, char *argv[])
{
	int code = 0;
	char options[] = "ispuU:cC:dV:v-";

	while ((code = getopt(argc, argv, options)) != -1)
	{
		char curopt = (char)code;
		if (curopt == 'i')
		{
			printf("uid: %d\neuid: %d\n", getuid(), geteuid());
			printf("gid: %d\negid: %d\n", getgid(), getegid());
		}
		if (curopt == 's')
		{
			pid_t pid = getpid();
			if (setpgid(pid, pid))
				perror("setpgid failed");
			printf("process now is leader of the process group\n");
		}
		if (curopt == 'p')
		{
			pid_t pid = getpid();
			pid_t ppid = getppid();
			printf("pid: %d\n", pid);
			printf("ppid: %d\n", ppid);
			printf("pgid: %d\n", getpgid(pid));
		}
		if (curopt == 'u')
		{
			struct rlimit lim;
			if (getrlimit(RLIMIT_FSIZE, &lim) == -1)
				perror("get ulimit failed");
			else
				printf("ulimit: %ld\n", lim.rlim_cur);
		}
		if (curopt == 'U')
		{
			if (str_not_int(optarg))
			{
				fprintf(stderr, "-U option argument is incorrect\n"); 
				continue;
			}
			struct rlimit ulim;
			if (getrlimit(RLIMIT_FSIZE, &ulim) == -1)
			{
				perror("can't get limit");
				continue;
			}
			ulim.rlim_cur = atoi(optarg);
			if (setrlimit(RLIMIT_FSIZE, &ulim) == -1)
			{
				perror("can't change ulimit");
				continue;
			}
			printf("ulimit changed to %ld\n", ulim.rlim_cur);
		}
		if (curopt == 'c')
		{
			struct rlimit lim;
			if (getrlimit(RLIMIT_CORE, &lim) == -1)
				perror("can't get core file size limit");
			printf("%ld\n", lim.rlim_cur);
		}
		if (curopt == 'C')
		{
			if (str_not_int(optarg))
			{
				fprintf(stderr, "-C option argument is incorrect\n");
				continue;
			}
			struct rlimit lim;
			if (getrlimit(RLIMIT_CORE, &lim) == -1)
			{
				perror("can't get core file size limit");
				continue;
			}
			lim.rlim_cur = atoi(optarg);
			if (setrlimit(RLIMIT_CORE, &lim) == -1)
			{
				perror("can't change core limit");
				continue;
			}
			printf("core file size limit changed to %ld\n", lim.rlim_cur);
		}
		if (curopt == 'd')
		{
			long max_path_len = pathconf(".", _PC_PATH_MAX);
			char *buf = getcwd(NULL, max_path_len);
			if (!buf)
			{
				perror("can't get current path");
				continue;
			}
			printf("%s\n", buf);
		}
		if (curopt == 'v')
		{
			while (*environ)
			{
				printf("%s\n", *environ);
				environ++;
			}
		}
		if (curopt == 'V')
		{
			char *value = arg_split(optarg);
			if (value == NULL)
			{
				fprintf(stderr, "incorrect argument of -V option\n");
				continue;
			}
			if (setenv(optarg, value, 1) == -1)
			{
				fprintf(stderr, "unable to set %s to %s; ", optarg, value);
				perror("setenv error");
				continue;
			}
		}
	}
	return 0;
}
