# [Documentation](README.md) > IPC

A form of inter-process communication is applied between the server and it's workers. Different mechanisms are used for 
Windows and Unix-ish operating systems. 

The memory pipe itself is created with a unique name before the worker process is spawned. The process will then 
connect to the memory pipe. This can be done because the server will supply the unique pipe name to the worker as
part of the argument sent to the application.

## OS specific implementations

### Windows

The application makes use of so called memory "pipes" for when communicating with another process. This is done by
creating a pipe with a unique name per child-process. The format for each pipe is: `\.\pipe\everstore$ID` (replacing 
`$ID` with the a unique ID for the process). This is the most common way to in-memory data between multiple processes on
Windows.

### Unix 

The application makes use of unix sockets. Depending on the what type of unix system you are using (BSD or Linux), the
unix socket might be represented as a file located in the `/tmp/everstore` directory or not.
