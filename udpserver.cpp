// Author: Van Abbott
// NetID: fabbott

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include "pg1lib.h"


#define PORT_NUM 41001
#define MAX_BUF 4096

bool print_form(char *mes, unsigned long int chcksum, time_t time){
		printf("**********New Connection**********\n");
		printf("Received Time: %s", ctime(&time));
		printf("Received Message: \n%s\n", mes);
		printf("Received Client Checksum: %lu\n", chcksum);
		printf("Calculated Checksum: %lu\n\n", checksum(mes));
		
		return chcksum == checksum(mes);
}

int main(int argc, char* argv[]){
		struct sockaddr_in server_in, client_addr;
		char buf[MAX_BUF];
		socklen_t addr_length; 
		int fd;
		char client_pubKey[MAX_BUF];
		char *server_pubKey = getPubKey();
		char *enc_message;
		char *message;
		ssize_t check;
		unsigned long int chcksum;
		time_t time_rcvd = time(0);

		bzero((char *)&server_in, sizeof(server_in));
		server_in.sin_family = AF_INET;
		server_in.sin_port = htons(PORT_NUM);
		server_in.sin_addr.s_addr = INADDR_ANY;
		printf("Server starting...\n");

		if((fd = socket(PF_INET, SOCK_DGRAM, 0))<0){perror("Error: socket\n"); exit(-1);}
		printf("Server listening on port %d\n", PORT_NUM);
		if((bind(fd, (struct sockaddr *)&server_in, sizeof(server_in)))<0){perror("Error: binding\n"); exit(-1);}
		addr_length = sizeof(client_addr);
		while(true){
				printf("Waiting...\n");
				
				//Wait for public key to be sent from client
				check = recvfrom(fd, client_pubKey, sizeof(client_pubKey), 0, (struct sockaddr *)&client_addr, &addr_length);	
				if(check == -1){perror("Error: receiving public key\n"); exit(-1);}
				
				//Encrypt servers public key and send it back to client
				enc_message = encrypt(server_pubKey, client_pubKey);
				check = sendto(fd, enc_message, strlen(enc_message)+1, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
				if(check < 0){perror("Error: sending public key back\n"); exit(-1);}
				
				//Receive message from client and decrypt it
				check = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_length);	
				if(check == -1){perror("Error: receiving message\n"); exit(-1);}
				time_rcvd = time(0);
				message = decrypt(buf);
				bzero((char *)&buf, sizeof(buf));	
				
				//Recieve checksum of message from client 
				check = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_length);
				if(check == -1){perror("Error: receiving checksum\n"); exit(-1);}
				chcksum = strtoul(buf,NULL, 0);
				bzero((char *)&buf, sizeof(buf));	
				if(print_form(message, chcksum, time_rcvd)){
						check = sendto(fd, ctime(&time_rcvd), strlen(ctime(&time_rcvd))+1, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						if(check < 0){perror("Error: sending timestamp\n"); exit(-1);} 
				}else{
						check = sendto(fd, "checksum", 8, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						if(check < 0){perror("Error: sending checksum failur\n"); exit(-1);}
				}
				
		}
		close(fd);
		return 0;
}
