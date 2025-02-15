#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <filesystem>
#include <fcntl.h> // For open, O_RDWR, etc.
#include <string>
#include "command.h"
#include <string.h>
#include <fstream>

const char *Colors::RED = "\033[0;31m";
const char *Colors::GREEN = "\033[0;32m";
const char *Colors::YELLOW = "\033[0;33m";
const char *Colors::BLUE = "\033[0;34m";
const char *Colors::PURPLE = "\033[0;35m";
const char *Colors::CYAN = "\033[0;36m";
const char *Colors::WHITE = "\033[0;37m";
const char *Colors::NO_COLOR = "\033[0m";

const char *LOG_FILE = "process_log.txt";

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments, _numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_appendOut = 0;
	_appendErr = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(_simpleCommands, _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}

	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	if (_errFile)
	{
		free(_errFile);
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_appendOut = 0;
	_appendErr = 0;
}

void Command::printColor(const char *str, const char *color)
{
	printf("%s%s%s", color, str, Colors::NO_COLOR);
}

void Command::printError(const char *str)
{
	std::string resultStr = "(ERROR) " + std::string(str);
	const char *result = (char *)resultStr.c_str();
	printColor(result, Colors::RED);
}

void Command::printLog(const char *str)
{
	printColor(str, Colors::GREEN);
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);

		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
		printf("\n");
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default", _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _background ? "YES" : "NO");
	printf("\n\n");
}

void Command::execute()
{
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}

	// exit command
	if (strcmp(_simpleCommands[0]->_arguments[0], "exit") == 0)
	{
		printLog("---------------------  Good bye  ---------------------\n");
		exit(0); // Exit the shell
	}

	// cd command
	if (strcmp(_simpleCommands[0]->_arguments[0], "cd") == 0)
	{
		const char *path = _simpleCommands[0]->_numberOfArguments > 1 ? _simpleCommands[0]->_arguments[1] : getenv("HOME");
		char cwd[PATH_MAX];

		if (chdir(path) != 0)
		{
			perror("cd failed");
		}

		if (getcwd(cwd, sizeof(cwd)) != NULL)
		{
			// std::string str = "Current working directory: " + std::string(cwd);
			// printLog(str.c_str());
			printLog("Current working directory: ");
			printf("%s\n", cwd);
		}
		else
		{
			printError("getcwd() error");
			printf("\n");
			exit(1);
		}

		clear();
		prompt();
		return; // Return after executing "cd"
	}

	print();

	int prevPipe[2] = {-1, -1}; // For previous pipe ends

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		int pipe_fd[2];

		// Create a pipe for all but the last command
		if (i < _numberOfSimpleCommands - 1 && pipe(pipe_fd) < 0)
		{
			printError("Pipe creation failed!");
			exit(1);
		}

		pid_t pid = fork();

		if (pid == 0) // Child process -> Command
		{

			// Redirect input from the previous pipe if it exists
			if (prevPipe[0] != -1)
			{
				dup2(prevPipe[0], STDIN_FILENO);
				close(prevPipe[0]);
			}

			// Redirect output to the current pipe if not the last command
			if (i < _numberOfSimpleCommands - 1)
			{
				dup2(pipe_fd[1], STDOUT_FILENO);
				close(pipe_fd[1]);
			}

			// If it's the last command, handle redirection
			if (i == _numberOfSimpleCommands - 1)
			{
				if (_inputFile)
				{
					int in_fd = open(_inputFile, O_RDONLY);

					dup2(in_fd, STDIN_FILENO);
					close(in_fd);
				}

				if (_outFile)
				{
					int flags = O_WRONLY | O_CREAT | (_appendOut ? O_APPEND : O_TRUNC);
					int out_fd = open(_outFile, flags, 0666);

					dup2(out_fd, STDOUT_FILENO);
					close(out_fd);
				}

				if (_errFile)
				{
					int flags = O_WRONLY | O_CREAT | (_appendErr ? O_APPEND : O_TRUNC);
					int err_fd = open(_errFile, flags, 0666);

					dup2(err_fd, STDERR_FILENO);
					close(err_fd);
				}
			}

			const char *commandWord = _simpleCommands[i]->_arguments[0];
			execvp(commandWord, _simpleCommands[i]->_arguments);
			printError("Command execution failed\n");
			exit(1);
		}
		else if (pid > 0) // Parent process
		{
			// Close the write end of the current pipe in the parent
			if (i < _numberOfSimpleCommands - 1)
			{
				close(pipe_fd[1]); // Close write end
			}

			// Close the read end of the previous pipe if it exists
			if (prevPipe[0] != -1)
			{
				close(prevPipe[0]);
			}

			// Store the read end of the current pipe for the next command
			prevPipe[0] = pipe_fd[0];
			prevPipe[1] = pipe_fd[1];

			// Wait for the child process
			if ((i < _numberOfSimpleCommands - 1) || (!_background))
			{
				waitpid(pid, nullptr, 0);
			}
			else
			{
				waitpid(pid, nullptr, WNOHANG);
				std::string str = "Started in background, PID: " + std::to_string(pid) + "\n";
				printLog(str.c_str());
			}
		}
		else
		{
			printError("Couldn't fork process");
			exit(1);
		}
	}

	// Clear to prepare for the next command
	clear();
	prompt();
}

// Shell implementation

void Command::prompt()
{
	printColor("myshell>", Colors::CYAN);
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

void handleSIGINT(int sig)
{
	// Print a message or just ignore it
	printf("\n");
	Command::_currentCommand.prompt();
}

// Signal handler for SIGCHLD
void handleSIGCHLD(int sig)
{
	// Open the log file in append mode
	std::ofstream logFile(LOG_FILE, std::ios::app);
	if (logFile.is_open())
	{
		logFile << "Child process terminated\n";
		logFile.close();
	}
}

int main()
{
	// Set up signal handling for Ctrl-C
	signal(SIGINT, handleSIGINT);

	// Clears log file
	int fd = open(LOG_FILE, O_WRONLY | O_TRUNC);
	close(fd);

	// Set up signal handling for SIGCHLD to log process termination
	signal(SIGCHLD, handleSIGCHLD);

	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}
