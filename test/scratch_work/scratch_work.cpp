#include <iostream>
#include <complex>
#include <fstream>

#include <complex.h>
#include <fftw3.h>

#define FAST_TIME 512
#define SLOW_TIME 64
#define RX 4
#define TX 3
#define IQ 2
#define SIZE_W_IQ TX*RX*FAST_TIME*SLOW_TIME*IQ
#define SIZE TX*RX*FAST_TIME*SLOW_TIME

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

// int to_index(int RX, int TX, int SLOW_TIME, int FAST_TIME) 

int main() {
    // Create a 1D float array
  auto adc_data = reinterpret_cast<float*>(malloc(SIZE_W_IQ*sizeof(float)));
  auto adc_data_reshaped = reinterpret_cast<float*>(malloc(SIZE_W_IQ*sizeof(float)));
  int indices[5] = {0};
  int rx=0;
  int tx=0;
  int iq=0;
  int fast_time=0;
  int slow_time=0;
  
  auto rdm_data = reinterpret_cast<std::complex<float>*>(malloc(SIZE*sizeof(std::complex<float>)));
  
  readFile("../data/adc_data/adc_data00.txt", adc_data, SIZE_W_IQ);
  
  shape_cube(adc_data, adc_data_reshaped, rdm_data);
  
  std::cout << rdm_data[0] << std::endl;
    
  return 0;
}