#pragma once
#include <vector>
#include <list>
#include <string>

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
struct Image{
    unsigned char* data;
    int width, height, channels;
    Image(unsigned char* _data, int _width, int _height, int _channels){
        data = _data;
        width = _width;
        height = _height;
        channels = _channels;
    }
};


unsigned char* Load_Image(const char* filepath, int& width, int& height, int& channels);

void Export_Image(Image image, const char* filepath);

void Rotate_Image_90_Counter(Image& image);

void Adjust_Brightness(Image& image, int adjustment);
void Adjust_Contrast(Image& image, float adjustment);
void Adjust_Temperature(Image& image, float adjustment);

void Scale_Image(Image& image, int outputWidth, int outputHeight);

void Handle_Effects(std::list<ImageEffect>& Effects, std::vector<Image>& images, int stopPoint);

void Print_Effects(std::list<ImageEffect> effects);
