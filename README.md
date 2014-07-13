libSesame++
========

Sesame++ C++ open-file management library. 

This library is meant primary for use in FUSE based user-space loop-back file-systems. 
A FUSE file-system has to potentially keep track of the open files of hundreds or more seperate processes. 
Each process may in theory have many hundreds of files open at any given time. As a result, the process implementing the user space file-system may need to keep track of tens of thousands of open files. If as is common in loopback file-systems, each open file is mapped to a file opened by the file-system process, the file-system process will hit the per-process open file-handle limit at an order of magnitude fewer open files. As an alternative, many FUSE loop-back file-systems will do an open->operation->close sequence on each read, write or other operation. This, while not running into the limit on open file-handles, comes at a rather high price. Instead of 3 times the number of kernel/userspace switches, we would be up to five times, thats double the overhead. This library provides the author of FUSE loopback file-systems with a better solution. It allows the creation of a container of virtually opened file handles, that it will try to keep open until its about to hit the per process open-file-handle limit.  When it does, it will look at the least recently used file handle, and will temporarily close that one, opening it just in time if the handle is addressed at any later time. 


