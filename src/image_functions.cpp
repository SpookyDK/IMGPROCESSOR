#include "image_functions.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>
#include <iostream>
#include <algorithm>
#include <QImage>
#include <QDoubleSpinBox>
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
        delete[] rotated;
    }

}

void Adjust_Brightness(Image& image, int adjustment){
        int end = image.width * image.height * image.channels;
        int i = 0;
        while (i < end){
            int value0 = static_cast<int>(image.data[i]) + adjustment;
            int value1 = static_cast<int>(image.data[i + 1]) + adjustment;
            int value2 = static_cast<int>(image.data[i + 2]) + adjustment;
            image.data[i] = static_cast<unsigned char>(std::clamp(value0, 0, 255));
            image.data[i + 1] = static_cast<unsigned char>(std::clamp(value1, 0, 255));
            image.data[i + 2] = static_cast<unsigned char>(std::clamp(value2, 0, 255));
            i += image.channels;
        }
        return;
}


void Adjust_Contrast(Image& image, float adjustment){
        int end = image.width * image.height * image.channels;
        int i = 0;
        while (i < end){
            int value0 = static_cast<int>( (((float)image.data[i]) - 128) * adjustment +128);
            int value1 = static_cast<int>( (((float)image.data[i+1]) - 128) * adjustment +128);
            int value2 = static_cast<int>( (((float)image.data[i+2]) - 128) * adjustment +128);
            image.data[i] = static_cast<unsigned char>(std::clamp(value0, 0, 255));
            image.data[i + 1] = static_cast<unsigned char>(std::clamp(value1, 0, 255));
            image.data[i + 2] = static_cast<unsigned char>(std::clamp(value2, 0, 255));
            i += image.channels;
        }
        return;
}
void Adjust_Temperature(Image& image, float adjustment){
        int end = image.width * image.height * image.channels;
        int i = 0;
        while (i < end){
            int valueR = static_cast<int>( ((((float)image.data[i]) / 255.0) * std::pow(2, adjustment)) * 255);
            int valueB = static_cast<int>( ((((float)image.data[i+2]) / 255.0) * std::pow(2, -adjustment)) * 255);
            image.data[i] = static_cast<unsigned char>(std::clamp(valueR, 0, 255));
            image.data[i + 2] = static_cast<unsigned char>(std::clamp(valueB, 0, 255));
            i += image.channels;
        }
        return;
}

void Scale_Image(Image& image, int outputWidth, int outputHeight) {
    // allocate new buffer
    unsigned char* scaled = (unsigned char*) malloc(outputWidth * outputHeight * 3);

    float xRatio = (outputWidth > 1) ? (float)(image.width  - 1) / (outputWidth  - 1) : 0.0f;
    float yRatio = (outputHeight > 1)? (float)(image.height - 1) / (outputHeight - 1): 0.0f;

    for (int i = 0; i < outputHeight; i++) {
        for (int j = 0; j < outputWidth; j++) {
            int xl = (int)floor(xRatio * j);
            int yl = (int)floor(yRatio * i);
            int xh = std::min(xl + 1, image.width  - 1);
            int yh = std::min(yl + 1, image.height - 1);

            float x_weight = (xRatio * j) - xl;
            float y_weight = (yRatio * i) - yl;

            // base indices for the four neighbors
            int idx_a = (yl * image.width + xl) * 3;
            int idx_b = (yl * image.width + xh) * 3;
            int idx_c = (yh * image.width + xl) * 3;
            int idx_d = (yh * image.width + xh) * 3;

            // output index
            int outIdx = (i * outputWidth + j) * 3;

            // ----- Red channel -----
            {
                float a = image.data[idx_a + 0];
                float b = image.data[idx_b + 0];
                float c = image.data[idx_c + 0];
                float d = image.data[idx_d + 0];

                float pixel = a * (1 - x_weight) * (1 - y_weight) +
                              b * x_weight * (1 - y_weight) +
                              c * (1 - x_weight) * y_weight +
                              d * x_weight * y_weight;

                scaled[outIdx + 0] = static_cast<unsigned char>(std::clamp(pixel, 0.0f, 255.0f));
            }

            // ----- Green channel -----
            {
                float a = image.data[idx_a + 1];
                float b = image.data[idx_b + 1];
                float c = image.data[idx_c + 1];
                float d = image.data[idx_d + 1];

                float pixel = a * (1 - x_weight) * (1 - y_weight) +
                              b * x_weight * (1 - y_weight) +
                              c * (1 - x_weight) * y_weight +
                              d * x_weight * y_weight;

                scaled[outIdx + 1] = static_cast<unsigned char>(std::clamp(pixel, 0.0f, 255.0f));
            }

            // ----- Blue channel -----
            {
                float a = image.data[idx_a + 2];
                float b = image.data[idx_b + 2];
                float c = image.data[idx_c + 2];
                float d = image.data[idx_d + 2];

                float pixel = a * (1 - x_weight) * (1 - y_weight) +
                              b * x_weight * (1 - y_weight) +
                              c * (1 - x_weight) * y_weight +
                              d * x_weight * y_weight;

                scaled[outIdx + 2] = static_cast<unsigned char>(std::clamp(pixel, 0.0f, 255.0f));
            }
        }
    }

    free(image.data);  // original was malloc'ed
    image.data = scaled;
    image.width = outputWidth;
    image.height = outputHeight;
    image.channels = 3; // enforce
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
                Adjust_Brightness(workingImage, savepoint->args[0]);
                break;
            case Contrast :
                std::cout << "\nADJUSTING CONTRAST\n";
                Adjust_Contrast(workingImage, savepoint->args[0]);
                break;
            case RotateCounterClock:
                std::cout << "\nROTATION 90 COUNTER\n";
                Rotate_Image_90_Counter(workingImage);
                break;
            case Scale :
                std::cout << "\nSCALING IMAGE\n";
                Scale_Image(workingImage, static_cast<int>(savepoint->args[0]), static_cast<int>(savepoint->args[1]));
                break;
            case Temperature :
                std::cout << "\nADJUSTING TEMPERATURE\n";
                Adjust_Temperature(workingImage,savepoint->args[0]);
                break;
            default:
                std::cout << "\nDEFAULT\n";
                break;
        }
        auto timeSwitchEnd = std::chrono::high_resolution_clock::now();
        int switchTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeSwitchEnd - timeSwitchStart).count();
        std::cout << "TIME FOR EFFECT = " << switchTime << "ms\n";
        if (switchTime != 0){
        std::cout << "PIXEL PR MS = " << imageSize / switchTime << "\n";}
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

