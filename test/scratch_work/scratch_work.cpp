#include <iostream>
#include <complex>
#include <fstream>

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

// output indices --> {IQ, FAST_TIME, SLOW_TIME, RX, TX}
void getIndices(int index_1D, int* indices, int rx, int iq, int fast_time, int tx, int slow_time){
  
    int i0 = index_1D/(rx*iq*fast_time*tx);
    int i1 = index_1D%(rx*iq*fast_time*tx);
    int i2 = i1%(rx*iq*fast_time);
    int i3 = i2%(rx*iq);
    int i4 = i3%(rx);

    indices[2] = i0;                    // SLOW_TIME | Chirp#
    indices[4] = i1/(rx*iq*fast_time);  // TX#
    indices[1] = i2/(rx*iq);            // FAST_TIME | Range#
    indices[0] = i3/rx;                 // IQ
    indices[3] = i4;                    // RX#
}

// int to_index(int RX, int TX, int SLOW_TIME, int FAST_TIME) 

int main() {
    // Create a 1D float array
    float adc_data[SIZE] = {0};
    int indices[5] = {0};
    auto rdm_data = new _Float16 [IQ][FAST_TIME][SLOW_TIME][RX][TX];

    readFile("/Users/bindingoath/Documents/CAPSTONE/CODEBASE/RadarPipeline/test/data/adc_data/adc_data00.txt", adc_data, SIZE);
    
    // get the mapped indices, then assign elements from raw 1D adc data into rdm 5D data cube;
    for (int i = 0; i<SIZE; i++){
        // std::cout <<"Index = "<< i << ": "<< adc_data[i] << std::endl;
        getIndices(i, indices, RX, IQ, FAST_TIME, TX, SLOW_TIME);
        // std::cout << "IQ: " << indices[0]+1 << std::endl;
        // std::cout << "RX: " << indices[3]+1 << std::endl;
        // std::cout << "Fast: " << indices[1]+1 << std::endl;
        // std::cout << "Chirp: " << indices[2]+1 << std::endl;
        // std::cout << "TX: " << indices[4]+1 << std::endl;
        // std::cout << indices[0] << indices[1]<< indices[2] << indices[3] << indices[4] << std::endl;
        // std::cout << "" << std::endl;
        rdm_data[indices[0]][indices[1]][indices[2]][indices[3]][indices[4]] = adc_data[i];
    }

    // std::complex<float> dummy[2];
    // dummy[1] = std::complex<float>(rdm_data[0][0][0][0][0], rdm_data[1][0][0][0][0]);    
    // std::cout << dummy[1] << std::endl;



    // time to combine the I and Q into a complex rdm 4D data cube
    auto rdm_data_complex = new std::complex<float>[FAST_TIME][SLOW_TIME][RX][TX];

    //ALLOCATE MEMORY
    // std::complex<float> ****rdm_data_complex=(std::complex<float>****)calloc(FAST_TIME,SIZEof(float****));
    // for(int i = 0 ; i < FAST_TIME; i++){
    //     for(int j = 0 ; j < SLOW_TIME; j++){
    //         for(int k = 0 ; k < RX; k++){
    //             for(int l = 0 ; l < TX; l++){
    //                 rdm_data_complex[i][j][k][l] = std::complex<float>(rdm_data[0][i][j][k][l], rdm_data[1][i][j][k][l]);
    //             }
    //         }
    //     }
    // }
    for(int i = 0 ; i < FAST_TIME; i++){
        for(int j = 0 ; j < SLOW_TIME; j++){
            for(int k = 0 ; k < RX; k++){
                for(int l = 0 ; l < TX; l++){
                    rdm_data_complex[i][j][k][l] = std::complex<float>(rdm_data[0][i][j][k][l], rdm_data[1][i][j][k][l]);
                }
            }
        }
    }

    std::cout << rdm_data_complex[0][0][0][0] << std::endl;
    
    return 0;
}




