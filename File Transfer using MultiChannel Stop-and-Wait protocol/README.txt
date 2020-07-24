EXECUTION INSTRUCTIONS:

PLEASE FOLLOW THIS ORDER:
 1)gcc -o server server.c
 2)./server
 3)gcc -o client client.c
 4)./client

Details:
* The issue of retransmission of lost packets has been handled with select() system call
* The timers also have been implemented partially with select and struct timeval by updating the struct timevals depending on the residue value of the struct timeval passed to select.
* Two sockets have been created to manage two separate connections
* The out of order packets also have been taken care of properly.
* Please find all the constants in packet.h file
*The code will take the input from input.txt file in the current directory and produce output.txt file in the current directory.
