#include "tinyfs.h"

#include <assert.h>
#include <getopt.h>

/*
* run_server:
*
*   attempt to connect to a socket, bind it, and listen
*  
*   returns socket file descriptor on success. a negative number on failure
*/
int run_server(int port)
{
    int sockfd;
    struct sockaddr_in servaddr;

	memset(&servaddr, 0, sizeof servaddr);
	servaddr.sin_family = AF_INET;	        /* IPv4 for simplicity */
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		return -1;
	}

	if (bind(sockfd, (SA*)&servaddr, sizeof servaddr) < 0) {
		perror("bind");
		return -2;
	}

	if (listen(sockfd, 10) < 0) {
		perror("listen");
		return -3;
	}
    return sockfd;
}

/**
* get_file:
*
*   attempt to read a file into a string
*
*   on success, a pointer to the string containing the file contents is returned
*   otherwise, NULL is returned
*/
char *get_file(char *file)
{
    FILE *fp;
    long length;
    char *buf = {0};

    fp = fopen(file, "r");
    if (fp == NULL) {
        perror("fopen:");
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buf = calloc(length+2, sizeof(char));
    if (buf == NULL) return NULL;

    fread(buf, 1, length, fp);
    buf[length] = '\0';
   
    fclose(fp);

    return buf;
}


/*
* get_client_ip
*
*   get the ip address of a connected client
*
*   returns the address in string form, or NULL if it fails to retrieve it
*/
char *get_client_ip(struct sockaddr_storage client_addr)
{
    struct sockaddr_in *client_ip;
    struct in_addr ip;
    static char s[INET_ADDRSTRLEN];

    client_ip = (struct sockaddr_in*)&client_addr;
    ip = client_ip->sin_addr;

    if (inet_ntop(AF_INET, &ip, s, INET_ADDRSTRLEN) == NULL)
        return NULL;
    return s;
}

/*
* client_conn
*
*   wait until a client connects. if it, attempt to send the file.
*
*   
*/
int client_conn(int sockfd, char *file, char *hostname, char *file_name)
{
    int clientfd;
    struct sockaddr_storage client_addr;
	socklen_t len;
    int status;

    printf("waiting for receiver to connect...\n");

    len = sizeof client_addr;
    clientfd = accept(sockfd, (SA*)&client_addr, &len);
    if (clientfd < 0) {
        perror("accept:");
        return 1;

    } else {
        char *ip = get_client_ip(client_addr);

        /* make sure the connected client is not an imposter */
        if (strcmp(hostname, ip) != 0) {
            fprintf(stderr, "An imposter has connected with the address of %s!!\n", ip);
            printf("Disconnecting!\n");
            exit(EXIT_FAILURE);
        }

        /* first let's send through the file name */
        if (send(clientfd, file_name, strlen(file_name), 0) < 0) {
            fprintf(stderr, "problem sending file name\n");
            exit(EXIT_FAILURE);
        }

        if (fork() == 0) {

            if (send(clientfd, file, strlen(file), 0) < 0) {
                fprintf(stderr, "error sending file\n");
                exit(EXIT_FAILURE);
            } 
            printf("file has been sent to %s\n", ip);
            exit(EXIT_SUCCESS);
        }
    }
    return 0;
}


void print_usage(int return_code);

int main(int argc, char **argv)
{
    int opt;
    char *hostname = NULL;
    char *file = NULL;
    char *file_name;
    int port = 3490;
    int sockfd;


    while ((opt = getopt(argc, argv, "h:f:p:u")) != -1) {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;
        case 'f':
            file = get_file(optarg);
            file_name = optarg;
            break;
        case 'h':
            hostname = optarg;
            break;
        case 'u':
            print_usage(EXIT_SUCCESS);
        default:
            fprintf(stderr, "%c not recofgnized\n", opt);
            exit(EXIT_FAILURE);
        }
    }

    /* we need a hostname and file */
    if (file == NULL || hostname == NULL)
        print_usage(EXIT_FAILURE);

    sockfd = run_server(port);
    if (sockfd < 0) {
        fprintf(stderr, "problem setting up server\n");
        exit(EXIT_FAILURE);
    }

    int n = client_conn(sockfd, file, hostname, file_name);
    
    free(file);
    close(sockfd);

    if (n) /* problem with connecting to client */
        return 1;
    return 0;
}


void print_usage(int return_code)
{
    printf("tinyfs - a tiny terminal fire sharing program\n\n");
    printf("Usage:\nsend [-u] [-h IP_OF_RECV] [-f FILE] [-p PORT]\n");
    printf("\nPort defailts to 3490. -f and -h are required\n");
    exit(return_code);
}