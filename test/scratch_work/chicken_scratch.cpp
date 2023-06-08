// TO RUN: g++ -std=c++11 -o chicken chicken_scratch.cpp `pkg-config --cflags --libs opencv4`; ./chicken
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std::chrono;
using namespace cv;
using namespace std;

#define FAST_TIME 512
#define SLOW_TIME 64
#define RX 4
#define TX 3
#define IQ 2
#define SIZE TX*RX*IQ*FAST_TIME*SLOW_TIME

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

int main() {

    auto rdm = new float [FAST_TIME][SLOW_TIME];
    auto adc = new float [FAST_TIME*SLOW_TIME];
    readFile("../data/rdm_data/RDM_TEST_OUTPUT.txt", adc, FAST_TIME*SLOW_TIME);

    for(int i = 0; i<FAST_TIME*SLOW_TIME; i++)
        rdm[i/SLOW_TIME][i%SLOW_TIME] = adc[i];   


    Mat matrix(FAST_TIME, SLOW_TIME, CV_8UC1);
    
    float min_val = rdm[0][0], max_val = rdm[0][0];
    
    // find the minimum and maximum values in the array
    for (int i = 0; i < FAST_TIME; i++) {
        for (int j = 0; j < SLOW_TIME; j++) {
            if (rdm[i][j] < min_val) {
                min_val = rdm[i][j];
            }
            if (rdm[i][j] > max_val) {
                max_val = rdm[i][j];
            }
        }
    }
    
    // fill in the matrix with the values scaled to 0-255 range
    for (int i = 0; i < FAST_TIME; i++) {
        for (int j = 0; j < SLOW_TIME; j++) {
            matrix.at<uint8_t>(i, j) = (uint8_t)((rdm[i][j] - min_val) / (max_val - min_val) * 255);
        }
    }

    // Start the timer
    Mat color;
    applyColorMap(matrix, color, cv::COLORMAP_JET);

    
    resize(color,color,Size(600,600));
    auto start = high_resolution_clock::now();
    namedWindow("Image", WINDOW_FREERATIO);
    imshow("Image", color);
    // Stop the timer
    auto stop = high_resolution_clock::now();
    
    // Calculate the elapsed time in microseconds
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Elapsed time: " << duration.count() << " microseconds" << endl;
    waitKey(0);
    return 0;
}
