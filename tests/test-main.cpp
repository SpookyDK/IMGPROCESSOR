#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <../include/image_functions.h>
#include <chrono>
#include <thread>
#include <xsimd/xsimd.hpp>



int main(){
    std::cout.imbue(std::locale("en_US.UTF-8"));
    int width = 0, height = 0, channels = 0;
    unsigned char* data = Load_Image("test.JPG", width, height, channels);
    Image image = Image(data,width,height,channels);
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now();
    float time_ms = 0;

    int imgSize = image.channels * image.width * image.height ;

    size_t size_in_bytes = width * height * channels;
    unsigned int numThreads = std::thread::hardware_concurrency();
    size_t bytesPerThread = ( size_in_bytes + numThreads - 1) / numThreads; // ceil div
    bytesPerThread = (bytesPerThread + 63) & ~63; // round up to multiple of 64
    int remainder = imgSize % numThreads;

    std::vector<unsigned char*> starts(numThreads);
    std::vector<unsigned char*> ends(numThreads);

    std::vector<std::thread> threads;


    using batch_type = xsimd::batch<unsigned char, xsimd::avx512bw>;
    constexpr size_t alignment = batch_type::arch_type::alignment();
    unsigned char* workData = static_cast<unsigned char*>(std::aligned_alloc(alignment, size_in_bytes));
    memcpy(workData, image.data, size_in_bytes);
    
    std::cout << "simd width = " << batch_type::size << "\n";
    std::cout << "using " << numThreads << " threads\n";

    int its = 1000;
    start_time = std::chrono::high_resolution_clock::now();

    int64_t pixels_total;
    long pixel_per_s;



    // threads.clear();
    // for (unsigned int t = 0; t < numThreads; ++t) {
    //     unsigned char* start = workData + bytesPerThread * t;
    //     unsigned char* end = start + bytesPerThread;
    //     if (t == numThreads - 1) end += remainder; 
    //         threads.emplace_back([&image, start, end, its, bytesPerThread, t]() {
    //         for (int i = 0; i < its; i++){
    //             Adjust_Brightness_SIMD(image, 10, start, end);
    //             }
    //         });
    // }
    // for (auto& th : threads) th.join();
    //
    // end_time = std::chrono::high_resolution_clock::now();
    // time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    // pixels_total = int64_t(image.width) * image.height * its;
    // pixel_per_s = pixels_total / double(time_ms) * 1000.0;
    //
    // std::cout << " Time ms = " << time_ms 
    //           << ": pixel/S = " << pixel_per_s 
    //           << " FHD fps = " << pixel_per_s / (1920*1080) << "\n";


    start_time = std::chrono::high_resolution_clock::now();


    Image temp = image;
            for (int i = 0; i < its; i++){
                Adjust_Contrast(temp, 1.1);
                }
    end_time = std::chrono::high_resolution_clock::now();
    time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    pixels_total = int64_t(image.width) * image.height * its;
    pixel_per_s = pixels_total / double(time_ms) * 1000.0;

    std::cout << " Time ms = " << time_ms 
              << ": pixel/S = " << pixel_per_s 
              << " FHD fps = " << pixel_per_s / (1920*1080) << "\n";


}
