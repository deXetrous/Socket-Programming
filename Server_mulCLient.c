/*
	./output_filename portno
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>                       // to write read(), close(), write()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char* msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		fprintf(stderr, "Port no not provided..");
		exit(1);
	}
	int sockfd, portno, n,numberClients;
	char buffer[255];

	printf("Enter no of clients ");
	scanf("%d",&numberClients);

	int newsockfd[numberClients];

	struct sockaddr_in serv_addr, cli_addr[numberClients];
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

	listen(sockfd, numberClients);
	

	for(int i=0;i<numberClients;i++)
	{
		clilen = sizeof(cli_addr[i]);
		newsockfd[i] = accept(sockfd, (struct sockaddr*) &cli_addr[i], &clilen);

		if(newsockfd[i] < 0)
			error("Error on accept");
	}
	

	while(1)
	{
		bzero(buffer, 255);                   // Clears the memory pointed by buffer
		for(int i=0;i<numberClients;i++)
		{
			n = read(newsockfd[i], buffer, 255);
			if(n < 0 )
				error("Error on read");
			printf("Client : %s\n",buffer);

			bzero(buffer, 255);
		}
		
		fgets(buffer, 255, stdin);
		for(int i=0;i<numberClients;i++)
		{
			n = write(newsockfd[i], buffer, strlen(buffer));
		}
		if(n < 0 )
			error("Error on writing");
		int i = strncmp("Bye", buffer, 3);
		if(i == 0 )
			break;
	}

	for(int i=0;i<numberClients;i++)
		close(newsockfd[i]);
	close(sockfd);
	return 0;

}