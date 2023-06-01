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
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <complex>
#include <cmath>
#include <fftw3.h>
#include <fstream>

#define SA struct sockaddr
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include "implementation.cpp"
#include <thread>

#include <iostream>
#include <thread>

#define FAST_TIME 9
#define SLOW_TIME 2
#define RX 4
#define TX 3
#define IQ 2
#define SIZE_W_IQ TX*RX*FAST_TIME*SLOW_TIME*IQ
#define SIZE TX*RX*FAST_TIME*SLOW_TIME

#define BUFFER_SIZE 4096 
#define PORT        4098
#define BYTES_IN_PACKET 1456 // Max packet size - sequence number and
			     // byte count = 1466-10
#define NUM_RX 4
#define NUM_TX 3
#define FAST_TIME 512 //ADC Samples
#define SLOW_TIME 64 //Chirps

#define RANGE_BINS FAST_TIME
#define DOPPLER_BINS SLOW_TIME
#define IQ_DATA 2 //Types of IQ (I and Q)
#define IQ_BYTES 2 
