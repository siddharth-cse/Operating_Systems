# Operating_Systems

Lab1

Processes (fork() and exec())

**init()** is the first process or the systemcall that is made. It has PID==1. Every other process spawned in user space has to be a child to this process.

Every new process is spawned from a sequence of fork() and exec(). 

So a new command say `ls` is a program so when we run it in shell, it runs **fork()** to get a new PID and runs **exec()**. This is because **fork()** fetches a new PID whereas **exec()** starts a new process with the same PID. So if this sequence is not maintained, and we forcefully run `exec ls`, we observe the shell collapses. The reason being `ls` had the same PID as the shell and it exists once it exits automatically once the program is executed. So in theory exec() ran `ls` but the shell was not running to show its output. 

dup2()

File descriptors are integer numbers which acts as points to files. FD==1 is STDOUT and FD==0 is STDIN. FD==2 is std err.
We can manupilate these FDs through dup2(). Say we call dup2(open("temp.txt"),1). And then we call printf(), the STDOUT is now replaced with temp.txt so all the output will be redirected to the file instead of the terminal.
