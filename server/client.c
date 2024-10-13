#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#define SRVPORT "9000"
#define BACKLOG 5
#define BUFFER_SIZE 1024

int main()
{

	openlog("aesdsocket_client", LOG_CONS | LOG_NDELAY | LOG_PID, LOG_LOCAL1);

	int status, sockfd, newfd;
	struct addrinfo hints;
	struct addrinfo *servinfo;

	char buffer[BUFFER_SIZE] = {0};
	char *hello = "Hello World from Client!\n";

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, SRVPORT, &hints, &servinfo)) != 0) {
		syslog(LOG_ERR, "Cannot get address info");
		exit(1);
	}

	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (sockfd < 0)
		syslog(LOG_ERR, "Cannot open client socket");
	int connect_result = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	if (connect_result < 0)
		syslog(LOG_ERR, "Cannot connect client: %s", strerror(errno));

	syslog(LOG_INFO, "Client running!");

	// Send a response to the client
	send(sockfd, hello, strlen(hello), 0);
	//printf("Hello message sent\n");

	while (recv(sockfd, buffer, BUFFER_SIZE, 0) > 0) {
       		printf(" %s", buffer);
	}
	//
	//
	freeaddrinfo(servinfo);
	syslog(LOG_INFO, "Client stop!");

	close(sockfd);
	closelog();

	return 0;
}
