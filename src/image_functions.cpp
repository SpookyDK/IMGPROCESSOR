#include "image_functions.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>
#include <iostream>
#include <algorithm>


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
void Adjust_Brightness(unsigned char* __restrict image, int& width, int& height, int& channels, int adjustmeant){
        int end = width * height * channels;
        int i = 0;
        while (i < end){
            int value0 = static_cast<int>(image[i]) + adjustmeant;
            int value1 = static_cast<int>(image[i + 1]) + adjustmeant;
            int value2 = static_cast<int>(image[i + 2]) + adjustmeant;
            image[i] = static_cast<char>(std::clamp(value0, 0, 255));
            image[i + 1] = static_cast<char>(std::clamp(value1, 0, 255));
            image[i + 2] = static_cast<char>(std::clamp(value2, 0, 255));
            i += channels;
        }
        return;
}
void Export_Image(const unsigned char* __restrict image, int& width, int& height, int&channels, const char* filepath){
    stbi_write_jpg(filepath, width, height, channels, image, 0);

}

