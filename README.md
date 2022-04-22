# SynchronizedProcessComunication
This is an example of process comunication on Linux.

Init program creates an array with semaphores, reserves shared memory and then creates 3 processes. Each one resolves his own proces_x program.

Process one asks us to type in something and then sends the message forward to process 2.
Process 2 adds "-2" suffix and then forwards the message to process 3.
Process 3 acts accordingly to process 2.

Processes can be controlled using singnals from another terminal. Those are:
  SIGBUS - used for ending the program,
  SIGSTKFLT - used for pausing,
  SIGCHLD - used for unpausing.
  
Signals work on every process.
