# Twitter-like client-server
This is a twitter-like client-server project. There are two applications in this repository (client and server).

The communication between client and server is done through a protocol defined by ourselves above TCP sockets. This was developed as the final 
project of the Operating Systems 2 class at UFRGS and uses many concepts of distributed systems and parallel programming.

## Client - General architecture
- Main thread: starts "To server" and "From server" threads and waits until they exit.
- "To server" thread: read from the input and send messages to the server, until EOF or the connection gets closed.
- "From server" thread: read from connection and print to output until the connection gets closed.
- SIGINT listener: triggered when the user types Ctrl+c. Close TCP connection.

## Server - General architecture
It's a TCP server that listens on the main thread. Whenever a client gets connected, a new thread is created to handle this communication.
Because of multithreading, many concepts of parallel programming were used, such as critical sessions, mutexes, semaphores, etc.

## How to build and run
1. Build:
 ```
 make
 ```
2. Start server:
 ```
 ./server/server 
 ```
3. Start client:
```
./client/client @profile 127.0.0.1 4242
```
