#include "builtin.h"


void checkSpaces(char **cmd);
void addHistory(char *cmd);
void printHistory();


// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	// Your code here
	checkSpaces(&cmd);
	if (strncmp(cmd, "exit", 4) != 0) {
		return false;
	}

	return cmd[4] == '\0' || cmd[4] == ' ' || cmd[4] == '\t';
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']


int
cd(char *cmd)
{
	// Your code here
	checkSpaces(&cmd);
	if (strncmp(cmd, "cd", 2) != 0) {
		return false;
	}


	char *dir = cmd + 3;

	checkSpaces(&dir);

	if (*dir == '\0') {
		if (chdir(getenv("HOME")) == -1) {
			printf("There is no home directory\n");
		}
		return true;
	}

	char *path = dir;

	while (*dir != '\0' && !isspace(*dir)) {
		dir++;
	}

	if (*dir != '\0')
		*dir = '\0';


	if (chdir(path) == -1) {
		printf("No such file or directory\n");
	}

	return true;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	// Your code here
	checkSpaces(&cmd);
	if (strncmp(cmd, "pwd", 3) != 0)
		return false;

	if (cmd[3] != '\0' && !isspace(cmd[3]))
		return false;

	char path[BUFLEN];
	printf("%s\n",
	       getcwd(path,
	              sizeof(path)));  // Deberia ver que pasa si no esta el archivo?
	return true;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here
	// checkSpaces(&cmd);
	// if (strncmp(cmd, "history", 7) != 0) {
	//	status = 1;
	//	return false;
	//}
	// char *arg = cmd + 7;
	// checkSpaces(&arg);
	// if (cmd[7] != '\0' && !isspace(cmd[7])) {
	//	status = 1;
	//	return false;
	//}
	return 0;
}


// Function to Print the previous n command
// TODO Implement this function
void
printHistory()
{
	// for (int i = 0; i < n; i++) {
	//	printf("%d %s\n", i + 1, commands[i]);
	// }
}


void
checkSpaces(char **cmd)
{
	while (isspace(**cmd)) {
		(*cmd)++;
	}
}