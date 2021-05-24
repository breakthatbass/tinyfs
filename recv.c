#include "tinyfs.h"


int serv_conn(struct addrinfo *servinfo)
{
    int sockfd;
    struct addrinfo *p;
    char s[INET6_ADDRSTRLEN];

    // loop trhough all results and connect to the first one we can
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
		// if we make it here, we've got a connection
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
void save_to_file(char *s, char *file_name)
{
    printf("%s", s);
    FILE *fp;
    fp = fopen(file_name, "a");
    fputs(s, fp);
    fclose(fp);
}
*/




/* usage: recv <ip_address> <port> */

int main(int argc, char **argv)
{
    int sockfd, opt, rv;
	struct addrinfo hints, *servinfo;
	char file_buf[MAXDATA];
	char port[5] = "3490";

	//assert(argc == 2);

	while ((opt = getopt(argc, argv, "p:")) != -1) {
		switch(opt) {
		case 'p':
			memset(port, 0, 5);
			strcpy(port, optarg);
			break;
		}
	}

    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// load the struct
	if ((rv = getaddrinfo(argv[1], port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(EXIT_FAILURE);
	}

    sockfd = serv_conn(servinfo);

	FILE *fp = fopen("testfile.txt", "a+");
	assert(fp);

	int MAX_BUF = 50;
	memset(file_buf, 0, MAX_BUF);
    int n = read(sockfd, file_buf, MAX_BUF);

	for (; n > 0; n = read(sockfd, file_buf, MAX_BUF)) {
		file_buf[n] = '\0';
		fputs(file_buf, fp);
		//fprintf(fp, "%s", file_buf);
		memset(file_buf, 0, MAX_BUF);
	}
    fclose(fp);

    close(sockfd);
    return 0;
}