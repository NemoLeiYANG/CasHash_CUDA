#include "KeyFileReader.h"

#include <stdio.h>
#include <cuda_runtime.h>

void KeyFileReader::KeyFileReader() {
    siftArray_ = NULL;
    siftArrayPitch = 0;
}

void KeyFileReader::~KeyFileReader() {
    free(siftArray_);
}

void KeyFileReader::TempArrayAdjust(size_t newSize) {
    fprintf(stderr, "Adjusting temporary host array to %ld bytes\n", newSize);
    siftArray_ = realloc(siftArray_, newSize);
    if(!siftArray_) {
        fprintf(stderr, "Can't (re-)allocate host array!\n");
        exit(EXIT_FAILURE);
    }
    siftArraySize_ = newSize;
}

void KeyFileReader::Read(ImageDataDevice *imgDevice, const char *keyPath) {
    FILE *fp = fopen(keyPath, "r");
    if(fp == NULL) {
        fprintf(stderr, "Key file %s does not exist!\n", keyPath);
        exit(EXIT_FAILURE);
    }

    int cntPoint, cntDim;
    fscanf(fp, "%d%d", &cntPoint, &cntDim);
    if(cntDim != kDimSiftData) {
        fprintf(stderr, "Unsupported SIFT vector dimension %d, should be %d!\n", cntDim);
        exit(EXIT_FAILURE);
    }
    size_t requiredSize = cntPoint * cntDim * sizeof(int);
    if(requiredSize > siftArraySize_) {
        TempArrayAdjust(requiredSize);
    }
    imgDevice->cntPoint = cntPoint;
    for(int i = 0; i < cntPoint; i++) {
        int *rowVec = siftArray_ + i * cntDim * sizeof(int);
        for(int j = 0; j < 128; j++) {
            fscanf(fp, "%d", &rowVec[j]);
        }
    }
    cudaMalloc2D(&(imgDevice->siftArray), &(imgDevice->siftArrayPitch), sizeof(int)* cntDim, cntPoint);
    cudaMemcpy2DAsync(imgDevice->siftArray, imgDevice->siftArrayPitch, siftArray_, sizeof(int) * cntDim, sizeof(int) * cntDim, cntPoint, cudaMemcpyHostToDevice); // TODO set stream
    CUDA_CHECK_ERROR;
}
