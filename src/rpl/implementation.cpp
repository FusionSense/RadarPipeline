using namespace std;
using namespace cv;

// Base class used for other modules
class RadarBlock
{

    int inputsize;
    int outputsize;

    public:
        // Public variables
        uint frame = 0;

        uint* inputframeptr;
        int* inputbufferptr;

        uint lastframe;

        // Public functions
        // Class constructor
        RadarBlock(int size_in, int size_out) : outputbuffer(new int[size_out])
        {   
            inputsize = size_in;
            outputsize = size_out;

            printf("New %s created.\n", typeid(*this).name());
        }

        // Class deconstructor
        ~RadarBlock()
        {
            delete[] outputbuffer; 

            printf("%s destroyed.\n", typeid(*this).name());
        }

        // Sets the input buffer pointer
        void setBufferPointer(int* ptr)
        {
            inputbufferptr = ptr;
        }

        // Sets the input frame pointer
        void setFramePointer(uint* ptr)
        {
            inputframeptr = ptr;
            lastframe = *ptr;
        }

        // Retrieve outputbuffer pointer
        int* getBufferPointer()
        {
            return outputbuffer;
        }

        // Retrieve frame pointer
        uint* getFramePointer()
        {
            return &frame;
        }

        // Complete desired calculations / data manipulation
        virtual void process()
        {
            printf("Process done!\n");
        }

        // Iterates
        void iteration()
        {
            for(;;)
            {
                listen();
                process();
                increment_frame();
            }
        }

    private:
        // Private variables
        int* outputbuffer;

        // Private functions
        // Listens for previous block (overwritten in some cases)
        void listen()
        {
            for(;;)
            {
                if(*inputframeptr != lastframe)
                {
                    lastframe = *inputframeptr;
                    break;
                }
            }
        }

        // Increments frame count
        void increment_frame()
        {
            frame++;
        }
};

// Visualizes range-doppler data
class Visualizer : public RadarBlock
{
    // Variables
    int width = 512;
    int height = 64;

    int px_width = 2;
    int px_height = 10;

    public:
        Visualizer(int size_in, int size_out) : RadarBlock(size_in, size_out), 
            image(px_height * height, px_width * width, CV_8UC1, Scalar(255))
        {
            namedWindow("Image", WINDOW_OPENGL);
        }

        // Visualizer's process
        void process() override
        {
            // 
            for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                    for(int x = 0; x < px_width; x++) {
                        for(int y = 0; y < px_height; y++) {
                            image.at<uint8_t>(px_height * j + y, px_width * i + x) = inputbufferptr[width * j + i];
                        }
                    }
                }
            }

            // Convert the matrix to a color image for visualization
            Mat colorImage;
            applyColorMap(image, colorImage, COLORMAP_JET);
            
            // Display the color image
            imshow("Image", colorImage);

            // Waits 1ms
            waitKey(1);
        }

    private:
        Mat image;
        
};

////// Data transfer //////

// Calculates speed of incoming data
void calc_speed(int connfd)
{
    uint8_t buff[CUBE];
    uint8_t temp_buff[CUBE];

    bzero(buff, sizeof(buff));
    bzero(temp_buff, sizeof(buff));

    clock_t start = clock();
    clock_t end;

    float mbps;
    float timer;
    int status;
    
    for (;;) {
        // Reads from the TCP connection
        status = recv(connfd, temp_buff, sizeof(buff), 0);

        // Checks to see if the packet is new
        if(memcmp(temp_buff, buff, sizeof(buff)) != 0)
        {
            // End timer
            end = clock();

            memcpy(buff, temp_buff, sizeof(buff));
            timer = (float)(end - start) / CLOCKS_PER_SEC;
            mbps = (float)sizeof(buff) * 8 / 1048576 / (float)timer;
            printf("From client: %.1f KB \t %.6fs \t %.3f Mbps\n", (float)status / 1024, timer, mbps);
            
            //Start new timer
            start = clock();
        }
    }
}

// Sends random packets of data of size CUBE
void send_rand(int sockfd)
{
    uint8_t buff[CUBE];
    bzero(buff, sizeof(buff));

    for (;;) {

        // Generate random data packets
        for (int f = 0; f < CUBE ;f++) 
        {
            buff[f] = rand() / 128;
        }

        send(sockfd, buff, sizeof(buff), 0);
    }
}

////// Connection management //////

// Starts the server connection
tuple<int,int> host()
{
    int sockfd, connfd;
    socklen_t len;
    struct sockaddr_in servaddr, cli;
   
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(TCP_PORT);
   
    // Binding newly created socket to given IP and verification
    if ((::bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
   
    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);
   
    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");

        return make_tuple(sockfd, connfd);
}

// Connects to the host server
int connect()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
 
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
 
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(TCP_PORT);
 
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    return sockfd;
}
