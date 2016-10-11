# Mutation

Mutation is *not* to be used. It is an experiment of a biologic concept transposed to computer science: the mutation.

From generation to generation, the DNA has small changes, the mutations. What about if we would try to make a program mutating ? That's what this repository is able to do, in a *very* simple way.

## How to bring it to life

You need a POSIX-conforming operating system.

Create any program, and copy it to `program/base` (it has to be an executable file). If you have no program, a very tiny hello world program written in GAS for ARM is provided in `program/base.s`.

Then, compile the worker:
```
gcc -o worker worker.c
```
The worker is a small program in charge of running the mutating program, creating its mutations, and determining if the mutations are viable. It is very verbose, because it says you how your program is evoluating (its size, its output, and its viability). It rejects any non-viable mutation (by testing the program), and re-uses the last working version.

A program is said as non-viable if:
- It cannot be read by the execve() system call
- It is interrupted by a signal (SIGTERM, SIGINT, SIGKILL, SIGSEGV)
- It runs for more than two seconds

By running the worker (`./worker`), you will see your screen flooded with mutation informations, and the output of your program (it may have no output). You program will slowly become bigger and bigger. After running the worker for some minutes, it has probably nothing to do with the original program.

Of course this random-created program will do nothing interesting. What is interesting is to see it mutating: you don't know what your computer is gonna do in the next mutation (well, probably nothing special).
