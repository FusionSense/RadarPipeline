#include <arpa/inet.h> // inet_addr()
#include <stdio.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <tuple>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
// #include <utility>
#include SERVER_PARAMS
#include CLIENT_PARAMS
#define SA struct sockaddr