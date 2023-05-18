using namespace std;
using namespace cv;

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
    int width = 64;
    int height = 512;

    int px_width = 18;
    int px_height = 2;

    public:
        Visualizer(int size_in, int size_out, bool verbose = false) : RadarBlock(size_in, size_out, verbose), 
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
                            image.at<uint8_t>(px_height * j + y, px_width * i + x) = static_cast<uint8_t>(inputbufferptr[width*height - (width*height - height * i + j)]);
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
            waitKey(0);
        }

    private:
        Mat image;
        
};

////// Data transfer //////

// Processes IQ data to make Range-Doppler map
class RangeDoppler : public RadarBlock
{
    public:
        RangeDoppler(int fast_time, int slow_time, int rx, int tx, int iq, const char* win) : RadarBlock(fast_time*slow_time,fast_time*slow_time)
        {
            WINDOW_TYPE = win;
            FAST_TIME = fast_time;
            SLOW_TIME = slow_time;
            RX = rx;
            TX = tx;
            IQ = iq;
            SIZE = TX*RX*FAST_TIME*SLOW_TIME;
            SIZE_W_IQ = TX*RX*FAST_TIME*SLOW_TIME*IQ;
            adc_data_flat = reinterpret_cast<float*>(malloc(SIZE_W_IQ*sizeof(float)));
            adc_data=reinterpret_cast<std::complex<float>*>(adc_data_flat);
            adc_data_reshaped = reinterpret_cast<float*>(malloc(SIZE_W_IQ*sizeof(float)));
            rdm_data = reinterpret_cast<std::complex<float>*>(malloc(SIZE * sizeof(std::complex<float>)));
            rdm_norm = reinterpret_cast<float*>(malloc(SIZE * sizeof(float)));
            rdm_avg = reinterpret_cast<float*>(calloc(SLOW_TIME*FAST_TIME, sizeof(float)));
            const int rank = 2;
            const int n[] = {SLOW_TIME, FAST_TIME};
            const int howmany = TX*RX;
            const int idist = SLOW_TIME*FAST_TIME;
            const int odist = SLOW_TIME*FAST_TIME;
            const int istride = 1;
            const int ostride = 1;
            plan = fftwf_plan_many_dft(rank, n, howmany,
                                reinterpret_cast<fftwf_complex*>(adc_data), n, istride, idist,
                                reinterpret_cast<fftwf_complex*>(rdm_data), n, ostride, odist,
                                FFTW_FORWARD, FFTW_ESTIMATE);
        }
        // Retrieve outputbuffer pointer
        float* getBufferPointer()
        {
            return rdm_avg;
        }
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

        int save_1d_array(float* arr, int width, int length, const char* filename) {
            std::ofstream outfile(filename);
            for (int i=0; i<length*width; i++) {
                outfile << arr[i] << std::endl;
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
            float max,min;
            
   
            for (int i=0; i<(VIRT_ANTS); i++) {
                for (int j=0; j<(RD_BINS); j++) {
                    idx=i*(RD_BINS)+j;
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
            scale_rdm_values(rdm_avg, max, min);
            fftshift_rdm(rdm_avg);
            return 0;   
        }
        
        
        
        
        void process(const char* filename)
        {
            readFile(filename, adc_data_flat, SIZE_W_IQ);
            adc_data=reinterpret_cast<std::complex<float>*>(adc_data_flat);
            shape_cube(adc_data_flat, adc_data_reshaped, adc_data);
            compute_range_doppler();
            compute_mag_norm(rdm_data, rdm_norm);
            averaged_rdm(rdm_norm, rdm_avg);
            // save_1d_array(rdm_avg, FAST_TIME, SLOW_TIME, "./out.txt");
            printf("Range-Doppler map done! \n");
        }

        private: 
            int FAST_TIME, SLOW_TIME, RX, TX, IQ, SIZE_W_IQ, SIZE;
            float *adc_data_flat, *rdm_avg, *rdm_norm, *adc_data_reshaped;
            std::complex<float> *rdm_data, *adc_data;
            fftwf_plan plan;
            const char *WINDOW_TYPE;
        
};
