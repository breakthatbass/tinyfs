#include "tinyfs.h"

/*
* client_init:
*
*	set up client with data from host and port.
*
*	returns a linked list
*/
struct addrinfo *client_init(char *host, char *port)
{
	struct addrinfo hints, *f;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(host, port, &hints, &f)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(EXIT_FAILURE);
	}
	return f;
}


/*
*  serv_conn
*	 attempt to open a socket and connect to server
*/
int serv_conn(struct addrinfo *servinfo)
{
    int sockfd;
    struct addrinfo *p;
    char s[INET_ADDRSTRLEN];

    /* loop through all results and connect to the first one we can */
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		/* if we make it here, we've got a connection */
		break;
	}
    if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		exit(EXIT_FAILURE);
	}

	inet_ntop(p->ai_family, (SA*)&p->ai_addr, s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);

    return sockfd;
}



/*
* get_file_name_from_send
*
*	get the file name of the file that sender wants to send
*	this is to get a name to save the incoming file as
*/
char *get_filename_from_send(int sockfd)
{
	static char file_name[BUF] = {0};
	
	int b = read(sockfd, file_name, BUF);
	if (b < 1)
		return NULL;
	return file_name;
}


/*
* read_in_file
*
*	read in file contents from sender 50 bytes at a time
*	and write those bytes to the file.
*	continue reading and writing until no more bytes come through 
*/
#define CHUNK 1096
#define TOTAL 4000000
int read_in_file(char *file_name, int sockfd)
{
	char buf[CHUNK];
	int total_size = 0;
	int n;

	FILE *fp = fopen(file_name, "wb");
	if (fp == NULL) 
		return 1;
	while (1) {
		memset(buf, 0, CHUNK);
		if ((n = recv(sockfd, buf, CHUNK, 0)) < 1) break;
		else {
			total_size += n;
			fwrite(buf, 1, CHUNK, fp);
		}
	}

	printf("file: %s\n", file_name);
	printf("size (bytes): %d\n", total_size);
    fclose(fp);
	return 0;
}


void print_usage(int return_code);

int main(int argc, char **argv)
{
    int sockfd, opt;
	struct addrinfo *servinfo;
	char file_name[BUF];
	char port[5] = "3490";
	char *host;


	while ((opt = getopt(argc, argv, "h:p:u")) != -1) {
		switch(opt) {
		case 'h':
			host = optarg;
			break;
		case 'p':
			memset(port, 0, 5);
			strcpy(port, optarg);
			break;
		case 'u':
			print_usage(EXIT_SUCCESS);
			break;
		default:
			fprintf(stderr, "%c not recognized\n", opt);
			exit(EXIT_FAILURE);
		}
	}

	/* don't allow any ports below 1024 */
	if (atoi(port) < 1024) {
		fprintf(stderr, "Beej says 'ports under 1024 are often considered special...'\n");
		fprintf(stderr, "use port 1024 or above\n");
		exit(EXIT_FAILURE);
	}

    servinfo = client_init(host, port);

    sockfd = serv_conn(servinfo);

	// first lets get file name
	strcpy(file_name, get_filename_from_send(sockfd));
	
	int n = read_in_file(file_name, sockfd);
	if (n == 1) {
		fprintf(stderr, "problem reading file\n");
		exit(EXIT_FAILURE);
	}
	if (n == 2) {
		fprintf(stderr, "problem reading from socket\n");
		exit(EXIT_FAILURE);
	}

    close(sockfd);
    return 0;
}

void print_usage(int return_code)
{
    printf("tinyfs - a tiny terminal fire sharing program\n\n");
    printf("Usage:\nrecv [-u] [-h IP_OF_RECV] [-p PORT]\n");
    printf("\nPort defailts to 3490. -h is required\n");
    exit(return_code);
}
