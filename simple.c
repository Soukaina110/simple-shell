#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "main.h"
#define MAX_PATH_LENGTH 1024
#define MAX_COMMAND_LENGTH 1024


/**
* display_environment - Displays the contents of the environment variables.
* This function iterates through the array of environment variables and prints
* each variable along with its value to the standard output.
* The environment variable array is terminated by a NULL pointer.
* Uses the global variable 'environ'
* that is automatically provided by the system.
*/


extern char **environ;

void display_environment(void)
{
char **env = environ;
while (*env != NULL)
{
printf("%s\n", *env);
env++;
}
}


/**
* checkCommand - Checks the existence of a file in a specified path.
* Given a path and a file name, this function constructs the full path to the
* file and checks if the file exists at that location. It uses the access
* function to determine the file's existence.
*
* @path: The path where the file is expected to be located.
* @file: The name of the file to be checked for existence.
*
* Return: 1 if the file exists, 0 if it does not exist, and -1 if
*          an error occurs during the access check.
*          Note: The access function is used with the F_OK flag to check only
*          for the file's existence.
*/

int checkCommand(const char *path, const char *file)
{
char full_path[MAX_PATH_LENGTH];
snprintf(full_path, sizeof(full_path), "%s/%s", path, file);
return (access(full_path, F_OK) != -1);
}

/**
* launchCommand - Attempts to launch a command by searching for its executable
* in the directories specified in the PATH environment variable.
* The function tokenizes the command string into executable and arguments,
* then searches for the executable in each directory listed in the PATH.
* If the executable is found, a child process is created using fork(),
* and the execvp() function is used to execute the command.
* The parent process waits for the child process to complete and returns
* the exit status of the child.
*
* @cmd: The command string to be launched, along with its arguments.
*
* Return: 0 if the command is successfully launched and executed,
*          and returns 1 if an error occurs during the process.
*          Note: The function uses the checkCommand function to verify the
*          existence of the command in each directory of the PATH.
*/


int launchCommand(const char *cmd)
{
pid_t pid;
char *path = getenv("PATH");
char path_copy[MAX_PATH_LENGTH];
char full_cmd[MAX_PATH_LENGTH];
char *arguments[MAX_COMMAND_LENGTH];
char *saveptr1, *saveptr2;
char *token;
char *token_cmd;
int i = 0;

strncpy(path_copy, path, MAX_PATH_LENGTH);

token_cmd = strtok_r((char *)cmd, " ", &saveptr1);
while (token_cmd != NULL)
{
arguments[i++] = token_cmd;
token_cmd = strtok_r(NULL, " ", &saveptr1);
}
arguments[i] = NULL;

token = strtok_r(path_copy, ":", &saveptr2);

do {
snprintf(full_cmd, sizeof(full_cmd), "%s/%s", token, arguments[0]);
if (checkCommand(token, arguments[0]))
{
pid = fork();
if (pid == -1)
{
perror("fork");
return (1);
}
if (pid == 0)
{
if (execvp(full_cmd, arguments) == -1)
{
perror(full_cmd);
exit(EXIT_FAILURE);
}
}
else
{
int status;
waitpid(pid, &status, 0);
if (WIFEXITED(status))
{
if (WEXITSTATUS(status) == 0)
return (0);
else
return (1);
}
else
{
perror("Child process did not exit successfully");
return (1);
}
}
}
} while ((token = strtok_r(NULL, ":", &saveptr2)) != NULL);
printf("'%s' not found\n", arguments[0]);
return (0);
}
/**
* main - Entry point for a simple shell program.
* This program provides a basic command-line interface, where the user can
* enter commands. It supports the 'exit' command to terminate the shell,
* the 'env' command to display environment variables, and other commands
* that are executed using the 'launchCommand' function.
* The shell checks for specific conditions, such as the presence of special
* characters or multiple words in a command, and provides appropriate error
* messages. It continues to prompt the user for commands until the 'exit'
* command is entered.
*
* Return: 0 upon successful execution of the program.
*/

int main(void)
{
char *cmd = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
char *space_ptr;

do {
printf("#cisfun$ ");
fgets(cmd, MAX_COMMAND_LENGTH, stdin);

if (strcmp(cmd, "exit\n") == 0)
{
printf("Exiting the shell. Goodbye!\n");
break;
}
else if (strcmp(cmd, "env\n") == 0)
{
display_environment();
}
else
{
if (strchr(cmd, ';') != NULL || strchr(cmd, '|') != NULL ||
strchr(cmd, '<') != NULL || strchr(cmd, '>') != NULL)
{
printf("Error: Command contains special characters or advanced features.\n");
continue;
}

space_ptr = strchr(cmd, ' ');
if (space_ptr != NULL && *(space_ptr + 1) != '-')
{
printf("Error: Command should contain only one word.\n");
continue;
}

cmd[strcspn(cmd, "\n")] = '\0';

if (launchCommand(cmd) == 0)
continue;

}
} while (1);

free(cmd);
return (0);
}

