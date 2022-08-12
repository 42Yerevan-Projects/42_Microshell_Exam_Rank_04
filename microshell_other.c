#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

size_t 	ft_strlen(char *str)
{
	int i = 0;
	while (str[i])
		++i;
	return (i);
}

void 	ft_strerror(char *str, char *arg)
{
	write(2,str,ft_strlen(str));
	if (arg)
		write(2,arg, ft_strlen(arg));
	write(2,"\n",1);
	kill(0,SIGINT);
}

void ft_cd(char **arg)
{
	if (!arg[1] || arg[2])
		ft_strerror("error: cd: bad arguments", NULL);
	else if (chdir(arg[1]) == -1)
		ft_strerror("error: cd: cannot change directory to", arg[1]);
}

void child_process(char **arg, int *fd, int fd_in, int last, char *env[])
{
	if (dup2(fd_in, 0) == -1)
		ft_strerror("error: fatal",NULL);
	if (last == 0)
		if (dup2(fd[1],1) == -1)
			ft_strerror("error: fatal",NULL);
	close(fd_in);
	close(fd[0]);
	close(fd[1]);
	if (execve(arg[0],arg,env) == -1)
		ft_strerror("error: cannot execute ", arg[0]);
}

int ft_execute(char **arg, char *env[], int fd_in)
{
	pid_t pid;
	int fd[2];
	int i = 0;
	int last = 0;

	while(arg[i] && strcmp(arg[i], "|"))
		++i;
	if (!arg[i])
		last = 1;
	arg[i] = 0;
	if (pipe(fd) == -1 || (pid = fork()) == -1)
		ft_strerror("error: fatal",NULL);
	else if (pid == 0)
		child_process(arg,fd,fd_in, last,env);
	else 
	{
		if (dup2(fd[0],fd_in) == -1)
			ft_strerror("error:fatal",NULL);
		close(fd[0]);
		close(fd[1]);
	}
	if (last == 0)
		return (i+1);
	return (i);

}

void launch_cmd(char **arg,char *env[])
{
	int i,status,count,fd_in;
	
	if(!arg[0])
	   return ;
	count = 0;
	if(!strcmp(arg[0], "cd"))
		return (ft_cd(arg));
	if ((fd_in = dup(0)) == -1)
		ft_strerror("error: fatal ",NULL);
	i = 0;
	while(arg[i])
	{
		i += ft_execute(arg + i,env,fd_in);
		++count;
	}
	close(fd_in);

	while(count > 0)
	{
		waitpid(-1,&status,0);
		--count;
	}
}

int main(int argc, char **argv, char **env)
{
	int i,start;

	if (argc < 2)
		 return 0;
	i = 1;
	start = 1;
	while(argv[i])
	{
		if (strcmp(argv[i], ";") == 0)
		{
			argv[i] = 0;
			launch_cmd(argv + start, env);
			++i;
			while(argv[i] && strcmp(argv[i], ";") == 0)
				++i;
			start = i;
		}
		else 
			++i;
	}
		launch_cmd(argv + start, env);
		return (0);
}
