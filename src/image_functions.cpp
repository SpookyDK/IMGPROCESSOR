#include "image_functions.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>
#include <iostream>
#include <algorithm>
#include <QImage>

std::string effectTypeToString(Effect_Type type) {
    switch (type) {
        case Brightness: return "Brightness";
        case Contrast:   return "Contrast";
        case Saturation: return "Saturation";
        case Vibrancy:   return "Vibrancy";
    }
    return "Unknown";
}

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

void Adjust_Brightness(Image image, int adjustmeant){
        int end = image.width * image.height * image.channels;
        int i = 0;
        while (i < end){
            int value0 = static_cast<int>(image.data[i]) + adjustmeant;
            int value1 = static_cast<int>(image.data[i + 1]) + adjustmeant;
            int value2 = static_cast<int>(image.data[i + 2]) + adjustmeant;
            image.data[i] = static_cast<char>(std::clamp(value0, 0, 255));
            image.data[i + 1] = static_cast<char>(std::clamp(value1, 0, 255));
            image.data[i + 2] = static_cast<char>(std::clamp(value2, 0, 255));
            i += image.channels;
        }
        return;
}

void Export_Image(const unsigned char* __restrict image, int& width, int& height, int&channels, const char* filepath){
    stbi_write_jpg(filepath, width, height, channels, image, 0);

}

void Handle_Effects(std::list<ImageEffect>& Effects, std::vector<Image>& images, int stopPoint){
    auto iterator = Effects.begin();
    auto savepoint = Effects.begin();
    while (iterator != Effects.end()){
        if (iterator-> changed){
            break;
        }
        if (iterator->imageCached && iterator->cacheIndex >=0 && iterator->cacheIndex < images.size()){
            savepoint = iterator;
        }
        ++iterator;
    }
    int itSinceSave = 0;
    int indexImage = savepoint->cacheIndex;
    if (indexImage < 0){
        indexImage = 0;
    }
    std::cout << "h"<< images[indexImage].height << "w"<< images[indexImage].width << "c"<< images[indexImage].channels << "\n"; 
    int imageSize = images[indexImage].height*images[indexImage].width*images[indexImage].channels;
    std::cout << "size = " << imageSize << "\n";
//Malloc copy last to temp;
    unsigned char* copy = new unsigned char[imageSize];
    std::memcpy(copy, images[indexImage].data, imageSize);
    Image workingImage(copy, images[indexImage].width, images[indexImage].height, images[indexImage].channels);
    // Print_Effects(Effects);

    std::cout << "_______________\n";
    while (savepoint != Effects.end()){
        switch(savepoint->effect){
            case Brightness :
                std::cout << "adjusting brightness\n";
                Adjust_Brightness(workingImage, 10);
                break;
            case Contrast :
                std::cout << "adjusting contrast\n";
                Adjust_Brightness(workingImage, -15);
                break;
            default:
                std::cout << "default\n";
                break;


        }

        if (itSinceSave > 4){
            unsigned char* copieddata = new unsigned char[imageSize];
            std::memcpy(copieddata, workingImage.data, imageSize);
            images.push_back(Image(copieddata, workingImage.width, workingImage.height, workingImage.channels));
            itSinceSave = 0;

            
        }

        itSinceSave++;
        std::cout << "corner value = " << workingImage.data[1] << "\n";
        savepoint->changed = false;
        ++savepoint;

    }
    //Add final image as last;
    std::cout<< "saving image\n";
    images.push_back(workingImage);
}

void Print_Effects(std::list<ImageEffect> effects){
    for (auto &effect : effects) {
            std::cout << effectTypeToString(effect.effect)
                      << " | changed: " << effect.changed
                      << " | cached: " << effect.imageCached
                      << " | args: ";
            for (auto arg : effect.args) {
                std::cout << arg << " ";
            }
            std::cout << "\n";
        }

}

