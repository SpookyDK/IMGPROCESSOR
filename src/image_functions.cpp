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
#include <xsimd/xsimd.hpp>


std::string effectTypeToString(Effect_Type type) {
    switch (type) {
        case Brightness: return "Brightness";
        case Contrast:   return "Contrast";
        case Saturation: return "Saturation";
        case Vibrancy:   return "Vibrancy";
    }
    return "Unknown";
}

void* Load_Image(const char* filepath, int& width, int& height, int& channels, ImageType imageType){

    unsigned char* raw_data = stbi_load(filepath, &width, &height, &channels, 3);
    if (!raw_data) {
            throw std::runtime_error(std::string("Failed to load image: ") + stbi_failure_reason());
    }

    if (imageType = IMG_UCHAR){

        using batch_type = xsimd::batch<unsigned char>;
        constexpr size_t alignment = batch_type::arch_type::alignment();

        size_t size_in_bytes = width * height * channels;
        unsigned char* aligned_data = static_cast<unsigned char*>(std::aligned_alloc(alignment, size_in_bytes));

        if (!aligned_data) {
            stbi_image_free(raw_data);
            throw std::runtime_error("Failed to allocate aligned memory");
        }
        std::memcpy(aligned_data, raw_data, size_in_bytes);
        stbi_image_free(raw_data);
        return aligned_data;
    }else if (imageType == IMG_FLOAT){
        using batch_type = xsimd::batch<float>;
        constexpr size_t alignment = batch_type::arch_type::alignment();

        size_t size_in_bytes = width * height * channels;
        float* aligned_data = static_cast<float*>(std::aligned_alloc(alignment, size_in_bytes));

        if (!aligned_data) {
            stbi_image_free(raw_data);
            throw std::runtime_error("Failed to allocate aligned memory");
        }
        std::memcpy(aligned_data, raw_data, size_in_bytes);
        stbi_image_free(raw_data);
        return aligned_data;

    }


}

image_error_code Export_Image(Image image, const char* filepath){
    stbi_write_jpg(filepath, image.width, image.height, image.channels, image.data, 0);
}

image_error_code Rotate_Image_90_Counter(Image& image){
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

image_error_code Adjust_Brightness(Image& image, const int adjustment){ 
        unsigned char* data = image.data;
        const int size = image.width * image.height * image.channels;
        unsigned char* end = data + size;
        int val;
        while (data + 12 <= end){
            for (int j = 0; j < 12; ++j, ++data) {
                val = *data + adjustment;
                *data = static_cast<unsigned char>((val & ~(val >> 31)) | (-(val > 255) & 255));
            }
        while (data < end) {
                val = *data + adjustment;
                *data++ = static_cast<unsigned char>((val & ~(val >> 31)) | (-(val > 255) & 255));
            }
        }
        return;
}

image_error_code Adjust_Brightness_SIMD(Image& image, const int adjustment, unsigned char* startAddress, const unsigned char* endAddress){ 
        using batch_uchar = xsimd::batch<unsigned char, xsimd::avx512bw>;
        batch_uchar adjustmentBatch((unsigned char)adjustment);
        unsigned char* data = startAddress;
        const size_t xsimd_size = batch_uchar::size;
        int val;
        while (data + xsimd_size <= endAddress){
                batch_uchar DataBatch = batch_uchar::load_aligned(data);
                batch_uchar result = xsimd::sadd(DataBatch, adjustmentBatch);
                result.store_aligned(data);
                data += xsimd_size;

        }
        while (data < endAddress) {
                val = *data + adjustment;
                if (val < 0) val = 0;
                else if (val > 255) val = 255;
                *data = (unsigned char)val;
            }
        return;
}

image_error_code Adjust_Contrast(Image& image, const float adjustment){
        unsigned char* data = image.data;
        const int size = image.width * image.height * image.channels;
        unsigned char* end = data + size;
        float offset = 128.0;
        int val;
        while (data + 12 <= end){
            for (int j = 0; j < 12; ++j, ++data){
                val = static_cast<int>( (((float)*data) - offset) * adjustment + offset);
                *data = static_cast<unsigned char>((val & ~(val >> 31)) | (-(val > 255) & 255));
            }
        while (data <= end){
                val = static_cast<int>( (((float)*data) - offset) * adjustment + offset);
                *data++ = static_cast<unsigned char>((val & ~(val >> 31)) | (-(val > 255) & 255));
            }
        }

        return;
}

image_error_code Adjust_Contrast_SIMD(Image& image, const float adjustment, unsigned char* startAddress, const unsigned char* endAddress){
        using batch_float = xsimd::batch<float>;
        using batch_char = xsimd::batch<unsigned char>;
        using batch_int = xsimd::batch<int>;
        batch_float adjustmentBatch(adjustment);
        unsigned char* data = startAddress;
        const size_t xsimd_size = batch_float::size;
        float offset = 128.0;
        batch_float offsetBatch(offset);
        int val;
        while (data + xsimd_size <= endAddress){
            batch_char rawData = batch_char::load_aligned(data);

            // Step 1: Convert batch_char to batch<int> (or batch<unsigned int>)
            batch_int intData = xsimd::batch_cast<int>(rawData);

            // Step 2: Convert batch<int> to batch<float>
            batch_float floatData = xsimd::to_float(intData);
            floatData = (floatData - offsetBatch) * adjustmentBatch + offsetBatch;

            // Clamp to [0, 255]
            floatData = xsimd::min(xsimd::max(floatData, batch_float(0.f)), batch_float(255.f));

            intData = xsimd::to_int(floatData);
            rawData = xsimd::narrow_cast<unsigned char>(intData);

            // Narrow to uint8_t and store
            xsimd::store_aligned(data, rawData);
            data += xsimd_size;
        }
        while (data <= endAddress){
                val = static_cast<int>( (((float)*data) - offset) * adjustment + offset);
                *data++ = static_cast<unsigned char>((val & ~(val >> 31)) | (-(val > 255) & 255));
            }

        return;
}
image_error_code Adjust_Temperature(Image& image, const float adjustment){
        unsigned char* data = image.data;
        const int size = image.width * image.height* image.channels;
        float adjustment_R = std::pow(2, adjustment);
        float adjustment_B = std::pow(2, -adjustment);
        unsigned char* end = data + size;
        while (data + 3 < end){
            int valueR = static_cast<int>((float)*data * adjustment_R);
            *data = static_cast<unsigned char>((valueR & ~(valueR >> 31)) | (-(valueR> 255) & 255));
            data++;
            data++;
            int valueB = static_cast<int>((float)*data * adjustment_B);
            *data = static_cast<unsigned char>((valueB & ~(valueB >> 31)) | (-(valueB > 255) & 255));
            data++;
        }
        return;
}

image_error_code Scale_Image(Image& image, const int outputWidth, const int outputHeight) {
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

            int idx_a = (yl * image.width + xl) * 3;
            int idx_b = (yl * image.width + xh) * 3;
            int idx_c = (yh * image.width + xl) * 3;
            int idx_d = (yh * image.width + xh) * 3;

            int outIdx = (i * outputWidth + j) * 3;

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

    free(image.data);  
    image.data = scaled;
    image.width = outputWidth;
    image.height = outputHeight;
    image.channels = 3; 
}

image_error_code Handle_Effects(std::list<ImageEffect>& Effects, std::vector<Image>& images, int stopPoint){

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
            if (!workingImage.data || imageSize == 0) {
                std::cerr << "Error: workingImage has no data!" << std::endl;
                std::cout << "Image empty\n" ;
                continue;
            }
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

image_error_code Print_Effects(std::list<ImageEffect> effects){
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

char* image_error_code_2_char(image_error_code errorCode){
    switch (errorCode) {
        case Success: return "Success";
        case ImageType_not_supported:   return "ImageType_not_supported";
        case Error_not_specified: return "Error_not_specified";
        case Vibrancy:   return "Vibrancy";
    }
    return "Unknown";
}
