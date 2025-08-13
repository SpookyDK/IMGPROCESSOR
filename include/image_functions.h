#pragma once

unsigned char* Load_Image(const char* filepath, int& width, int& height, int& channels);

void Export_Image(const unsigned char* __restrict image, int& width, int& height, int&channels, const char* filepath);

void Rotate_Image_90_Counter(unsigned char* __restrict image, int& width, int& height, int& channels);

void Adjust_Brightness(unsigned char* __restrict image, int& width, int& height, int& channels, int adjustmeant);

