# CS118 Project 2

Done by Richard Min and Joanne Park

Student-ids: 604-451-118 and 104450395

# Notes
The server IP is automatically assigned. The server will output this number to stderr. 
The format of the custom TCP header is: 

	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|        Sequence Number        |     Acknowledgment Number     |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|             Window            |          Not Used       |A|S|F|
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

This project is mandatorily dependent on gcc-5 to compile and run properly: vagrant should provision this through our vagrant file.

There are two clients: one supports buffering and one doesn't. 
./client 3000 10.0.0.3
./server 3000 large.txt
would be how we tested: 10.0.0.3 is assigned to the server typically, as per our vagrant specifications (and it's the IP for the server.)

The windows inflate and slide for the server, as well as using TCP cwnd/ssthresh (and printing the results of this)