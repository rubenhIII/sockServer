#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define SRVPORT "9000"
#define BACKLOG 5
#define FILE_PATH "/var/tmp/aesdsocketdata"
#define DEFAULT_BUFFER_SIZE 20

#define OPT_FLAG "-d"

void buffer_stream(int, int, int);
void exit_handler(int);

char input_stream;
int input_stream_counter = 0;
int buffer_size = DEFAULT_BUFFER_SIZE;
char *file_buffer;

struct sockaddr_in their_addr;
struct sockaddr_in client_addr;
socklen_t addrsize, client_addr_size;
int status, sockfd, newfd, fd;
struct addrinfo hints;
struct addrinfo *servinfo;
struct sigaction sigaction_exit;

char opt_flag[] = OPT_FLAG;
int pid = 0;

int main(int argc, char **argv)
{
	if (argc > 1 && strncmp(argv[1], opt_flag, 2) == 0) {
		syslog(LOG_INFO, "Running server as daemon");
		pid = fork();
	}

	switch (pid) {
		case -1:
			syslog(LOG_ERR, "Fork failure");
			return -1;
		case 0:
			memset(&sigaction_exit, 0, sizeof(sigaction_exit));
			sigaction_exit.sa_handler = exit_handler;

			openlog("aesdsocket_server", LOG_CONS | LOG_NDELAY | LOG_PID, LOG_LOCAL1);
			sigaction(SIGINT, &sigaction_exit, NULL);
			sigaction(SIGTERM, &sigaction_exit, NULL);

			fd = open(FILE_PATH, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
			if (fd < 0) {
				syslog(LOG_ERR, "Unable to open file");
				return -1;
			}

			file_buffer = (char *) malloc(sizeof(char) * buffer_size);
			if (file_buffer < 0) {
				syslog(LOG_ERR, "Stream buffer allocation failed!");
				return -1;
			}

			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_PASSIVE;

			if ((status = getaddrinfo(NULL, SRVPORT, &hints, &servinfo)) != 0) {
				syslog(LOG_ERR, "getaddrinfo error: %s", strerror(errno));
				return -1;
			}

			int option = 1;
			sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
			setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

			int bind_result = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
			if (bind_result < 0) {
				syslog(LOG_ERR, "Error server binding: %s", strerror(errno));
				return -1;
			}
			int listen_result = listen(sockfd, BACKLOG);
			if (listen_result < 0) {
				syslog(LOG_ERR, "Error server listening to: %s", strerror(errno));
				return -1;
			}
	
			while (1) {
				addrsize = sizeof(their_addr);
				newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addrsize);
				if (newfd < 0) {
					syslog(LOG_ERR, "Error accepting connection!");
					return -1;
				}

				client_addr_size = sizeof(client_addr);
				getpeername(newfd, (struct sockaddr *)&client_addr, &client_addr_size);
				syslog(LOG_INFO, "Accepted connection from %s", inet_ntoa(client_addr.sin_addr));

				input_stream_counter = 0;
				// Loop for send and receive data from stream
				buffer_stream(1, newfd, fd);

				shutdown(newfd, SHUT_RDWR);
				close(newfd);
				syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(client_addr.sin_addr));
			}
		default:
			syslog(LOG_INFO, "Server running background");
			return 0;
	}

	return 0;
}

void buffer_stream(int input_src, int newfd, int fd)
{
	if (!input_src) {
		if (lseek(fd, 0, SEEK_SET) < 0) {
			syslog(LOG_ERR, "Failed to seek to the begining");
			exit(-1);
		}
	}
	while ((input_src > 0 ? recv(newfd, &input_stream, 1, 0) : read(fd , &input_stream, 1)) > 0) {
		//if (input_src)
		//	printf("Message received: %c\n", input_stream);
		if (input_stream_counter < buffer_size) {
			file_buffer[input_stream_counter] = input_stream;
			input_stream_counter++;
		} else {
			buffer_size = buffer_size + 10;
			file_buffer = (char *) realloc(file_buffer, sizeof(char) * (buffer_size));
			if (file_buffer < 0) {
				syslog(LOG_ERR, "Stream buffer reallocation failed!");
				exit(-1);
			}
			syslog(LOG_INFO, "Stream buffer reallocation succeeds!");
			file_buffer[input_stream_counter] = input_stream;
			input_stream_counter++;
		}
		if (input_stream == '\n') {
			input_src ? write(fd, file_buffer, input_stream_counter) : send(newfd, file_buffer, input_stream_counter, 0);
			input_stream_counter = 0;
			buffer_size = DEFAULT_BUFFER_SIZE;
			file_buffer = (char *) realloc(file_buffer, sizeof(char) * buffer_size);
			if (file_buffer < 0) {
				syslog(LOG_ERR, "Stream buffer reallocation after write failed!");
				exit(-1);
			}
			if (input_src)
				buffer_stream(0, newfd, fd);
		}
	}
}

void exit_handler(int param)
{
	syslog(LOG_INFO, "Caught signal, exiting");
	close(fd);
	if (unlink(FILE_PATH) < 0) {
		syslog(LOG_ERR, "Error deleting the file");
	}
	freeaddrinfo(servinfo);
	close(sockfd);
	closelog();
	exit(0);
}
