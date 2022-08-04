#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

static void	print_error(char *msg)
{
	int	i;

	i = 0;
	if (msg != NULL)
	{
		while (msg[i] != 0)
			i++;
		write(STDERR_FILENO, msg, i);
	}
}

static int	errors(int err_num, char *msg)
{
	if (err_num == 1)
		print_error("error: cd: bad arguments");
	else if (err_num == 2)
		print_error("error: cd: cannot change directory to ");
	else if (err_num == 3)
		print_error("error: fatal");
	else if (err_num == 4)
		print_error("error: cannot execute ");
	print_error(msg);
	print_error("\n");
	if (err_num == 1 || err_num == 2)
		return (0);
	exit(EXIT_FAILURE);
}

static char	**next_pipe(char **cmd)
{
	int	i;

	i = 0;
	if (cmd == 0)
		return (0);
	while (cmd[i] != 0)
	{
		if (strcmp(cmd[i], "|") == 0)
			return (cmd + i + 1);
		i++;
	}
	return (0);
}

static void	exec_cmd(char **cmd, char **envp)
{
	pid_t	pid;

	pid = fork();
	if (pid < 0)
		errors(3, NULL);
	if (pid == 0)
		if (execve(cmd[0], cmd, envp) == -1)
			errors(4, cmd[0]);
	waitpid(0, NULL, 0);
}

static void	child(char **cmd, char **envp, int in, int fd[2])
{
	int	i;

	i = -1;
	if (dup2(in, STDIN_FILENO) < 0)
		errors(3, NULL);
	if (next_pipe(cmd) != 0 && dup2(fd[1], STDOUT_FILENO) < 0)
		errors(3, NULL);
	close(in);
	close(fd[0]);
	close(fd[1]);
	while (cmd[++i] != 0)
		if (strcmp(cmd[i], "|") == 0)
			break ;
	cmd[i] = 0;
	exec_cmd(cmd, envp);
	exit(0);
}

static void	parent(char ***cmd, int in, int fd[2], int *nb_wait)
{
	if (dup2(fd[0], in) < 0)
		errors(3, NULL);
	close(fd[0]);
	close(fd[1]);
	*cmd = next_pipe(*cmd);
	(*nb_wait)++;
}

static void	exec_pipes(char **cmd, char **envp)
{
	pid_t	pid;
	int		in;
	int		fd[2];
	int		nb_wait;

	nb_wait = 0;
	in = dup(STDIN_FILENO);
	if (in < 0)
		errors(3, NULL);
	while (cmd)
	{
		if (pipe(fd) < 0)
			errors(3, NULL);
		pid = fork();
		if (pid < 0)
			errors(3, NULL);
		if (pid == 0)
			child(cmd, envp, in, fd);
		else
			parent(&cmd, in, fd, &nb_wait);
	}
	close(in);
	while (nb_wait-- > 0)
		waitpid(0, NULL, 0);
}

static void	exec_cd(char **cmd)
{
	if (cmd[1] == 0 || cmd[2] != 0)
		errors(1, NULL);
	else if (chdir(cmd[1]) == -1)
		errors(2, cmd[1]);
}

static void	exec(char **cmd, char **envp)
{
	if (strcmp(cmd[0], "cd") == 0)
		exec_cd(cmd);
	else if (next_pipe(cmd) == 0)
		exec_cmd(cmd, envp);
	else
		exec_pipes(cmd, envp);
}

int	main(int argc, char **argv, char **envp)
{
	int	i;
	int	start;

	i = 1;
	start = 1;
	if (argc < 2)
		return (0);
	while (argv[i] != 0)
	{
		if (strcmp(argv[i], ";") == 0)
		{
			argv[i] = 0;
			exec(argv + start, envp);
			i++;
			while (argv[i] != 0 && strcmp(argv[i], ";") == 0)
				i++;
			start = i;
		}
		else
			i++;
	}
	exec(argv + start, envp);
	return (0);
}
