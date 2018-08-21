/*
	 g++ Server_nonStopMssg.cpp -std=c++11 -lpthread -o Server_nonStopMssg
	./output_filename portno
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>                       // to write read(), close(), write()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <thread>

using namespace std;

void error(const char* msg)
{
	perror(msg);
	exit(1);
}

void reading(int newsockfd)
{
	while(true)
	{
		char buffer[255];
		bzero(buffer, 255);                   // Clears the memory pointed by buffer
		int n = read(newsockfd, buffer, 255);
		if(n < 0 )
			error("Error on read");
		printf("Client : %s\n",buffer);

		bzero(buffer, 255);
	}
	
}

void writing(int newsockfd)
{
	while(true)
	{
		char buffer[255];
		fgets(buffer, 255, stdin);
		int n = write(newsockfd, buffer, strlen(buffer));
		if(n < 0 )
			error("Error on writing");
		int i = strncmp("Bye", buffer, 3);
		if(i == 0 )
			break;

	}
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		fprintf(stderr, "Port no not provided..");
		exit(1);
	}
	int sockfd, newsockfd, portno, n;
	char buffer[255];

	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		error("error opening socket.");
	}

	bzero((char*) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		error("Binding failed");

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);

	if(newsockfd < 0)
		error("Error on accept");


	thread r(reading, newsockfd);	
	thread w(writing, newsockfd);
	r.join();
	w.join();
		

	close(newsockfd);
	close(sockfd);
	return 0;

}