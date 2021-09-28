/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcollas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/26 18:09:22 by rcollas           #+#    #+#             */
/*   Updated: 2021/09/28 19:31:44 by rcollas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/pipex.h"

int	infile_cmd(t_var *var, int **pipefd, int i)
{
	char	**cmd_args;

	cmd_args = ft_split(var->av[i + 2], ' ');
	printf("cmds infile = %s\n", var->cmds[i]);
	dup2(var->file1, STDIN_FILENO);
	dup2(pipefd[i + 1][1], STDOUT_FILENO);
	if (close(var->file1) == -1)
	{
		perror("Infile close failed");
		return (free_arg(cmd_args));
	}
	if (execve(var->cmds[i], cmd_args, NULL) == FAIL)
	{
		printf("cmds infile = %s\n", var->cmds[i]);
		perror("Execve failed:");
		return (free_arg(cmd_args));
	}
	free_arg(cmd_args);
	return (1);
}

int	in_between_cmds(t_var *var, int **pipefd, int i)
{
	char	**cmd_args;

	cmd_args = ft_split(var->av[i + 2], ' ');
	dup2(pipefd[i][0], STDIN_FILENO);
	dup2(pipefd[i + 1][1], STDOUT_FILENO);
	if (execve(var->cmds[i], cmd_args, NULL) == FAIL)
	{
		perror("Execve failed:");
		return (free_arg(cmd_args));
	}
	free_arg(cmd_args);
	return (1);
}

int	outfile_cmd(t_var *var, int **pipefd, int i)
{
	char	**cmd_args;

	cmd_args = ft_split(var->av[i + 2], ' ');
	printf("cmds outfile = %s\n", var->cmds[i]);
	dup2(var->file2, STDOUT_FILENO);
	dup2(pipefd[i][0], STDIN_FILENO);
	if (close(var->file2) == -1)
	{
		perror("Outifle close failed");
		return (free_arg(cmd_args));
	}
	if (execve(var->cmds[i], cmd_args, NULL) == FAIL)
	{
		printf("cmds outfile = %s\n", var->cmds[i]);
		perror("Execve failed:");
		return (free_arg(cmd_args));
	}
	free_arg(cmd_args);
	return (1);
}

int	proceed_pipes(t_var *var, int **pipefd, int i)
{
	char	**cmd_args;

	cmd_args = ft_split(var->av[i + 2], ' ');
	if (i == 0)
		infile_cmd(var, pipefd, i);
	else if (i == var->size - 2)
		outfile_cmd(var, pipefd, i);
	else
		in_between_cmds(var, pipefd, i);
	return (1);
}

int	close_pipes(t_var *var, int **pipefd, int i)
{
	int	j;

	j = -1;
	while (++j < var->size + 1)
	{
		if (i != j)
		{
			//printf("[%d][0]\n", j);
			if (close(pipefd[j][0]) == -1)
			{
				//printf("failed on [%d][0]\n", j);
				perror("Close failed");
				return (0);
			}
		}
		if (i + 1 != j)
		{
			//printf("[%d][1]\n", j);
			if (close(pipefd[j][1]) == -1)
			{
				//printf("failed on [%d][1]\n", j);
				perror("Close failed");
				return (0);
			}
		}
	}
	return (1);
}

int	exec(t_var *var, int **pipefd, pid_t *pids)
{
	int	i;
	int	status;

	i = -1;
	while (++i < var->size - 1)
	{
		pids[i] = fork();
		if (pids[i] == -1)
		{
			perror("Fork failed:");
			return (0);
		}
		if (pids[i] == 0)
		{
			if (close_pipes(var, pipefd, i) == FAIL)
				return (0);
			if (proceed_pipes(var, pipefd, i) == FAIL)
				return (0);
		}
	}
	if (close(pipefd[0][0]) == -1)
		perror("Close failed");
	if (close(pipefd[var->size][1] == -1))
		perror("Close failed");
	while (++i < var->size - 1)
		waitpid(pids[i], &status, 0);
	return (1);
}
