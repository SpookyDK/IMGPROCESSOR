#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <numbers>
#include <iostream>
#include <iostream>
#include <chrono>
#include "mainwindow.h"
#include "image_functions.h"




int main(int argc, char *argv[]) {
    auto start_time = std::chrono::high_resolution_clock::now();

    int width, height, channels;
    unsigned char* data = Load_Image("../test.JPG", width, height, channels);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "\nLOAD img time = " << duration.count() << " ms\n";
    std::cout << "IMG Width:Height:Total(pixels):TotalBytes" << width << ":" << height << ":" << width*height << ":" << width*height*channels*sizeof(data);

    start_time = std::chrono::high_resolution_clock::now();

    Rotate_Image_90_Counter(data, width, height, channels);

    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "\nRotate time = " << duration.count() << " ms\n";
    std::cout << "IMG Width:Height:Total(pixels):TotalBytes" << width << ":" << height << ":" << width*height << ":" << width*height*channels*sizeof(data);
    start_time = std::chrono::high_resolution_clock::now();

    start_time = std::chrono::high_resolution_clock::now();

    Adjust_Brightness(data, width, height, channels, 50);

    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "\nBrightness time = " << duration.count() << " ms\n";

    start_time = std::chrono::high_resolution_clock::now();

    Export_Image(data, width, height, channels, "export.JPG");

    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "\nImage Write Time = " << duration.count() << " ms\n";
    QApplication app(argc, argv);
    MyMainWindow window;
    window.show();
    return app.exec();

}
