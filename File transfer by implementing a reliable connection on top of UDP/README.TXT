EXECUTION INSTRUCTIONS:

PLEASE FOLLOW THIS ORDER:
 1)gcc -o server server.c
 2)./server
 3)gcc -o relay1 relay1.c
 4)./relay1
 5)gcc -o relay2 relay2.c
 6)./relay2
 7)gcc -o client client.c
 8)./client

Details:
* The issue of retransmission of lost packets has been handled with select() system call
* The Relays have been implemented by using two separate files.
* The sequence numbers of data and acknowledgements are embedded in the packet itself.
* The log will be displayed on the respected consoles
* Please find all the constants in packet.h file
* The out of order packets also have been taken care of properly.
*The code will take the input from input.txt file in the current directory and produce output.txt file in the current directory.
