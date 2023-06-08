// g++ -std=c++11 -Wall -Wextra -pedantic -lfftw3f -lm -o bokbok bokbok.cpp; ./bokbok
#include <complex>
#include <cmath>
#include <fftw3.h>
#include <fstream>
#include <iostream>
#include <chrono>
// #include <opencv2/opencv.hpp>
// using namespace cv;
using namespace std::chrono;
class RangeDoppler 
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

    

        void blackman_window(float* arr, int size){
            for(int i = 0; i<size; i++)
                arr[i] = 0.42 - 0.5*cos(2*M_PI*i/(size-1))+0.08*cos(4*M_PI*i/(size-1));
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

        // output indices --> {TX, RX, SLOW_TIME, FAST_TIME, IQ}
        // output indices --> { 0,  1,         2,         3,  4}
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
            indices[4] = i3/(RX);               // IQ
            indices[1] = i4;                    // RX#
        }

        void shape_cube(float* in, float* mid, std::complex<float>* out, std::string& window_type) { 
            int rx=0;
            int tx=0;
            int iq=0;
            int fast_time=0;
            int slow_time=0;
            int indices[5] = {0};
            float window[FAST_TIME];
            if(window_type.compare("blackman") == 0)
                blackman_window(window, FAST_TIME);
            else if(window_type.compare("hann") == 0)
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
                mid[tx+rx+slow_time+fast_time+iq]=in[i]*window[indices[3]];
            }

            for(int i=0; i<SIZE; i++){
                out[i]=std::complex<float>(mid[2*i+0], mid[2*i+1]);
            }
        }

        int compute_range_doppler() {
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
        void process(std::string& file) 
        {
            auto start = high_resolution_clock::now();
            readFile(file, adc_data_flat, SIZE_W_IQ);
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(stop - start);
            std::cout << "YES: " << duration.count() << " microseconds" << std::endl;
        
            std::string WINDOW_TYPE = "blackman";
            shape_cube(adc_data_flat, adc_data_reshaped, adc_data, WINDOW_TYPE);
            compute_range_doppler();
            compute_mag_norm(rdm_data, rdm_norm);
            averaged_rdm(rdm_norm, rdm_avg);
            save_1d_array(rdm_avg, FAST_TIME, SLOW_TIME, "./out.txt");
            printf("Range-Doppler map done!");
        }

        private: 
            int FAST_TIME, SLOW_TIME, RX, TX, IQ, SIZE_W_IQ, SIZE;
            float *adc_data_flat, *rdm_avg, *rdm_norm, *adc_data_reshaped;
            std::complex<float>* rdm_data;
            std::complex<float>* adc_data;
            fftwf_plan plan;
           

};

int main(){
    int fast_time = 512;
    int slow_time = 64;
    int rx = 4;
    int tx = 3;
    int iq = 2;
    // auto start = high_resolution_clock::now();
    RangeDoppler rdm = RangeDoppler(fast_time,slow_time,rx,tx,iq);
    // auto stop = high_resolution_clock::now();
    std::string file = "../data/adc_data/out_DAQ.txt";
    auto start = high_resolution_clock::now();
    rdm.process(file);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    std::cout << "Elapsed time: " << duration.count() << " microseconds" << std::endl;

    // int width = 512;
    // int height = 64;

    // int px_width = 2;
    // int px_height = 10;
    // Mat image(px_height * height, px_width * width, CV_8UC1, Scalar(255));
    // namedWindow("Image", WINDOW_OPENGL);
    // for (int i = 0; i < width; i++) {
    //             for (int j = 0; j < height; j++) {
    //                 for(int x = 0; x < px_width; x++) {
    //                     for(int y = 0; y < px_height; y++) {
    //                         image.at<uint8_t>(px_height * j + y, px_width * i + x) = inputbufferptr[width * j + i];
    //                     }
    //                 }
    //             }
    //         }

    //         // Convert the matrix to a color image for visualization
    //         Mat colorImage;
    //         applyColorMap(image, colorImage, COLORMAP_JET);
            
    //         // Display the color image
    //         imshow("Image", colorImage);

    //         // Waits 1ms
    //         waitKey(1);



    return 0;
}