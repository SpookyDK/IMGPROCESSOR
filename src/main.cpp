#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <numbers>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <iostream>
#include <iostream>
#include <chrono>

unsigned char* Load_Image(const char* filepath, int& width, int& height, int& channels){

    unsigned char* data = stbi_load(filepath, &width, &height, &channels, 3);
    if (!data) {
            throw std::runtime_error(std::string("Failed to load image: ") + stbi_failure_reason());
    }
    return data;
}
void Rotate_Image_90_Counter(unsigned char* __restrict image, int& width, int& height, int& channels){
    if (image){
        unsigned char* __restrict rotated = (unsigned char*) malloc(width * height * channels);
        int y = 0;
        while (y < height){
            int x = 0;
            while (x < width){
                rotated[( (width - 1 - x) * height + y ) * channels] = image[(y * width + x) * channels];
                rotated[( (width - 1 - x) * height + y ) * channels + 1] = image[(y * width + x) * channels + 1];
                rotated[( (width - 1 - x) * height + y ) * channels + 2] = image[(y * width + x) * channels + 2];
                x++;
            }
            y++;
        }
        std::swap(width,height);
        int i = 0;
        while (i < width*height*channels){
            image[i] = rotated[i];
            image[i+1] = rotated[i+1];
            image[i+2] = rotated[i+2];
            i += channels;

        }
    }
}



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

    // QApplication app(argc, argv);
    // QImage qimg(data, width, height, width * 3, QImage::Format_RGB888);
    // QLabel* label = new QLabel;
    // label->setPixmap(QPixmap::fromImage(qimg));
    // label->show();

    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "\nQT display time = " << duration.count() << " ms\n";

    // return app.exec();
}
