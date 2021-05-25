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
*	parse_file_name:
*
*	parse through a file path to get the file name
*/
char *parse_file_name(char *file_name)
{
	char *buf;
	if ((buf = strstr(file_name, "/")) == NULL)
		return file_name;
	
	do {
        buf = strstr(buf, "/");
        buf++;
    } while (strstr(buf, "/"));

	return buf;
}


/*
* get_file_name_from_send
*
*	get the file name of the file that sender wants to send
*	this is to get a name to save the incoming file as
*/
char *get_filename_from_send(int sockfd)
{
	static char file_name[BUF];
	memset(file_name, 0, BUF);
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
int read_in_file(char *file_name, int sockfd)
{
	static char file_buf[MAX_BUF+1];

	FILE *fp = fopen(parse_file_name(file_name), "a+");
	if (fp == NULL) 
		return 1;

	memset(file_buf, 0, MAX_BUF);
    int n = read(sockfd, file_buf, MAX_BUF);
	if (n < 0)
		return 1;

	for (; n > 0; n = read(sockfd, file_buf, MAX_BUF)) {
		fputs(file_buf, fp);
		memset(file_buf, 0, MAX_BUF);
	}
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
		fprintf(stderr, "problem collecting file data\n");
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