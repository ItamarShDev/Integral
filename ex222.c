/*
 * program2.c
 *
 *  Created on: May 10, 2014
 *      Author: itamar
 */


//IO includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// shell includes
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//signals includes
#include <signal.h>
//socket
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#define SIZE 256
void RunSockets(int port);
int main(int argc, char *argv[])
{

	int portno;
	if(argc<2)// case didnt entered the port
	{
		perror("need the port as argument");
		exit(1);
	}
	portno= atoi(argv[1]);
	RunSockets(portno);
	fputs("\nSession Finished\n", stdout);
	return EXIT_SUCCESS;
}
/*
 * opens up the sockets
 * then getting input in loop and sending to the server
 * to get opt of the loop the input must be "quit" or "exit"
 */
void RunSockets(int port)
{
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[SIZE];
	while(1)
	{
		//Opens the main socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
			perror("ERROR opening socket");

		server = gethostbyname("localhost");


		if (server == NULL) {
			fprintf(stderr,"ERROR, no such host\n");
			exit(0);
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;

		bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

		serv_addr.sin_port = htons(port);

		//connecting to server with the same socket and port
		if (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
			perror("ERROR connecting");
		bzero(buffer,SIZE);
		printf("Insert a Polynomial to calculate its integral: \n");
		fgets(buffer,SIZE, stdin);
		//sending the digit to server
		n = write(sockfd, buffer, SIZE);
		if (n < 0)
			perror("ERROR writing to socket");
		if(strncmp(buffer,"quit", 4)==0||strncmp(buffer,"exit",4)==0)
				break;

	}
	close(sockfd);
}
