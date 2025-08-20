#include "image_functions.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>
#include <iostream>
#include <algorithm>
#include <QImage>
#include <chrono>

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

void Export_Image(Image image, const char* filepath){
    stbi_write_jpg(filepath, image.width, image.height, image.channels, image.data, 0);
}

void Rotate_Image_90_Counter(Image& image){
    if (image.data){
        unsigned char* __restrict rotated = (unsigned char*) malloc(image.width * image.height * image.channels);
        int y = 0;
        while (y < image.height){
            int x = 0;
            while (x < image.width){
                rotated[( (image.width - 1 - x) * image.height + y ) * image.channels] = image.data[(y * image.width + x) * image.channels];
                rotated[( (image.width - 1 - x) * image.height + y ) * image.channels + 1] = image.data[(y * image.width + x) * image.channels + 1];
                rotated[( (image.width - 1 - x) * image.height + y ) * image.channels + 2] = image.data[(y * image.width + x) * image.channels + 2];
                x++;
            }
            y++;
        }
        std::swap(image.width,image.height);
        int i = 0;
        while (i < image.width*image.height*image.channels){
            image.data[i] = rotated[i];
            image.data[i+1] = rotated[i+1];
            image.data[i+2] = rotated[i+2];
            i += image.channels;

        }
    }
}

void Adjust_Brightness(Image& image, int adjustmeant){
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


void Handle_Effects(std::list<ImageEffect>& Effects, std::vector<Image>& images, int stopPoint){

    std::cout << "__________________\n";
    auto timeStartHandleEffects = std::chrono::high_resolution_clock::now();
    auto iterator = Effects.begin();
    auto savepoint = Effects.begin();
    if (images.size() > 0){
    while (iterator != Effects.end()){
        if (iterator-> changed){
            break;
        }
        if (iterator->imageCached && iterator->cacheIndex > 0 && iterator->cacheIndex < images.size()){
            savepoint = iterator;
        }
        ++iterator;
    }
    int itSinceSave = 0;
    int indexImage = savepoint->cacheIndex;
    std::cout << "CACHE INDEX = " << indexImage << "\n";
    if (indexImage == 0){
        std::cout << "RESETING CACHE\n";
        int i = 1;
        while (i < images.size() -1){
            delete[] images[i].data;
            i++;
        }
        images.erase(images.begin() +1, images.end()); 
        std::cout << "NEW CACHE SIZE = " << images.size() << "\n";

    }else if (indexImage != images.size()-1){
        std::cout << "MINIMIZING CACHE\n" << images.size();
        int i = indexImage + 1;
        while (i < images.size() -1){
            delete[] images[i].data;
            i++;
        }
        images.erase(images.begin() + indexImage, images.end()); 
        std::cout << "NEW CACHE SIZE = " << images.size() << "\n";
    }
    auto timeEndHandleEffects = std::chrono::high_resolution_clock::now();
    int timeHandleEffects = std::chrono::duration_cast<std::chrono::milliseconds>(timeEndHandleEffects - timeStartHandleEffects).count();
    std::cout << "TIME FOR EFFECT LIST = " << timeHandleEffects << "ms\n";
    int imageSize = images[indexImage].height*images[indexImage].width*images[indexImage].channels;
    //Malloc copy last to temp;
    unsigned char* copy = new unsigned char[imageSize];
    std::memcpy(copy, images[indexImage].data, imageSize);
    Image workingImage(copy, images[indexImage].width, images[indexImage].height, images[indexImage].channels);
    // Print_Effects(Effects);
    auto timeSwitchStart = std::chrono::high_resolution_clock::now();
    auto timeSwitchEnd = std::chrono::high_resolution_clock::now();
    while (savepoint != Effects.end()){
        timeSwitchStart = std::chrono::high_resolution_clock::now();
        switch(savepoint->effect){
            case Brightness :
                std::cout << "\nADJUSTING BRIGHTNESS\n";
                Adjust_Brightness(workingImage, 10);
                break;
            case Contrast :
                std::cout << "\nADJUSTING CONTRAST\n";
                Adjust_Brightness(workingImage, -15);
                break;
            case RotateCounterClock:
                std::cout << "\nROTATION 90 COUNTER\n";
                Rotate_Image_90_Counter(workingImage);
                break;
            default:
                std::cout << "\nDEFAULT\n";
                break;
        }
        auto timeSwitchEnd = std::chrono::high_resolution_clock::now();
        int switchTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeSwitchEnd - timeSwitchStart).count();
        std::cout << "TIME FOR EFFECT = " << switchTime << "ms\n";
        std::cout << "PIXEL PR MS = " << imageSize / switchTime << "\n";
        std::cout << "IT SINCE SAVE = " << itSinceSave << "\n\n";

        


        if (itSinceSave > 2){
            std::cout << "SAVING CACHE....\n";
            unsigned char* copieddata = new unsigned char[imageSize];
            std::memcpy(copieddata, workingImage.data, imageSize);
            images.push_back(Image(copieddata, workingImage.width, workingImage.height, workingImage.channels));
            savepoint->cacheIndex = images.size()-1;
            savepoint->imageCached = true;
            itSinceSave = 0;

            
        }

        itSinceSave++;
        savepoint->changed = false;
        ++savepoint;

    }
    if (images.size() == 1){
        images.push_back(workingImage);
    }else{
        images.back() = workingImage;
    }
    //Add final image as last;
    std::cout<< "TOTAL IMAGES CACHED=  "<< images.size() << "\n";

    auto timeEndEffects = std::chrono::high_resolution_clock::now();
    std::cout << "TOTAL PROCESSING TIME = " << timeEndEffects - timeStartHandleEffects << "\n";
    }
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

