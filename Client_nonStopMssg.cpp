/*
		g++ Client_nonStopMssg.cpp -std=c++11 -lpthread -o Client_nonStopMssg
		./output_filename server_ipaddress portno 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>                     // defines the hostent structure
#include <iostream>
#include <thread>

using namespace std;

void error(const char* msg)
{
	perror(msg);
	exit(1);
}

void reading(int sockfd)
{
	while(true)
	{
		char buffer[255];
		bzero(buffer, 255);
		int n = read(sockfd, buffer, 255);
		if(n < 0)
			error("Error on reading");
		printf("Server : %s\n", buffer);

		int i = strncmp("Bye", buffer, 3);
		if(i == 0)
			break;
	}
}

void writing(int sockfd)
{
	while(true)
	{
		char buffer[255];
		bzero(buffer, 255);
		fgets(buffer, 255, stdin);
		int n = write(sockfd, buffer, strlen(buffer));
		if(n < 0)
			error("Error on writing");
	}
}


int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent* server;

	char buffer[255];
	if(argc < 3)
	{
		fprintf(stderr, "usage %s hostname port \n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		error("error opening socket");

	server = gethostbyname(argv[1]);
	if(server == NULL)
	{
		fprintf(stderr, "Error, no such host");
	}

	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	// copies bytes of data from hostent server to serv_addr
	bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		error("Connection failed");

	

	
	thread r(reading, sockfd);
		
	thread w(writing, sockfd);
		//printf("hi\n");

	r.join();
	w.join();

	close(sockfd);
	return 0;

}