#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <../include/image_functions.h>
#include <chrono>
#include <thread>



int main(){
    std::cout.imbue(std::locale("en_US.UTF-8"));
    int width = 0, height = 0, channels = 0;
    unsigned char* data = Load_Image("test.JPG", width, height, channels);
    Image image = Image(data,width,height,channels);
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now();
    float time_ms = 0;

    int imgSize = image.channels * image.width * image.height ;

    unsigned int numThreads = std::thread::hardware_concurrency();
    int chunkSize = imgSize  / numThreads;
    int remainder = imgSize % numThreads;

    std::vector<unsigned char*> starts(numThreads);
    std::vector<unsigned char*> ends(numThreads);

    std::vector<std::thread> threads;

    int its = 100;
    start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < its; i ++){
        threads.clear();
        Image temp = image;
        unsigned char* start = temp.data;
            for (unsigned int t = 0; t < numThreads; ++t) {
                unsigned char* end = start + chunkSize;
                if (t == numThreads - 1) end += remainder; 
                threads.emplace_back([&temp, start, end]() {
                            Adjust_Brightness_SIMD(temp, 10, start, end);
                        });
            }
            for (auto& th : threads) th.join();

    }

    end_time = std::chrono::high_resolution_clock::now();
    time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    int64_t pixels_total = int64_t(image.width) * image.height * image.channels * its;
    long pixel_per_s = pixels_total / double(time_ms) * 1000.0;

    std::cout << " Time ms = " << time_ms 
              << ": pixel/S = " << pixel_per_s 
              << " FHD fps = " << pixel_per_s / (1920*1080) << "\n";



}
