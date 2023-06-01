using namespace std;
using namespace cv;
using namespace std::chrono;
// Base class used for other modules
class RadarBlock
{
    int inputsize;
    int outputsize;
    bool verbose;

    public:
        // Public variables
        uint frame = 0;

        uint* inputframeptr;
        float* inputbufferptr;

        uint lastframe;

        // Public functions
        // Class constructor
        RadarBlock(int size_in, int size_out, bool v = false) : outputbuffer(new float[size_out])
        {   
            inputsize = size_in;
            outputsize = size_out;
            verbose = v;

            printf("New %s created.\n", typeid(*this).name());
        }

        // Class deconstructor
        ~RadarBlock()
        {
            delete[] outputbuffer; 

            printf("%s destroyed.\n", typeid(*this).name());
        }

        // Sets the input buffer pointer
        void setBufferPointer(float* ptr)
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
        float* getBufferPointer()
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

                // start timer
                auto start = chrono::high_resolution_clock::now();

                process();

                // stop timer
                auto stop = chrono::high_resolution_clock::now();

                if(verbose)
                {
                    // calculate elapsed time in microseconds
                    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);

                    // print elapsed time
                    cout << "Elapsed time: " << duration.count() << " microseconds" << endl;
                }

                increment_frame();
            }
        }

    private:
        // Private variables
        float* outputbuffer;

        // Private functions
        // Listens for previous block (overwritten in some cases)
        virtual void listen()
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
    int width = 64;
    int height = 512;

    int px_width = 10;
    int px_height = 1;

    float X_SCALE = 0.16;
    float Y_SCALE = 0.035;
    int stepSizeX = 64;
    int stepSizeY = 57;

    public:
        Visualizer(int size_in, int size_out, bool verbose = false) : RadarBlock(size_in, size_out, verbose), 
            image(px_height * height, px_width * width, CV_8UC1, Scalar(255))
        {
            
            namedWindow("Image");
        }

        // Visualizer's process
        void process() override
        {
            auto start = chrono::high_resolution_clock::now();
            // 
            for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                    for(int x = 0; x < px_width; x++) {
                        for(int y = 0; y < px_height; y++) {
                            image.at<uint8_t>(px_height * j + y, px_width * i + x) = static_cast<uint8_t>(inputbufferptr[width*height - ((width-1)*height - height * i + j)]);
                        }
                    }
                }
            }
            
            // Convert the matrix to a color image for visualization
            Mat colorImage;
            applyColorMap(image, colorImage, COLORMAP_JET);
            
            int borderSize = 30;
            cv::Scalar borderColor(0, 0, 0); // Green color

            // Add the padded border
            cv::Mat borderedImage;
            cv::copyMakeBorder(colorImage, borderedImage, borderSize, borderSize, borderSize, borderSize,
                            cv::BORDER_CONSTANT, borderColor);

            cv::Point origin(borderSize, borderedImage.rows-borderSize);
            cv::Point xEnd(borderedImage.cols-borderSize, borderedImage.rows-borderSize);
            cv::Point yEnd(borderSize, borderSize);
            
            cv::line(borderedImage, origin, xEnd, cv::Scalar(0, 0, 255), 2);
            cv::line(borderedImage, origin, yEnd, cv::Scalar(0, 255, 0), 2);
            
            //cv::putText(borderedImage, "m/s", xEnd, cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(0, 0, 255), 2);
            //cv::putText(borderedImage, "m", yEnd, cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(0, 255, 0), 2);
            
            
            for (int i = origin.x + stepSizeX; i < borderedImage.cols; i += stepSizeX) {
                std::ostringstream stream;
                stream << std::fixed << std::setprecision(0) << ((i - origin.x) - width*px_width/2)*X_SCALE/px_width;
                cv::Point pt(i, origin.y);
                cv::line(borderedImage, pt, pt - cv::Point(0, 5), cv::Scalar(255, 255, 255), 2);
                cv::putText(borderedImage, stream.str(), pt + cv::Point(-10, 20),
                            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
            }
            for (int i = origin.y - stepSizeY; i >= 0; i -= stepSizeY) {
                std::ostringstream stream;
                stream << std::fixed << std::setprecision(0) << (origin.y - i)*Y_SCALE;
                cv::Point pt(origin.x, i);
                cv::line(borderedImage, pt, pt + cv::Point(5, 0), cv::Scalar(255, 255, 255), 2);
                cv::putText(borderedImage, stream.str(), pt + cv::Point(-30, 10),
                            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
            }



            // Display the color image
            imshow("Image", borderedImage);

            // Waits 1ms
            waitKey(wait_time);
            auto stop = chrono::high_resolution_clock::now();
            auto duration_vis_process = duration_cast<microseconds>(stop - start);
            std::cout << "VIS Process Time " << duration_vis_process.count() << " microseconds" << std::endl;
            
        }

        void setWaitTime(int num){
            wait_time = num;
        }
        
        void listen() override
        {
            return;
        }

    private:
        Mat image;
        int wait_time;
        
};

// Processes IQ data to make Range-Doppler map
class RangeDoppler : public RadarBlock
{
    public:
        RangeDoppler(int fast_time, int slow_time, int rx, int tx, int iq, const char* win = "blackman") : RadarBlock(fast_time*slow_time,fast_time*slow_time)
        {
            // RANGE DOPPLER PARAMETER INITIALIZATION
            WINDOW_TYPE = win;          //Determines what type of windowing will be cone
            FAST_TIME = fast_time;      //Initializes the number of fast time samples | # of range samples
            SLOW_TIME = slow_time;      //Initializes the number of slow time samples | # of doppler samples
            RX = rx;    // # of Rx
            TX = tx;    // # of Tx
            IQ = iq;    // Equals to 2 as there is both I and Q data
            SIZE = TX*RX*FAST_TIME*SLOW_TIME;           // Size of the total number of COMPLEX samples from ONE frame
            SIZE_W_IQ = TX*RX*FAST_TIME*SLOW_TIME*IQ;   // Size of the total number of separate IQ sampels from ONE frame
            adc_data_flat = reinterpret_cast<float*>(malloc(SIZE_W_IQ*sizeof(float)));                          // allocate mem for Separate IQ adc data from Data aquisition
            adc_data=reinterpret_cast<std::complex<float>*>(adc_data_flat);                                     // allocate mem for COMPLEX adc data from Data aquisition 
            adc_data_reshaped = reinterpret_cast<float*>(malloc(SIZE_W_IQ*sizeof(float)));                      // allocate mem for reorganized/reshaped adc data
            rdm_data = reinterpret_cast<std::complex<float>*>(malloc(SIZE * sizeof(std::complex<float>)));      // allocate mem for processed complex adc data
            rdm_norm = reinterpret_cast<float*>(malloc(SIZE * sizeof(float)));                                  // allocate mem for processed magnitude adc data
            rdm_avg = reinterpret_cast<float*>(calloc(SLOW_TIME*FAST_TIME, sizeof(float)));                     // allocate mem for averaged adc data across all virtual antennas
            
            // FFT SETUP PARAMETERS
            const int rank = 2;     // Determines the # of dimensions for FFT
            const int n[] = {SLOW_TIME, FAST_TIME};
            const int howmany = TX*RX;
            const int idist = SLOW_TIME*FAST_TIME;
            const int odist = SLOW_TIME*FAST_TIME;
            const int istride = 1;
            const int ostride = 1;
            plan = fftwf_plan_many_dft(rank, n, howmany,
                                reinterpret_cast<fftwf_complex*>(adc_data), n, istride, idist,
                                reinterpret_cast<fftwf_complex*>(rdm_data), n, ostride, odist,
                                FFTW_FORWARD, FFTW_ESTIMATE);      // create the FFT plan
        }
        // Retrieve outputbuffer pointer
        float* getBufferPointer()
        {
            return rdm_avg;
        }

        void setBufferPointer(uint16_t* arr){
            input = arr;
        }

        // FILE READING METHODS
        void readFile(const std::string& filename, float* arr, int size) {      //
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

        int save_1d_array(float* arr, int width, int length, string& filename) {
            std::ofstream outfile(filename);
            for (int i=0; i<length*width; i++) {
                outfile << arr[i] << std::endl;
            }

            //outfile.close();
            std::cout << "Array saved to file. " << std::endl;
            return 0;
        }

        // WINDOW TYPES
        void blackman_window(float* arr, int fast_time){
            for(int i = 0; i<fast_time; i++)
                arr[i] = 0.42 - 0.5*cos(2*M_PI*i/(fast_time-1))+0.08*cos(4*M_PI*i/(fast_time-1));
        }

        void hann_window(float* arr, int fast_time){
            for(int i = 0; i<fast_time; i++)
                arr[i] = 0.5 * (1 - cos((2 * M_PI * i) / (fast_time - 1)));
        }
        
        void no_window(float* arr, int fast_time){
            for(int i = 0; i<fast_time; i++)
                arr[i] = 1;
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
            float window[FAST_TIME];
            if(strcmp(WINDOW_TYPE,"blackman") == 0)
                blackman_window(window, FAST_TIME);
            else if(strcmp(WINDOW_TYPE,"hann") == 0)
                hann_window(window, FAST_TIME);
            else
                no_window(window, FAST_TIME);
            
            for (int i =0; i<SIZE_W_IQ; i++) {
                getIndices(i, indices);
                tx=indices[0]*RX*SLOW_TIME*FAST_TIME*IQ;
                rx=indices[1]*SLOW_TIME*FAST_TIME*IQ;
                slow_time=indices[2]*FAST_TIME*IQ;
                fast_time=indices[3]*IQ;
                iq=indices[4];
                mid[tx+rx+slow_time+fast_time+iq]=in[i]*window[fast_time/IQ];
            }

            for(int i=0; i<SIZE; i++){
                out[i]=std::complex<float>(mid[2*i+0], mid[2*i+1]);
            }
        }

        int compute_range_doppler() {
            fftwf_execute(plan);
            return 0;
        }

        
        void scale_rdm_values(float* arr, float max_val, float min_val){
            // fill in the matrix with the values scaled to 0-255 range
            for (int i = 0; i < FAST_TIME*SLOW_TIME; i++) {
                arr[i] = (arr[i] - min_val) / (max_val - min_val) * 255;
            }
        }

        void fftshift_rdm(float* arr){
            int midRow = FAST_TIME / 2;
            int midColumn = SLOW_TIME / 2;
            float fftshifted[SLOW_TIME*FAST_TIME];
           
            for (int i = 0; i < FAST_TIME; i++) {
                for (int j = 0; j < SLOW_TIME; j++) {
                    int newRow = (i + midRow) % FAST_TIME;          // ROW WISE FFTSHIFT
                    int newColumn = (j + midColumn) % SLOW_TIME;    // COLUMN WISE FFTSHIFT
                    fftshifted[newRow * SLOW_TIME + j] = arr[i * SLOW_TIME + j]; // only newRow is used so only row wise fftshift
                }
            }
            for(int i = 0; i < FAST_TIME*SLOW_TIME; i++)
                arr[i] = fftshifted[i];
        }

        int compute_mag_norm(std::complex<float>* rdm_complex, float* rdm_magnitude) {
            float norm, log;
            std::complex<float> val; 
            for(int i=0; i<SIZE; i++) {
                val=rdm_complex[i];
                norm=std::norm(val);
                log=log2f(norm)/2.0f;
                rdm_magnitude[i]=log;
            }         
            return 0;
        }
        // rdm_avg should be zero-filled
        int averaged_rdm(float* rdm_norm, float* rdm_avg) {
            int idx;
            const int VIRT_ANTS = TX*RX;
            const int RD_BINS = SLOW_TIME*FAST_TIME;
            float max,min;
            for (int i=0; i<(VIRT_ANTS); i++) {
                for (int j=0; j<(RD_BINS); j++) {
                    idx=i*(RD_BINS)+j;
                    if(i==0)
                        rdm_avg[j] = 0;
                    rdm_avg[j]+=rdm_norm[idx]/((float) RD_BINS);
                    if(i == (VIRT_ANTS-1)){
                        if (j==0){
                            max = rdm_avg[0];
                            min =  rdm_avg[0];                            
                        }
                        if (rdm_avg[j] > max)
                            max = rdm_avg[j];
                        else if(rdm_avg[j] < min)
                            min = rdm_avg[j];
                        
                    }
                }
            }
            max = 0.0108;
            min = 0.008;
            //std::cout << "MAX: " << max << "      |        MIN:  " << min << std::endl;
            
            scale_rdm_values(rdm_avg, max, min);
            fftshift_rdm(rdm_avg);
            return 0;   
        }
        
        void process() override
        {
            auto start = chrono::high_resolution_clock::now();
            // std::cout<< "RDM PROCESS ACTIVATED" << std::endl;
            for(int i = 0; i<SIZE_W_IQ; i++){
                adc_data_flat[i] = (float)input[i];
                // if( i>90 && i<110)
                    // std::cout<< adc_data_flat[i] << "   |    " << input[i] << std::endl;
            }
            // auto start = high_resolution_clock::now();
            // std::cout << "1: IN RDM: First data in frame = " << adc_data_flat[100] << std::endl;
            shape_cube(adc_data_flat, adc_data_reshaped, adc_data);
            compute_range_doppler();
            compute_mag_norm(rdm_data, rdm_norm);
            averaged_rdm(rdm_norm, rdm_avg);
            // std::cout << "rdm_data = " << rdm_data[100] << "     |       rdm_norm = " << rdm_norm[100] << "   |    rdm_avg = " << rdm_avg[100] << std::endl;
            // std::cout << "3: IN RDM: First data in frame = " << adc_data_flat[100] << std::endl;
            // string str = ("./out") + to_string(frame) + ".txt";
            // save_1d_array(rdm_avg, FAST_TIME, SLOW_TIME, str);
            // auto stop = high_resolution_clock::now();
            // auto duration = duration_cast<microseconds>(stop - start);
            // std::cout << "Elapsed PROCESS time: " << duration.count() << " microseconds" << std::endl;
            // printf("Range-Doppler map done! \n");

            auto stop = chrono::high_resolution_clock::now();
            auto duration_rdm_process = duration_cast<microseconds>(stop - start);
            std::cout << "RDM Process Time " << duration_rdm_process.count() << " microseconds" << std::endl;
        }

        private: 
            int FAST_TIME, SLOW_TIME, RX, TX, IQ, SIZE_W_IQ, SIZE;
            float *adc_data_flat, *rdm_avg, *rdm_norm, *adc_data_reshaped;
            std::complex<float> *rdm_data, *adc_data;
            fftwf_plan plan;
            uint16_t* input;
            const char *WINDOW_TYPE;
        
};

// Data Acquisition class 
class DataAcquisition : public RadarBlock
{ 
    public:
        DataAcquisition(int buffer_size, int port, int bytes_in_packet, int fast_time, int slow_time, int rx, int tx, int iq_data, int iq_bytes) : RadarBlock(fast_time*slow_time,fast_time*slow_time)
        {
            BUFFER_SIZE = buffer_size; 
            PORT = port;
            BYTES_IN_PACKET = bytes_in_packet;
            RX = rx;
            TX = tx;
            FAST_TIME = fast_time;
            SLOW_TIME = slow_time;
            IQ_DATA = iq_data;
            IQ_BYTES = iq_bytes;
            SIZE_W_IQ = TX*RX*IQ_DATA*SLOW_TIME*FAST_TIME;
            frame_data = reinterpret_cast<uint16_t*>(malloc(SIZE_W_IQ*sizeof(uint16_t)));
            BYTES_IN_FRAME = SLOW_TIME*FAST_TIME*RX*TX*IQ_DATA*IQ_BYTES;
            BYTES_IN_FRAME_CLIPPED = BYTES_IN_FRAME/BYTES_IN_PACKET*BYTES_IN_PACKET;
            PACKETS_IN_FRAME_CLIPPED = BYTES_IN_FRAME / BYTES_IN_PACKET;
            UINT16_IN_PACKET = BYTES_IN_PACKET / 2; //728 entries in packet
            UINT16_IN_FRAME = BYTES_IN_FRAME / 2;
            packets_read = 0;
            buffer=reinterpret_cast<char*>(malloc(BUFFER_SIZE*sizeof(char)));
            packet_data=reinterpret_cast<uint16_t*>(malloc(UINT16_IN_PACKET*sizeof(uint16_t)));

            
        }

        // create_bind_socket - returns a socket object titled sockfd
        int create_bind_socket(){
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
            // std::cout << "Socket Binded Success!" << std::endl;
            return 0;
        }
        
        void close_socket(){
            close(sockfd);
        }

        // read_socket will generate the buffer object that holds all raw ADC data
        void read_socket(){
            auto start = chrono::high_resolution_clock::now();
            // n is the packet size in bytes (including sequence number and byte count)

            n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
            buffer[n] = '\0'; // Null-terminate the buffer
            auto stop = chrono::high_resolution_clock::now();
            auto duration_read_socket = duration_cast<microseconds>(stop - start);
            std::cout << "Read Socket " << duration_read_socket.count() << std::endl;

            start = chrono::high_resolution_clock::now();

            stop = chrono::high_resolution_clock::now();
            auto duration_set_packet_data = duration_cast<microseconds>(stop - start);
            std::cout << "Set Packet Data " << duration_set_packet_data.count() << std::endl;
        }

        // get_packet_num will look at the buffer and return the packet number
        uint32_t get_packet_num(){
            uint32_t packet_number = ((buffer[0] & 0xFF) << 0)  |
                                    ((buffer[1] & 0xFF) << 8)  |
                                    ((buffer[2] & 0xFF) << 16) |
                                    ((long) (buffer[3] & 0xFF) << 24);
            return packet_number;
        }
        // get_byte_count will look at the buffer and return the byte count of the packet
        uint64_t get_byte_count(){
            uint64_t byte_count = ((buffer[4] & 0xFF) << 0)  |
                                ((buffer[5] & 0xFF) << 8)  |
                                ((buffer[6] & 0xFF) << 16) |
                                ((buffer[7] & 0xFF) << 24) |
                                ((unsigned long long) (buffer[8] & 0xFF) << 32) |
                                ((unsigned long long) (buffer[9] & 0xFF) << 40) |
                                ((unsigned long long) (0x00) << 48) |
                                ((unsigned long long) (0x00) << 54);
            return byte_count;
        }

        /*void set_packet_data(){
            // printf("Size of packet data = %d \n", BYTES_IN_PACKET);
            for (int i = 0; i< UINT16_IN_PACKET; i++)
            {
                packet_data[i] =  buffer[2*i+10] | (buffer[2*i+11] << 8);
            }
        }*/

        void set_frame_data(){
            //Add packet_data to frame_data
            for (int i = UINT16_IN_PACKET*packets_read; i < (UINT16_IN_PACKET*(packets_read+1)); i++)
            {
                frame_data[i] = buffer[2*(i-UINT16_IN_PACKET*packets_read)+10] | (buffer[2*(i-UINT16_IN_PACKET*packets_read)+11] << 8);
                // frame_data[i] = packet_data[i%UINT16_IN_PACKET];
                
            }
            packets_read++;
        }

        int end_of_frame(){
            uint64_t byte_mod = (packets_read*BYTES_IN_PACKET) % BYTES_IN_FRAME_CLIPPED;
            if (byte_mod == 0) //end of frame found
                return 1;           
            return 0;
        }

        int save_1d_array(uint16_t* arr, int width, int length, string& filename) {
            std::ofstream outfile(filename);
            for (int i=0; i<length*width; i++) {
                outfile << arr[i] << std::endl;
            }

            //outfile.close();
            std::cout << "Array saved to file. " << std::endl;
            return 0;
        }


        uint16_t* getBufferPointer(){
            return frame_data;
        }

        void listen() override
        {
             for(;;)
            {
                if(frame==0){
                    break;
                }
                if(*inputframeptr != lastframe)
                {   
                    lastframe = *inputframeptr;
                    break;
                }
            }
        }

        void process() override
        {

            auto start = chrono::high_resolution_clock::now();
            // create buffer array of preset size to hold one packet
            // buffer[BUFFER_SIZE];
            create_bind_socket();
            
            //auto duration_read_socket = duration_cast<microseconds>(stop - start);
            //auto duration_set_frame_data = duration_cast<microseconds>(stop - start);

            // while true loop to get a single frame of data from UDP 
            // std::cout<< "DAQ PROCESS ACTIVATED" << std::endl;
            // std::cout << "FRAME #: " << frame << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            //start = chrono::high_resolution_clock::now();
            while (true)
                {
                    //std::cout << "Packet Num " << get_packet_num() << std::endl;  //optional 
                    //rc = poll();
                    //if (rv == 1)
                    //{
                        read_socket();
                        //start = chrono::high_resolution_clock::now();
                        set_frame_data();
                        //stop = chrono::high_resolution_clock::now();
                        //duration_set_frame_data = duration_cast<microseconds>(stop - start);
                        //std::cout << "Set Frame Data " << duration_set_frame_data.count() << std::endl;
                        //std::cout << std::endl;   
                    //}
                    // get_byte_count();  //optional
 
                    if (end_of_frame() == 1){
                        // printf("End of frame found \n");
                        // printf("IN DAQ: First data in frame = %d \n", frame_data[100]);
                        //  for(int i = 91; i<110; i++){
                        //     std::cout<< frame_data[i] << std::endl;
                        // }
                        //stop = chrono::high_resolution_clock::now();
                        //auto duration_read_write_frame = duration_cast<microseconds>(stop - start);

                        //string str = ("./out") + to_string(frame) + ".txt";
                        // save_1d_array(frame_data, FAST_TIME*TX*RX*IQ_DATA, SLOW_TIME, str);
                        packets_read = 0;
                        // first_packet = true;
                        //start = chrono::high_resolution_clock::now();

                        close_socket();
                        //stop = chrono::high_resolution_clock::now();
                        //auto duration_close_socket = duration_cast<microseconds>(stop - start);


                        //std::cout << "Create Socket " << duration_create_socket.count() << std::endl;
                        //std::cout << std::endl;
                        //std::cout << "Read/Write Frame " << duration_read_write_frame.count() << std::endl;
                        //std::cout << std::endl;
                        //std::cout << "Close Socket " << duration_close_socket.count() << std::endl;
                        //std::cout << std::endl;
                        break;
                    }
                }
            //auto stop = chrono::high_resolution_clock::now();
            //auto duration = duration_cast<microseconds>(stop - start);
            //std::cout << "Create Socket " << duration_create_socket.count() << std::endl;
            //std::cout << std::endl;
            // auto stop = chrono::high_resolution_clock::now();
            // auto duration = duration_cast<microseconds>(stop - start);
            // std::cout << "Process Time " << duration.count() << std::endl;
            // std::cout << std::endl;
            auto stop = chrono::high_resolution_clock::now();
            auto duration_daq_process = duration_cast<microseconds>(stop - start);
            std::cout << "DAQ Process Time " << duration_daq_process.count() << " microseconds" << std::endl;
            std::cout << "~~~~~~~~~~~~~~~~~~~END OF SINGLE FRAME~~~~~~~~~~~~~~~~~~~~" << std::endl;


        }

        private: 
            int BUFFER_SIZE, PORT, BYTES_IN_PACKET, RX, TX, FAST_TIME, SLOW_TIME, IQ_DATA, IQ_BYTES, SIZE_W_IQ; 
            
            int sockfd;                             // socket file descriptor
            struct sockaddr_in servaddr, cliaddr;   // initialize socket
            socklen_t len;
            
            char* buffer;
            int n;  // n is the packet size in bytes (including sequence number and byte count)
            
            uint16_t *packet_data, *frame_data;  
            uint32_t packet_num;
            uint64_t BYTES_IN_FRAME, BYTES_IN_FRAME_CLIPPED, PACKETS_IN_FRAME_CLIPPED, UINT16_IN_PACKET, UINT16_IN_FRAME, packets_read;
            
            std::complex<float> *rdm_data, *adc_data;
            float *adc_data_flat, *rdm_avg, *rdm_norm, *adc_data_reshaped;
            
        
};
