#include "cudahelper.h"
#include <cuda_runtime.h>

int getCudaDeviceCount() {
    int count = 0;
    cudaGetDeviceCount(&count);
    return count;
}
