
#ifndef command_h
#define command_h

// Command Data Structure
struct SimpleCommand
{
	// Available space for arguments currently preallocated
	int _numberOfAvailableArguments;

	// Number of arguments
	int _numberOfArguments;
	char **_arguments;

	SimpleCommand();
	void insertArgument(char *argument);
};

struct Command
{
	int _numberOfAvailableSimpleCommands;
	int _numberOfSimpleCommands;
	SimpleCommand **_simpleCommands;
	char *_outFile;
	char *_inputFile;
	char *_errFile;
	int _background;
	int _appendOut;
	int _appendErr;

	void prompt();
	void execute();
	void clear();
	void print();
	void printColor(const char *str, const char *color);
	void printError(const char *str);
	void printLog(const char *str);

	Command();
	void insertSimpleCommand(SimpleCommand *simpleCommand);

	static Command _currentCommand;
	static SimpleCommand *_currentSimpleCommand;
};

struct Colors
{
	static const char *RED;
	static const char *GREEN;
	static const char *YELLOW;
	static const char *BLUE;
	static const char *PURPLE;
	static const char *CYAN;
	static const char *WHITE;
	static const char *NO_COLOR;
};

#endif
