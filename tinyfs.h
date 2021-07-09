
#ifndef COMMON_H__
#define COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <getopt.h>
#include <assert.h>

#define MAXDATA 300000
#define MAX_BUF 500
#define BUF 25
#define SA struct sockaddr		// for less messy casting

#endif
