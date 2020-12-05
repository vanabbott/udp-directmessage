// Author: Van Abbott
// NetID: fabbott

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <chrono>
#include "pg1lib.h"

#define PORT_NUM 41001
#define MAX_BUF 4096

void print_form(char *chcksum, char *time, long long int rtt){
		printf("Checksum sent: %s\n", chcksum);
		printf("Server has successfully received the message at:\n%s", time);
		printf("RTT: %lli microseconds\n", rtt);
}

void usage(int status){
		printf("Usage: ./udpclient host_name port_number message\n");
		exit(status);
}

int main(int argc, char* argv[]){
		FILE *fp;
		struct hostent *hp;
		struct sockaddr_in socket_in;
		char *host, *enc_message;
		char *client_pubKey = getPubKey();
		char server_pubKey[MAX_BUF];
		char buf[MAX_BUF];
		int s, len;
		ssize_t message;
		char chcksum[MAX_BUF];
		char chcksum_cmp[9] = "checksum";
		int port = PORT_NUM;
		char name[MAX_BUF];
		char msg[MAX_BUF];
		
		if(argc<4){
				usage(0);
				exit(-1);		
		}else{
				host = argv[1];
				sscanf(argv[2], "%d", &port);
				bcopy(argv[3], name, strlen(argv[3]));
		}
		
		fp = fopen(name, "r");
		if(fp){
				fread(msg, MAX_BUF, 1, fp);
		}else{
				bcopy(name, msg, strlen(name));
		}
				
		hp = gethostbyname(host);
		if(!hp){perror("Error: unkown host\n"); exit(-1);}

		bzero((char *)&socket_in, sizeof(socket_in));
		socket_in.sin_family = AF_INET;
		bcopy(hp->h_addr, (char *)&socket_in.sin_addr, hp->h_length);
		socket_in.sin_port = htons(port);
		if((s = socket(PF_INET, SOCK_DGRAM, 0)) <0){perror("Error: socket"); exit(-1);}	
		socklen_t addr_length = sizeof(socket_in);
		
		//Send the clients public key to server
		message = sendto(s, client_pubKey, strlen(client_pubKey)+1, 0, (struct sockaddr *)&socket_in, sizeof(struct sockaddr));
		if(message<0){perror("Error: sending public key\n"); exit(-1);}
		
		//Wait for the server's encrypted public key
		message = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&socket_in, &addr_length);
		if(message < 0){perror("Error: receiving public key\n"); exit(-1);}
		
		//Decrypt public key and store it
		bcopy(decrypt(buf), server_pubKey, strlen(buf));
		bzero((char *)&buf, sizeof(buf));
		
		//Encrypt the message and send it to server
		enc_message = encrypt(msg, server_pubKey);
		len = strlen(enc_message) + 1;
		message = sendto(s, enc_message, len, 0, (struct sockaddr *)&socket_in, sizeof(struct sockaddr));
		auto start = std::chrono::high_resolution_clock::now();
		if(message < 0){perror("Error: sending message\n"); exit(-1);}
		
		//calculate checksum of message and send it to the server
		sprintf(chcksum, "%lu", checksum(msg));
		len = strlen(chcksum)+1;
		message = sendto(s, chcksum, len, 0, (struct sockaddr *)&socket_in, sizeof(struct sockaddr));
		if(message < 0){perror("Error: sending checksum\n"); exit(-1);}
			
		//wait for response of timestamp from server
		message = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&socket_in, &addr_length);
		if(message < 0){perror("Error: receiving timestamp\n"); exit(-1);}

		if(strcmp(buf, chcksum_cmp)!=0){
				auto rtt = std::chrono::high_resolution_clock::now()-start;
				long long microseconds= std::chrono::duration_cast<std::chrono::microseconds>(rtt).count();
				print_form(chcksum, buf, microseconds);
		}else{
				perror("Error: corrupted data (checksums didn't match)\n");
				exit(-1);
		}

		
		close(s);
		return 0;
}
