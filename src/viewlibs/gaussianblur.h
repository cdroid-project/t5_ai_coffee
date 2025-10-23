/*
 * @Author: Ricken
 * @Email: hackricken@gmail.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:35:49
 * @FilePath: /t5_ai_demo/src/viewlibs/gaussianblur.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by Ricken, All Rights Reserved. 
 * 
**/

#ifndef __GAUSSIANBLUR_H
#define __GAUSSIANBLUR_H

#include <stdint.h>
#ifdef __cplusplus
extern "C"{
#endif
// static void CalGaussianCoeff(float sigma, float *a0, float *a1, float *a2, float *a3, float *b1, float *b2,
//                       float *cprev, float *cnext);

// static void gaussianVertical(uint8_t *bufferPerLine, uint8_t *lpRowInitial,
//                       uint8_t *lpColInitial, int height, int width, int Channels, float a0a1,
//                       float a2a3, float b1b2, float cprev, float cnext);

// static void gaussianHorizontal(uint8_t *bufferPerLine, uint8_t *lpRowInitial,
//                         uint8_t *lpColumn, int width, int height, int Channels, int Nwidth,
//                         float a0a1, float a2a3, float b1b2, float cprev, float cnext);

// void GaussianBlurFilter(uint8_t *input , uint8_t *output, int Width, int Height, float GaussianSigma);
void GaussianBlurFilter(uint8_t *input , int Width, int Height, float GaussianSigma);
#ifdef __cplusplus
}
#endif

#endif //__GAUSSIANBLUR_H