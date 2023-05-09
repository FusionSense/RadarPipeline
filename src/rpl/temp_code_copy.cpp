#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>

#define BUFFER_SIZE 4096 
#define PORT        4098
#define BYTES_IN_PACKET 1456 // Max packet size - sequence number and byte count = 1466-10 
#define NUM_RX 4
#define NUM_TX 3
#define FAST_TIME 512 //ADC Samples
#define SLOW_TIME 64 //Chirps
#define IQ_DATA 2 //Types of IQ (I and Q)
#define IQ_BYTES 2 


void read_udp_packets(uint16_t* frame_data);
int save_1d_array(uint16_t* arr, int width, int length, const char* filename);

int main()
{
    uint16_t frame_data[NUM_RX*NUM_TX*FAST_TIME*SLOW_TIME*IQ_DATA]={0};
    read_udp_packets(frame_data);

    save_1d_array(frame_data, FAST_TIME*NUM_TX*NUM_RX*IQ_DATA, SLOW_TIME, "./out.txt");

    return 0;
}

int save_1d_array(uint16_t* arr, int width, int length, const char* filename) {
    std::ofstream outfile(filename);
    for (int i=0; i<length*width; i++) {
        outfile << arr[i] << std::endl;
    }

    //outfile.close();
    std::cout << "Array saved to file. " << std::endl;
    return 0;
}

void read_udp_packets(uint16_t* frame_data) {

    // Variable initialization
    uint64_t BYTES_IN_FRAME = SLOW_TIME*FAST_TIME*NUM_RX*NUM_TX*IQ_DATA*IQ_BYTES;
    uint64_t BYTES_IN_FRAME_CLIPPED = BYTES_IN_FRAME/BYTES_IN_PACKET*BYTES_IN_PACKET;
    uint16_t PACKETS_IN_FRAME_CLIPPED = BYTES_IN_FRAME / BYTES_IN_PACKET;
    uint16_t UINT16_IN_PACKET = BYTES_IN_PACKET / 2; //728 entries in packet
    uint64_t UINT16_IN_FRAME = BYTES_IN_FRAME / 2;

    uint64_t packets_read = 0;

    // socket file descriptor
    int sockfd; 
    // create buffer array of preset size to hold one packet
    char buffer[BUFFER_SIZE];
    // initialize socket
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    // Create a UDP socket file descriptor which is UNbounded
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) <  0){
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 

    // Filling in the servers (DCA1000EVMs) information
    servaddr.sin_family     = AF_INET;      //this means it is a IPv4 address
    servaddr.sin_addr.s_addr= INADDR_ANY;   //sets address to accept incoming messages
    servaddr.sin_port       = htons(PORT);  //port number to accept from
    
    // Now we bind the socket with the servers (DCA1000EVMs) address 
    // if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 

    // Bind the socket to any available IP address and a specific port
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    // n is the packet size in bytes (including sequence number and byte count)
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
    buffer[n] = '\0'; // Null-terminate the buffer

    uint32_t starting_sequence_num = ((buffer[0] & 0xFF) << 0)  |
                                ((buffer[1] & 0xFF) << 8)  |
                                ((buffer[2] & 0xFF) << 16) |
                                ((long) (buffer[3] & 0xFF) << 24);
    //std::cout << "First Sequence Number = " << starting_sequence_num << std::endl;

    int byte_count_old = 0;
    
    // Receive UDP packets and save the data to a buffer

    while (true) {
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);

        buffer[n] = '\0'; // Null-terminate the buffer

        // first four bytes from buffer (the sequence number)
        uint32_t packet_num = ((buffer[0] & 0xFF) << 0)  |
                                ((buffer[1] & 0xFF) << 8)  |
                                ((buffer[2] & 0xFF) << 16) |
                                ((long) (buffer[3] & 0xFF) << 24);
        // print packet number (packet count from DCA data) -> misses packets
        //std::cout << "Packet Number = " << packet_num << std::endl;

        //unpack byte count
        uint64_t byte_count = ((buffer[4] & 0xFF) << 0)  |
                                ((buffer[5] & 0xFF) << 8)  |
                                ((buffer[6] & 0xFF) << 16) |
                                ((buffer[7] & 0xFF) << 24) |
                                ((unsigned long long) (buffer[8] & 0xFF) << 32) |
                                ((unsigned long long) (buffer[9] & 0xFF) << 40) |
                                ((unsigned long long) (0x00) << 48) |
                                ((unsigned long long) (0x00) << 54);

        // print byte count from DCA data
        //std::cout << "Byte Count = " << byte_count << std::endl;
 

        std::cout << "Packet Number: " << packet_num << "| Bytes since last packet = " << byte_count - byte_count_old << std::endl;
        byte_count_old = byte_count;
        //uint16 data 
        uint16_t packet_data[UINT16_IN_PACKET] = {0};
        //Read packet data from buffer (limit to size of packet data rather than buffer size)
        for (int i = 0; i< sizeof(packet_data)/2; i++)
        {
            packet_data[i] =  buffer[2*i+10] | (buffer[2*i+11] << 8);
        }
        //Do something with the data, e.g., save it to a file or process it in some way

        //Add packet_data to ret_frame
        for (int i = UINT16_IN_PACKET*packets_read; i < (UINT16_IN_PACKET*(packets_read+1)); i++)
        {
            frame_data[i] = packet_data[i%UINT16_IN_PACKET];
        }
        packets_read++;
        uint64_t byte_mod = (packets_read*BYTES_IN_PACKET) % BYTES_IN_FRAME_CLIPPED;

        if (byte_mod == 0) //end of frame found
        {
            //int count = packet_num-starting_sequence_num;
            std::cout << "End of frame found" << std::endl;
            std::cout << "Size of frame_data " << sizeof(frame_data)<< std::endl;
            return;           
        }
    }
    // Close the socket
    close(sockfd);
}

