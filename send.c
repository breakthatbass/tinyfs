#include "tinyfs.h"

#include <assert.h>
#include <getopt.h>

/**
* run_server
*
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

	// bind the socket to the PORT
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
* DESCRIPTION
*   attempt to read a file into a string
*
* RETURN VALUE
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

    // read the entire file into buffer
    fread(buf, 1, length, fp);
    buf[length] = '\0';

    //printf("%s\n", buf);
   
    fclose(fp);

    return buf;
}


char *get_client_ip(struct sockaddr_storage client_addr)
{
    struct sockaddr_in *client_ip;
    struct in_addr ip;
    static char s[INET_ADDRSTRLEN];

    client_ip = (struct sockaddr_in*)&client_addr;
    ip = client_ip->sin_addr;

    inet_ntop(AF_INET, &ip, s, INET_ADDRSTRLEN);
    return s;
}


int client_conn(int sockfd, char *file, char *hostname)
{
    int clientfd;
    struct sockaddr_storage client_addr;
	socklen_t len;

     /* wait until client connects */
    int status;
    int connected = 0; /* not connect to client yet */

    while (1) {

        if (!connected) {

            len = sizeof client_addr;
            clientfd = accept(sockfd, (SA*)&client_addr, &len);
            if (clientfd < 0) {
                perror("accept:");
                continue;

            } else {
                char *ip = get_client_ip(client_addr);

                /* make sure the connected client is not an imposter! */
                if (strcmp(hostname, ip) != 0) {
                    fprintf(stderr, "An imposter has connected with the address of %s!!\n", ip);
                    printf("Disconnecting!!\n");
                    exit(EXIT_FAILURE);
                }
                
                connected = 1;

                if (fork() == 0) {

                    if (send(clientfd, file, strlen(file), 0) < 0) {
                        perror("send:");
                    } 
                    printf("file sent successfully\n");
                } 
            }
        } else {
            wait(&status);
            return 0;
        }
    }
    return 1;
}



int main(int argc, char **argv)
{
    /* getopt stuff */
    int opt;
    char *hostname = NULL;
    char *file = NULL;
    int port = 3490;
    int sockfd;


    while ((opt = getopt(argc, argv, "h:f:p:")) != -1) {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;
        case 'f':
            file = get_file(optarg);
            break;
        case 'h':
            hostname = optarg;
            break;
        default:
            fprintf(stderr, "%c not recofgnized\n", opt);
            exit(1);
        }
    }

    /* we need a hostname and file */
    if (file == NULL || hostname == NULL) {
        //print_usage();
        exit(EXIT_FAILURE);
    }

    sockfd = run_server(port);
    if (sockfd < 0) {
        fprintf(stderr, "problem setting up server\n");
        exit(EXIT_FAILURE);
    }
    printf("server running...\n");

    int n = client_conn(sockfd, file, hostname);

    free(file);
    close(sockfd);
    return 0;
}