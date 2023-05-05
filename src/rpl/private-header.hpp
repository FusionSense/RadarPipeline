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
#include <complex>
#include <cmath>
#include <fftw3.h>
#include <fstream>
#include SERVER_PARAMS
#include CLIENT_PARAMS
#define SA struct sockaddr

#include "implementation.cpp"

#include <iostream>
#include <thread>

#define FAST_TIME 9
#define SLOW_TIME 2
#define RX 4
#define TX 3
#define IQ 2
#define SIZE_W_IQ TX*RX*FAST_TIME*SLOW_TIME*IQ
#define SIZE TX*RX*FAST_TIME*SLOW_TIME