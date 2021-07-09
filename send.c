#include "tinyfs.h"

#include <assert.h>
#include <getopt.h>
#include <libgen.h>  // basename()

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
*   attempt to read a file into a string and define its size
*
*   on success, a pointer to the string containing the file contents is returned
*   otherwise, NULL is returned
*/
char *get_file(char *file, int *size)
{
    FILE *fp;
    int length;
    char *buf = {0};

    fp = fopen(file, "rb");
    if (fp == NULL) {
        perror("fopen:");
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    *size = length;

    buf = malloc(length+1 * sizeof(char));
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
*   wait and attepmt to connect to a client
*
*	returns client socket file descriptor on success.
*	on failire, returns -1   
*/
int client_conn(int sockfd, char *hostname)
{
    int clientfd;
    struct sockaddr_storage client_addr;
	socklen_t len;

    printf("waiting for receiver to connect...\n");

    len = sizeof client_addr;
    clientfd = accept(sockfd, (SA*)&client_addr, &len);
    if (clientfd < 0) {
        perror("accept:");
        return -1;
    } 
    return clientfd;
}





/*
* sned_file
*
*   send a file and it's name through a socket
*
*   returns 0 on success, 1 if file name fails to send, 2 if file failes to send
*/
int send_file(int socket, char *file, int file_size, char *file_name)
{
    /* first let's send through the file name */

    // get basename of file
    char *file_base = basename(file_name);
   
    if (send(socket, file_base, strlen(file_base), 0) < 0) {
        fprintf(stderr, "problem sending file name\n");
        return 1;
    }

    //file_data = get_file(file, &file_size);
    printf("file name: %s\nfile size: %d bytes\n", file_base, file_size);

    if (send(socket, file, file_size, 0) < 0) {
        fprintf(stderr, "problem sending file data\n");
        return 2;
    }
    return 0;
}


void print_usage(int return_code);

int main(int argc, char **argv)
{
    int opt;
    char *hostname = NULL;
    char *file = NULL;
    int file_size;
    char *file_name;
    int port = 3490;
    int sockfd;


    while ((opt = getopt(argc, argv, "h:f:p:u")) != -1) {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;
        case 'f':
            file = get_file(optarg, &file_size);
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

    int client_sock = client_conn(sockfd, hostname);
    assert(client_sock > -1);

    int n = send_file(client_sock, file, file_size, file_name);
    assert(n == 0);
    printf("sent succesfully\n");
    
    free(file);
    close(sockfd);
    close(client_sock);

    return 0;
}


void print_usage(int return_code)
{
    printf("tinyfs - a tiny terminal fire sharing program\n\n");
    printf("Usage:\nsend [-u] [-h IP_OF_RECV] [-f FILE] [-p PORT]\n");
    printf("\nPort defailts to 3490. -f and -h are required\n");
    exit(return_code);
}
