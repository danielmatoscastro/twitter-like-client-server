# Twitter - Client

## General archtecture
- Main thread: starts "worker" threads and waits until they exit.
- To server thread: read from input and send messages to server, until EOF or connection get closed.
- From server thread: read from connection and print to output until connection get closed.
- SIGINT listener: triggered when user types Ctrl+c. Close TCP connection.
