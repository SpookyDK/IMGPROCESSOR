#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <../include/image_functions.h>
#include <chrono>



int main(){
    std::cout.imbue(std::locale("en_US.UTF-8"));
    int width = 0, height = 0, channels = 0;
    unsigned char* data = Load_Image("test.JPG", width, height, channels);
    Image image = Image(data,width,height,channels);
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now();
    float time_ms = 0;


    

    int its = 100;
    start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < its; i ++){
        Image temp = image;
        Adjust_Contrast(temp,1.1);
    }

    end_time = std::chrono::high_resolution_clock::now();
    time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    int64_t pixels_total = int64_t(image.width) * image.height * image.channels * its;
    long pixel_per_s = pixels_total / double(time_ms) * 1000.0;

    std::cout << "Adjust_Brightness:10 ms = " << time_ms 
              << ": pixel/S = " << pixel_per_s << "\n";



}
