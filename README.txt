Frank Eslami
CS 372 Project 1
README.txt


How to compile the programs
============================
1. Place all 3 files in the same directory:
     1) chatserve.cpp: server
     2) chatclient.c: client
     3) makefile

2. Type "make" without quotations to compile the programs.


How to launch the programs
===========================
3. Start chatserve like so:
     chatserve <port>
     Port represents a free port number that can be used.
     The chatserve will now be listening for incoming connections.
     The command prompt will show the hostname and port of chatserve.

4. Start chatclient like so:
     chatclient <hostname> <port>
     Hostname is the hostname of chatserve (provided by chatserve command prompt).
     Port is the port number of chatserve (provided by chatseve command prompt).


How the programs communicate
=============================
Once steps 1-4 above are complete, chatclient prompts the user for their user handle (one word with 10 or less characters). 

Then chatclient asks the user for a message to send to chatserve.

Chatclient sends the message to chatserve.

Chatserve displays chatclient's message and asks its user for a message to send to chatclient.

And back and forth they go until one of the process sends "\quit," in which case the connection between the two processes is terminated and chatserve continues to listen for other connections.


References
===========
As suggested in the project instructions, Beej's Guide was heavily referenced to implement the socket programming code.
http://beej.us/guide/bgnet/


Testing Machine
=================
The OSU flip server. Two windows were used, one for the client and the other for the server.
