# udp-directmessage
The programs I wrote for this assignment are udpclient.cpp and udpserver.cpp.
To compile both the programs type "make all" into the command line with the Makefile in the cwd
To start the server simply type "./udpserver" in the command line.
To connect to the server with the client program from a different machine type "./udpclient {host name} {port number} {message/file}"
An example of this would be "./udpclient student05.cse.nd.edu 41001 "hello world" or "./udpclient student05.cse.nd.edu 41001 File1.txt"
