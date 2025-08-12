# Script Executor and Grep Utility

## Project Overview

This project contains two C programs:
1. `scripter.c` - Executes script files with support for pipes, redirections, and background processes
2. `mygrep.c` - A simple grep-like tool to search files line by line

## Compilation

Compile both programs with:

```bash
gcc scripter.c -o scripter
gcc mygrep.c -o mygrep

---
```
# scripter - Script Executor
### Usage

```bash
./scripter script_file
```
### Features
- Executes commands from script files
- Supports pipes (|) between commands
- Handles I/O redirections:
  - < for input
  - \> for output
  - !> for stderr
- Background execution with &

### Script Format
Script Must Start With:
```bash
## Script de SSOO
```
Followed by one command per line. Example:
```bash
## Script de SSOO
ls -l | grep .c > output.txt
sleep 10 &
```

# mygrep - File Search Tool
### Usage
```bash
./mygrep file_path search_term
```
### Features
- Searches for terms in files
- Prints matching lines with numbers
- Uses low-level file operations
- Case-sensitive matching
