# Shell Project

## Overview

This project is a simple shell implementation in C++ that supports executing commands, handling input/output redirection, and piping multiple commands. It demonstrates process management using `fork`, `execvp`, and `waitpid` system calls.

## Features

- Command execution using `execvp`
- Input (`<`), output (`>`), and error (`2>`) redirection
- Pipelining (`|`) support for chaining commands
- Background process execution using `&`
- Color-coded output for logs and errors

## Requirements

To build and run the shell, ensure you have:

- A Linux-based system
- g++ compiler
- Make

## Installation & Compilation

```sh
# Clone the repository
git clone https://github.com/Ah-Ibrahim/Shell.git
cd /Shell

# Compile the shell
make   # If a Makefile is available

# Then run the shell
./shell
```

## Usage

```sh
./myshell
```

Once inside the shell, you can execute commands like:

```sh
ls -l
ls | grep .cpp
cat file.txt > output.txt
./script.sh &
```

## Files

```
├── .gitignore            # Git ignore file
├── command.cc            # Command class implementation
├── command.h             # Command class header file
├── command.o             # Compiled object file
├── lex.yy.c              # Generated lexical analyzer
├── lex.yy.o              # Lexical analyzer object file
├── Makefile              # Compilation instructions
├── process_log.txt       # Log file for processes
├── README.md             # Documentation
├── shell                 # Shell executable
├── shell.l               # Lex file for lexical analysis
├── shell.y               # Yacc/Bison grammar file
├── y.tab.c               # Generated parser code
├── y.tab.h               # Generated parser header
├── y.tab.o               # Parser object file
```

## Issues & Debugging

If you encounter issues:

- Ensure the compiled binary has execution permissions: `chmod +x myshell`
- Use `strace ./myshell` to debug system calls
- Add debug prints in `execute()` to track fork and execvp behavior

## Author

Developed by Ahmed Ibrahim. Contributions and feedback are welcome!
