#pragma once
#include <vector>

enum Image_Type{JPG, BMP, PNG, RAW, CRAW};


enum Effect_Type{
                Brightness,
                Contrast,
                Saturation,
                Vibrancy
                };

struct ImageEffects{
    std::vector<Effect_Type> effects;
    std::vector<std::vector<float>> args;
    std::vector<bool> changed;
    std::vector<bool> imageCached;
};


unsigned char* Load_Image(const char* filepath, int& width, int& height, int& channels);

void Export_Image(const unsigned char* __restrict image, int& width, int& height, int&channels, const char* filepath);

void Rotate_Image_90_Counter(unsigned char* __restrict image, int& width, int& height, int& channels);

void Adjust_Brightness(unsigned char* __restrict image, int& width, int& height, int& channels, int adjustmeant);


void Handle_Effects(ImageEffects Effects, std::vector<unsigned char*> images, int stopPoint);

