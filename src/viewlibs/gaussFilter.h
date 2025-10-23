/*
 * @Author: Ricken
 * @Email: hackricken@gmail.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:37:22
 * @FilePath: /t5_ai_coffee/src/viewlibs/gaussFilter.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by Ricken, All Rights Reserved. 
 * 
**/



#ifndef XAGISPALGORITHM_GAUSSFILTER_H
#define XAGISPALGORITHM_GAUSSFILTER_H

#ifdef CDROID_X64
#include "NEON_2_SSE.h"
#endif

#ifdef __cplusplus
extern "C"{
#endif

typedef unsigned char U8;

/**
 * @brief 高斯滤波浮点C实现
 * @param src
 * @param dst
 * @param row
 * @param col
 * @param channel
 * @param ksize
 * @param sigma
 */
void GaussianFilter(unsigned char *src, unsigned char* dst, int row, int col, int channel, int ksize, float sigma);

/**
 * @brief 核分离高斯滤波浮点C实现
 * @param src
 * @param dst
 * @param row
 * @param col
 * @param channel
 * @param ksize
 * @param sigma
 */
void separateGaussianFilter(unsigned char *src, unsigned char* dst, int row, int col, int channel, int ksize, float sigma);

/**
 * @brief 核分离高斯滤波定点型NEON实现
 * @param src
 * @param dst
 * @param height
 * @param width
 * @param channel
 * @param ksize
 * @param sigma
 */
void gaussianFilter_u8_Neon(U8* src, U8* dst, int height, int width, int channel, int ksize);

/**
 * @brief 核分离高斯滤波浮点型NEON实现
 * @param src
 * @param dst
 * @param height
 * @param width
 * @param channel
 * @param ksize
 * @param sigma
 */
void gaussianFilter_float_Neon(float* src, float* dst, int height, int width, int channel, int ksize, float sigma);


#ifdef __cplusplus
}
#endif

#endif //XAGISPALGORITHM_GAUSSFILTER_H
