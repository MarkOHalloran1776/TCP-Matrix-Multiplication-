# TCP-Matrix-Multiplication-
This is a program written in C that uses sockets to communicate between two programs where one (client2.c) asks what n for 2 n x n randomised matricies to multiply from the user which then sends it over to the other program (server2.c) which does the multiplication and sends them back over using the fork() method.

Client2.c is the master process where you input an ipv4 address via the cml (./client2 <ip> , if you do not input any parametres it defaults to local host
server2.c: this is the worker program which does not take in any parametres , this just needs to be running before the client sends requests
