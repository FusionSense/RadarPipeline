using namespace std;

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

        // Complete desired calculations / data manipulation
        void process()
        {
            printf("Process done!\n");
        }

        // Increments frame count
        void increment_frame()
        {
            frame++;
        }
};

// Processes IQ data to make Range-Doppler map
class RangeDoppler : public RadarBlock
{
    public:
        RangeDoppler(int fast_time, int slow_time, int rx, int tx, int iq)
        {
            FAST_TIME = fast_time;
            SLOW_TIME = slow_time;
            RX = rx;
            TX = tx;
            IQ = iq;
            SIZE = TX*RX*FAST_TIME*SLOW_TIME;
            SIZE_W_IQ = TX*RX*FAST_TIME*SLOW_TIME*IQ;
            auto adc_data_flat = reinterpret_cast<float*>(malloc(SIZE_W_IQ*sizeof(float)));
            auto adc_data_reshaped = reinterpret_cast<float*>(malloc(SIZE_W_IQ*sizeof(float)));
            auto rdm_data = reinterpret_cast<std::complex<float>*>(malloc(SIZE * sizeof(std::complex<float>)));
            auto rdm_norm = reinterpret_cast<float*>(malloc(SIZE * sizeof(float)));
            auto rdm_avg = reinterpret_cast<float*>(calloc(SLOW_TIME*FAST_TIME, sizeof(float)));
        }

        void blackman_window(float* arr, int size){
            for(int i = 0; i<size; i++)
                arr[i] = 0.42 - 0.5*cos(2*M_PI*i/(size-1))+0.08*cos(4*M_PI*i/(size-1));
        }

        void readFile(const std::string& filename, float* arr, int size) {
            std::ifstream file(filename);
            if (file.is_open()) {
                std::string line;
                
                int i = 0;
                while (std::getline(file, line)) {
                    if(i > size){
                        std::cerr << "Error: More samples than SIZE " << filename << std::endl;
                        break;
                    }
                    float value = std::stof(line);
                    arr[i] = value;
                    i++;
                }
                std::cout << "File Successfully read!" << std::endl;
                file.close();
            } else {
                std::cerr << "Error: Could not open file " << filename << std::endl;
            }
        }

        int save_2d_array(float* arr, int width, int length, const char* filename) {
            std::ofstream outfile(filename);
            for (int i=0; i<length; i++) {
                for (int j=0; j<width; j++) {
                outfile << arr[i*width+j] << " ";
                }
                outfile << std::endl;
            }

            //outfile.close();
            std::cout << "Array saved to file. " << std::endl;
            return 0;
        }

        // output indices --> {IQ, FAST_TIME, SLOW_TIME, RX, TX}
        void getIndices(int index_1D, int* indices){
            int iq=2;  
            int i0 = index_1D/(RX*IQ*FAST_TIME*TX);
            int i1 = index_1D%(RX*IQ*FAST_TIME*TX);
            int i2 = i1%(RX*IQ*FAST_TIME);
            int i3 = i2%(RX*IQ);
            int i4 = i3%(RX);
            
            indices[2] = i0;                    // SLOW_TIME | Chirp#
            indices[0] = i1/(RX*IQ*FAST_TIME);  // TX#
            indices[3] = i2/(RX*IQ);            // FAST_TIME | Range#
            indices[4] = i3/(RX);                 // IQ
            indices[1] = i4;                    // RX#
        }

        void shape_cube(float* in, float* mid, std::complex<float>* out) { 
            int rx=0;
            int tx=0;
            int iq=0;
            int fast_time=0;
            int slow_time=0;
            int indices[5] = {0};
            
            for (int i =0; i<SIZE_W_IQ; i++) {
                getIndices(i, indices);
                tx=indices[0]*RX*SLOW_TIME*FAST_TIME*IQ;
                rx=indices[1]*SLOW_TIME*FAST_TIME*IQ;
                slow_time=indices[2]*FAST_TIME*IQ;
                fast_time=indices[3]*IQ;
                iq=indices[4];
                mid[tx+rx+slow_time+fast_time+iq]=in[i];
            }

            for(int i=0; i<SIZE; i++){
                out[i]=std::complex<float>(mid[2*i+0], mid[2*i+1]);
            }
        }

        int compute_range_doppler(std::complex<float>* adc, std::complex<float>* rdm) {
            const int rank = 2;
            const int n[] = {SLOW_TIME, FAST_TIME};
            const int howmany = TX*RX;
            const int idist = SLOW_TIME*FAST_TIME;
            const int odist = SLOW_TIME*FAST_TIME;
            const int istride = 1;
            const int ostride = 1;

            fftwf_plan plan = fftwf_plan_many_dft(rank, n, howmany,
                                reinterpret_cast<fftwf_complex*>(adc), n, istride, idist,
                                reinterpret_cast<fftwf_complex*>(rdm), n, ostride, odist,
                                FFTW_FORWARD, FFTW_ESTIMATE);

            fftwf_execute(plan);
            return 0;
        }

        int compute_mag_norm(std::complex<float>* rdm_complex, float* rdm_norm) {
            float norm, log;
            std::complex<float> val;
            for(int i=0; i<SIZE; i++) {
                val=rdm_complex[i];
                norm=std::norm(val);
                log=log2f(norm)/2.0f;
                rdm_norm[i]=log;
            }
            return 0;
        }

        // rdm_avg should be zero-filled
        int averaged_rdm(float* rdm_norm, float* rdm_avg) {
            int idx;
            const int VIRT_ANTS = RX*TX;
            const int RD_BINS = SLOW_TIME*FAST_TIME;
            
            for (int i=0; i<(VIRT_ANTS); i++) {
                for (int j=0; j<(RD_BINS); j++) {
                idx=i*(RD_BINS)+j;
                rdm_avg[j]+=rdm_norm[idx]/((float) RD_BINS);
                }
            }
            return 0;   
        }
        void process() 
        {
            std::complex<float>* adc_data;
            readFile("../data/adc_data/STRUCT_VERIF.txt", adc_data_flat, SIZE_W_IQ);
            adc_data=reinterpret_cast<std::complex<float>*>(adc_data_flat);
            shape_cube(adc_data_flat, &adc_data_reshaped, adc_data);
            // compute_range_doppler(adc_data, rdm_data);
            // compute_mag_norm(rdm_data, rdm_norm);
            // averaged_rdm(rdm_norm, rdm_avg);
            printf("Range-Doppler map done!");
        }

        private: 
            int FAST_TIME, SLOW_TIME, RX, TX, IQ, SIZE_W_IQ, SIZE;
            float* adc_data_flat, rdm_avg, rdm_norm, adc_data_reshaped;
            std::complex<float>* rdm_data;
           

};

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

// Connection management //

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
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
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
