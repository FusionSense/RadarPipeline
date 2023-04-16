#include "rpl/private-header.hpp"
 
int main()
{   
    // Upstream frame count
    uint inputframe = 0;
    uint* inputptr = &inputframe;

    // New block
    RadarBlock obj(8);
    obj.setInputPointer(inputptr);

    thread myThread(&RadarBlock::iteration, &obj, 42);
    inputframe++;

    // int* bufferptr = obj.getBufferPointer();
    // uint* frameptr = obj.getFramePointer();

    myThread.join();
    return 0;
}
