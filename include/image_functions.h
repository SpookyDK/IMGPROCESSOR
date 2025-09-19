#pragma once
#include <vector>
#include <list>
#include <string>

typedef enum {
    Success,
    ImageType_not_supported,
    Error_not_specified
} image_error_code;


enum Image_Type{JPG, BMP, PNG, RAW, CRAW};


enum Effect_Type{
                Crop,
                Scale,
                Temperature,
                RotateClock,
                RotateCounterClock,
                FlipX,
                FlipY,
                Brightness,
                Contrast,
                Saturation,
                Vibrancy
                };

std::string effectTypeToString(Effect_Type type);

struct ImageEffect{
    Effect_Type effect;
    std::vector<float> args;
    bool changed;
    bool imageCached;
    int cacheIndex;
    ImageEffect(Effect_Type _effect, std::vector<float> _args){
        effect = _effect;
        args = _args;
        changed = true;
        imageCached = false;
        cacheIndex = 0;
    }
};
typedef enum {
    IMG_UCHAR,
    IMG_FLOAT
} ImageType;

struct Image{
    void* data;
    ImageType type;
    int width, height, channels;
    Image(int _width, int _height, int _channels){
        width = _width;
        height = _height;
        channels = _channels;
    }
};


image_error_code Load_Image(const char* filepath, ImageType imageType, Image& image);

image_error_code Export_Image(Image image, const char* filepath);
image_error_code Rotate_Image_90_Counter(Image& image);

image_error_code Adjust_Brightness(Image& image, const int adjustment);
image_error_code Adjust_Brightness_SIMD(Image& image, const int adjustment, unsigned char* startAddress, const unsigned char* endAddress);
image_error_code Adjust_Contrast(Image& image, const float adjustment);
image_error_code Adjust_Temperature(Image& image, const float adjustment);
image_error_code Convert_Image_Format(Image& image, const ImageType type);
image_error_code Scale_Image(Image& image, const int outputWidth, const int outputHeight);

image_error_code Handle_Effects(std::list<ImageEffect>& Effects, std::vector<Image>& images, int stopPoint);

image_error_code Print_Effects(std::list<ImageEffect> effects);

char* image_error_code_2_char(image_error_code errorCode);
