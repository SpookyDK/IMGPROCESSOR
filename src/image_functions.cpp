#include "image_functions.h"
#include <qtypes.h>
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

image_error_code Load_Image(const char* filepath, const ImageType imageType, Image& image){

    unsigned char* raw_data = stbi_load(filepath, &image.width, &image.height, &image.channels, 3);
    if (!raw_data) {
            throw std::runtime_error(std::string("Failed to load image: ") + stbi_failure_reason());
    }

    if (imageType == IMG_UCHAR){

        using batch_type = xsimd::batch<unsigned char>;
        constexpr size_t alignment = batch_type::arch_type::alignment();

        size_t size_in_bytes = image.width * image.height * image.channels;
        unsigned char* aligned_data = static_cast<unsigned char*>(std::aligned_alloc(alignment, size_in_bytes));

        if (!aligned_data) {
            stbi_image_free(raw_data);
            throw std::runtime_error("Failed to allocate aligned memory");
        }
        std::memcpy(aligned_data, raw_data, size_in_bytes);
        stbi_image_free(raw_data);
        image.data = aligned_data;
        image.type = IMG_UCHAR;
        return Success;
    }else if (imageType == IMG_FLOAT){
        using batch_type = xsimd::batch<float>;
        constexpr size_t alignment = batch_type::arch_type::alignment();

        size_t size_in_bytes = image.width * image.height * image.channels * sizeof(float);
        float* aligned_data = static_cast<float*>(std::aligned_alloc(alignment, size_in_bytes));

        if (!aligned_data) {
            stbi_image_free(raw_data);
            throw std::runtime_error("Failed to allocate aligned memory");
        }
        std::memcpy(aligned_data, raw_data, size_in_bytes);
        stbi_image_free(raw_data);
        image.data = aligned_data;
        image.type = IMG_FLOAT;
        return Success;

    }


}
image_error_code Copy_Image(const Image& image_in, Image& image_out){
    if (image_in.type == IMG_UCHAR){
        using batch_type = xsimd::batch<unsigned char>;
        constexpr size_t alignment = batch_type::arch_type::alignment();

        size_t size_in_bytes = image_in.width * image_in.height * image_in.channels;
        unsigned char* aligned_data = static_cast<unsigned char*>(std::aligned_alloc(alignment, size_in_bytes));

        std::memcpy(aligned_data, image_in.data, size_in_bytes);
        image_out.data = aligned_data;
        image_out.width = image_in.width;
        image_out.height = image_in.height;
        image_out.channels = image_in.channels;
        image_out.type = image_in.type;
        return Success;
    }else if (image_in.type == IMG_FLOAT){
        using batch_type = xsimd::batch<float>;
        constexpr size_t alignment = batch_type::arch_type::alignment();

        size_t size_in_bytes = image_in.width * image_in.height * image_in.channels * sizeof(float);
        float* aligned_data = static_cast<float*>(std::aligned_alloc(alignment, size_in_bytes));

        std::memcpy(aligned_data, image_in.data, size_in_bytes);
        image_out.data = aligned_data;
        image_out.width = image_in.width;
        image_out.height = image_in.height;
        image_out.channels = image_in.channels;
        image_out.type = image_in.type;
        return Success;
    }else{return ImageType_not_supported;}
}

image_error_code Export_Image(const Image& image, const char* filepath){
    stbi_write_jpg(filepath, image.width, image.height, image.channels, image.data, 0);
    return Success;
}

image_error_code Convert_Image_Format(Image& image, const ImageType outType){
    const size_t size = image.width * image.height * image.channels;
    if (image.type == IMG_FLOAT){
        float* data = (float*)image.data;
        if (outType == IMG_FLOAT){
            image.data = data;
            image.type = IMG_FLOAT;
            return Success;
        }else if (outType == IMG_UCHAR){
            unsigned char convertedData[size];
            for (int i = 0; i < size; i++){
                convertedData[i] = (unsigned char)data[i];
            }
            free(data);
            image.data = convertedData;
            image.type = IMG_UCHAR;
            return Success;
        }
    }else if (image.type == IMG_UCHAR){
        unsigned char* data = (unsigned char*)image.data;

        if (outType == IMG_FLOAT){
            float convertedData[size];
            for (int i = 0; i < size; i++){
                convertedData[i] = (float)data[i];
            }
            image.data = convertedData;
            image.type = IMG_FLOAT;
            free(data);
            return Success;
        }else if (outType == IMG_UCHAR){
            image.data = data;
            image.type = IMG_UCHAR;
            return Success;
        }
    }
    return ImageType_not_supported;
}
image_error_code Rotate_Image_90_Counter(Image& image){
    if (image.data){
        if (image.type == IMG_UCHAR){
        unsigned char* data = (unsigned char*)image.data;
        unsigned char* __restrict rotated = (unsigned char*) malloc(image.width * image.height * image.channels);
        int y = 0;
        while (y < image.height){
            int x = 0;
            while (x < image.width){
                rotated[( (image.width - 1 - x) * image.height + y ) * image.channels] = data[(y * image.width + x) * image.channels];
                rotated[( (image.width - 1 - x) * image.height + y ) * image.channels + 1] = data[(y * image.width + x) * image.channels + 1];
                rotated[( (image.width - 1 - x) * image.height + y ) * image.channels + 2] = data[(y * image.width + x) * image.channels + 2];
                x++;
            }
            y++;
        }
        std::swap(image.width,image.height);
        int i = 0;
        while (i < image.width*image.height*image.channels){
            data[i] = rotated[i];
            data[i+1] = rotated[i+1];
            data[i+2] = rotated[i+2];
            i += image.channels;

        }
        delete[] rotated;
        }else if(image.type == IMG_FLOAT){
            float* data = (float*)image.data;
            float* __restrict rotated = (float*) malloc(image.width * image.height * image.channels);
            int y = 0;
            while (y < image.height){
                int x = 0;
                while (x < image.width){
                    rotated[( (image.width - 1 - x) * image.height + y ) * image.channels] = data[(y * image.width + x) * image.channels];
                    rotated[( (image.width - 1 - x) * image.height + y ) * image.channels + 1] = data[(y * image.width + x) * image.channels + 1];
                    rotated[( (image.width - 1 - x) * image.height + y ) * image.channels + 2] = data[(y * image.width + x) * image.channels + 2];
                    x++;
                }
                y++;
            }
            std::swap(image.width,image.height);
            int i = 0;
            while (i < image.width*image.height*image.channels){
                data[i] = rotated[i];
                data[i+1] = rotated[i+1];
                data[i+2] = rotated[i+2];
                i += image.channels;
            }
            delete[] rotated;
        }

        }
}

image_error_code Adjust_Brightness(Image& image, const int adjustment){ 
        unsigned char* data = (unsigned char*)image.data;
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
        return Success;
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
        return Success;
}

image_error_code Adjust_Contrast(Image& image, const float adjustment){
        const int size = image.width * image.height * image.channels;
        if (image.type == IMG_UCHAR){
            unsigned char* data = (unsigned char*)image.data;
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

            return Success;
        } else if (image.type == IMG_FLOAT){
            using batch_float = xsimd::batch<float>;
            batch_float adjustmentBatch(adjustment);
            const size_t xsimd_size = batch_float::size;
            float* data = (float*)image.data;
            float* end = data + size;
            float val;
            while (data + xsimd_size <= end){
                    batch_float DataBatch = batch_float::load_aligned(data);
                    batch_float result = DataBatch * adjustmentBatch;
                    result.store_aligned(data);
                    data += xsimd_size;

            }
            while (data < end) {
                    val = *data + adjustment;
                    if (val < 0) val = 0;
                    else if (val > 255) val = 255;
                    *data = (unsigned char)val;
                }
            return Success;
        }
        return ImageType_not_supported;
}

image_error_code Adjust_Temperature(Image& image, const float adjustment){
            const int size = image.width * image.height* image.channels;
            float adjustment_R = std::pow(2, adjustment);
            float adjustment_B = std::pow(2, -adjustment);
        if (image.type == IMG_UCHAR){
            unsigned char* data = (unsigned char*)image.data;
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
            return Success;
        }else if (image.type == IMG_FLOAT){
            return ImageType_not_supported;
            float* data = (float*)image.data;
            float* end = data + size;
            using batch_float = xsimd::batch<unsigned char>;
            batch_float adjustmentBatch_R(adjustment_R);
            batch_float adjustmentBatch_B(adjustment_R);
            int xsimd_size = batch_float::size;
            float val;
            while (data + xsimd_size <= end){
                    batch_float DataBatch = batch_float::load_aligned(data);
                    batch_float result = xsimd::sadd(DataBatch, adjustmentBatch_B);
                    result.store_aligned(data);
                    data += xsimd_size;

            }
            while (data < end) {
                    val = *data + adjustment;
                    if (val < 0) val = 0;
                    else if (val > 255) val = 255;
                    *data = (unsigned char)val;
                }
                return Success;
            }
}

image_error_code Scale_Image(Image& image, const int outputWidth, const int outputHeight) {

    if (image.type == IMG_UCHAR){
        using batch_type = xsimd::batch<unsigned char>;
        constexpr size_t alignment = batch_type::arch_type::alignment();

        size_t size_in_bytes = image.width * image.height * image.channels;
        unsigned char* scaled_data = static_cast<unsigned char*>(std::aligned_alloc(alignment, size_in_bytes));

        float xRatio = (outputWidth > 1) ? (float)(image.width  - 1) / (outputWidth  - 1) : 0.0f;
        float yRatio = (outputHeight > 1)? (float)(image.height - 1) / (outputHeight - 1): 0.0f;
        unsigned char* data = (unsigned char*)image.data;

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
                    float a = data[idx_a + 0];
                    float b = data[idx_b + 0];
                    float c = data[idx_c + 0];
                    float d = data[idx_d + 0];

                    float pixel = a * (1 - x_weight) * (1 - y_weight) +
                                  b * x_weight * (1 - y_weight) +
                                  c * (1 - x_weight) * y_weight +
                                  d * x_weight * y_weight;

                    scaled_data[outIdx + 0] = static_cast<unsigned char>(std::clamp(pixel, 0.0f, 255.0f));
                }

                {
                    float a = data[idx_a + 1];
                    float b = data[idx_b + 1];
                    float c = data[idx_c + 1];
                    float d = data[idx_d + 1];

                    float pixel = a * (1 - x_weight) * (1 - y_weight) +
                                  b * x_weight * (1 - y_weight) +
                                  c * (1 - x_weight) * y_weight +
                                  d * x_weight * y_weight;

                    scaled_data[outIdx + 1] = static_cast<unsigned char>(std::clamp(pixel, 0.0f, 255.0f));
                }

                {
                    float a = data[idx_a + 2];
                    float b = data[idx_b + 2];
                    float c = data[idx_c + 2];
                    float d = data[idx_d + 2];

                    float pixel = a * (1 - x_weight) * (1 - y_weight) +
                                  b * x_weight * (1 - y_weight) +
                                  c * (1 - x_weight) * y_weight +
                                  d * x_weight * y_weight;

                    scaled_data[outIdx + 2] = static_cast<unsigned char>(std::clamp(pixel, 0.0f, 255.0f));
                }
            }
        }

        free(image.data);  
        image.data = scaled_data;
        image.width = outputWidth;
        image.height = outputHeight;
        image.channels = 3; 
        return Success;
    }else if(image.type == IMG_FLOAT){
        return ImageType_not_supported;
    }else{
        return ImageType_not_supported;
    }

}

image_error_code Handle_Effects(std::list<ImageEffect>& Effects, std::vector<Image>& images, int stopPoint){

    std::cout << "__________________\n";
    auto timeStartHandleEffects = std::chrono::high_resolution_clock::now();
    auto savepoint = Effects.begin();
    Image workingImage = Image(images[0].width, images[0].height, images[0].channels);
    Copy_Image(images[0], workingImage);
        if (images.size() > 0){
            while (savepoint != Effects.end()){
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
                savepoint++;
            }
        }
        images.push_back(workingImage);
        return Success;
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
