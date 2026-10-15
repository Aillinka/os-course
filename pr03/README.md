# Practical Work 3 — Processes in Linux

**Course:** Operating Systems
**Level:** C
**Environment:** Ubuntu Linux

## Objective

The objective of this practical work is to study
process creation, execution, synchronization,
and interprocess communication in Linux.

## Implementation

A mini-shell was implemented in C.

The program supports:
- Reading and executing commands
- Command-line arguments
- Process creation using fork()
- Program execution using execvp()
- Waiting for processes using waitpid()
- Built-in cd and exit commands
- Pipelines using pipe() and dup2()
- Input redirection using <
- Output redirection using >
- Background execution using &
- Reaping terminated background processes
  using SIGCHLD and waitpid()

## Compilation

    gcc -std=c11 -O2 -Wall -Wextra -o minishell minishell.c

## Execution

    ./minishell

Example commands:

    pwd
    echo Hello Linux
    ls | wc -l
    echo Hello > output.txt
    cat < output.txt
    sleep 2 &
    exit

## Testing

Automated test commands are stored in
test_commands.txt.

Test output is saved in test_results.txt.

## System Call Analysis

The program was investigated using strace -f.

The results are stored in trace.txt.

The command:

    ls | wc -l

requires the shell to create a pipe and
two child processes.

fork() creates child processes.

execvp() replaces the child process image
with the requested executable.

dup2() connects standard input and output
to the pipe.

waitpid() allows the parent to wait for
foreground commands.

SIGCHLD notifies the shell when child
processes terminate.

## Process Management

Foreground commands are executed while the
parent waits for their completion.

Background commands are started without
blocking the shell.

Terminated background children are reaped
to avoid persistent zombie processes.

## Limitations

This is an educational mini-shell.

It does not implement all Bash features,
such as environment variable expansion,
logical operators, or complete job control.

## Conclusion

The practical work demonstrates how Linux
processes are created and managed using
fork(), execvp(), waitpid(), and SIGCHLD.

Pipelines allow multiple processes to
exchange data using file descriptors.

The strace analysis shows how high-level
process operations correspond to Linux
system calls.
