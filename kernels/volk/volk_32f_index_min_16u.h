/* -*- c++ -*- */
/*
 * Copyright 2021 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/*!
 * \page volk_32f_index_min_16u
 *
 * \b Overview
 *
 * Returns Argmin_i x[i]. Finds and returns the index which contains
 * the fist minimum value in the given vector.
 *
 * Note that num_points is a uint32_t, but the return value is
 * uint16_t. Providing a vector larger than the max of a uint16_t
 * (65536) would miss anything outside of this boundary. The kernel
 * will check the length of num_points and cap it to this max value,
 * anyways.
 *
 * <b>Dispatcher Prototype</b>
 * \code
 * void volk_32f_index_min_16u(uint16_t* target, const float* source, uint32_t num_points)
 * \endcode
 *
 * \b Inputs
 * \li source: The input vector of floats.
 * \li num_points: The number of data points.
 *
 * \b Outputs
 * \li target: The index of the fist minimum value in the input buffer.
 *
 * \b Example
 * \code
 *   int N = 10;
 *   uint32_t alignment = volk_get_alignment();
 *   float* in = (float*)volk_malloc(sizeof(float)*N, alignment);
 *   uint16_t* out = (uint16_t*)volk_malloc(sizeof(uint16_t), alignment);
 *
 *   for(uint32_t ii = 0; ii < N; ++ii){
 *       float x = (float)ii;
 *       // a parabola with a minimum at x=4
 *       in[ii] = (x-4) * (x-4) - 5;
 *   }
 *
 *   volk_32f_index_min_16u(out, in, N);
 *
 *   printf("minimum is %1.2f at index %u\n", in[*out], *out);
 *
 *   volk_free(in);
 *   volk_free(out);
 * \endcode
 */

#ifndef INCLUDED_volk_32f_index_min_16u_a_H
#define INCLUDED_volk_32f_index_min_16u_a_H

#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <volk/volk_common.h>

#ifdef LV_HAVE_AVX
#include <immintrin.h>

static inline void
volk_32f_index_min_16u_a_avx(uint16_t* target, const float* source, uint32_t num_points)
{
    num_points = (num_points > USHRT_MAX) ? USHRT_MAX : num_points;
    const uint32_t eighthPoints = num_points / 8;

    float* inputPtr = (float*)source;

    __m256 indexIncrementValues = _mm256_set1_ps(8);
    __m256 currentIndexes = _mm256_set_ps(-1, -2, -3, -4, -5, -6, -7, -8);

    float min = source[0];
    float index = 0;
    __m256 minValues = _mm256_set1_ps(min);
    __m256 minValuesIndex = _mm256_setzero_ps();
    __m256 compareResults;
    __m256 currentValues;

    __VOLK_ATTR_ALIGNED(32) float minValuesBuffer[8];
    __VOLK_ATTR_ALIGNED(32) float minIndexesBuffer[8];

    for (uint32_t number = 0; number < eighthPoints; number++) {

        currentValues = _mm256_load_ps(inputPtr);
        inputPtr += 8;
        currentIndexes = _mm256_add_ps(currentIndexes, indexIncrementValues);

        compareResults = _mm256_cmp_ps(currentValues, minValues, _CMP_LT_OS);

        minValuesIndex = _mm256_blendv_ps(minValuesIndex, currentIndexes, compareResults);
        minValues = _mm256_blendv_ps(minValues, currentValues, compareResults);
    }

    // Calculate the smallest value from the remaining 4 points
    _mm256_store_ps(minValuesBuffer, minValues);
    _mm256_store_ps(minIndexesBuffer, minValuesIndex);

    for (uint32_t number = 0; number < 8; number++) {
        if (minValuesBuffer[number] < min) {
            index = minIndexesBuffer[number];
            min = minValuesBuffer[number];
        } else if (minValuesBuffer[number] == min) {
            if (index > minIndexesBuffer[number])
                index = minIndexesBuffer[number];
        }
    }

    for (uint32_t number = eighthPoints * 8; number < num_points; number++) {
        if (source[number] < min) {
            index = number;
            min = source[number];
        }
    }
    target[0] = (uint16_t)index;
}

#endif /*LV_HAVE_AVX*/

#ifdef LV_HAVE_SSE4_1
#include <smmintrin.h>

static inline void
volk_32f_index_min_16u_a_sse4_1(uint16_t* target, const float* source, uint32_t num_points)
{
    num_points = (num_points > USHRT_MAX) ? USHRT_MAX : num_points;
    const uint32_t quarterPoints = num_points / 4;

    float* inputPtr = (float*)source;

    __m128 indexIncrementValues = _mm_set1_ps(4);
    __m128 currentIndexes = _mm_set_ps(-1, -2, -3, -4);

    float min = source[0];
    float index = 0;
    __m128 minValues = _mm_set1_ps(min);
    __m128 minValuesIndex = _mm_setzero_ps();
    __m128 compareResults;
    __m128 currentValues;

    __VOLK_ATTR_ALIGNED(16) float minValuesBuffer[4];
    __VOLK_ATTR_ALIGNED(16) float minIndexesBuffer[4];

    for (uint32_t number = 0; number < quarterPoints; number++) {

        currentValues = _mm_load_ps(inputPtr);
        inputPtr += 4;
        currentIndexes = _mm_add_ps(currentIndexes, indexIncrementValues);

        compareResults = _mm_cmplt_ps(currentValues, minValues);

        minValuesIndex = _mm_blendv_ps(minValuesIndex, currentIndexes, compareResults);
        minValues = _mm_blendv_ps(minValues, currentValues, compareResults);
    }

    // Calculate the smallest value from the remaining 4 points
    _mm_store_ps(minValuesBuffer, minValues);
    _mm_store_ps(minIndexesBuffer, minValuesIndex);

    for (uint32_t number = 0; number < 4; number++) {
        if (minValuesBuffer[number] < min) {
            index = minIndexesBuffer[number];
            min = minValuesBuffer[number];
        } else if (minValuesBuffer[number] == min) {
            if (index > minIndexesBuffer[number])
                index = minIndexesBuffer[number];
        }
    }

    for (uint32_t number = quarterPoints * 4; number < num_points; number++) {
        if (source[number] < min) {
            index = number;
            min = source[number];
        }
    }
    target[0] = (uint16_t)index;
}

#endif /*LV_HAVE_SSE4_1*/


#ifdef LV_HAVE_SSE

#include <xmmintrin.h>

static inline void
volk_32f_index_min_16u_a_sse(uint16_t* target, const float* source, uint32_t num_points)
{
    num_points = (num_points > USHRT_MAX) ? USHRT_MAX : num_points;
    const uint32_t quarterPoints = num_points / 4;

    float* inputPtr = (float*)source;

    __m128 indexIncrementValues = _mm_set1_ps(4);
    __m128 currentIndexes = _mm_set_ps(-1, -2, -3, -4);

    float min = source[0];
    float index = 0;
    __m128 minValues = _mm_set1_ps(min);
    __m128 minValuesIndex = _mm_setzero_ps();
    __m128 compareResults;
    __m128 currentValues;

    __VOLK_ATTR_ALIGNED(16) float minValuesBuffer[4];
    __VOLK_ATTR_ALIGNED(16) float minIndexesBuffer[4];

    for (uint32_t number = 0; number < quarterPoints; number++) {

        currentValues = _mm_load_ps(inputPtr);
        inputPtr += 4;
        currentIndexes = _mm_add_ps(currentIndexes, indexIncrementValues);

        compareResults = _mm_cmplt_ps(currentValues, minValues);

        minValuesIndex = _mm_or_ps(_mm_and_ps(compareResults, currentIndexes),
                                   _mm_andnot_ps(compareResults, minValuesIndex));
        minValues = _mm_or_ps(_mm_and_ps(compareResults, currentValues),
                              _mm_andnot_ps(compareResults, minValues));
    }

    // Calculate the smallest value from the remaining 4 points
    _mm_store_ps(minValuesBuffer, minValues);
    _mm_store_ps(minIndexesBuffer, minValuesIndex);

    for (uint32_t number = 0; number < 4; number++) {
        if (minValuesBuffer[number] < min) {
            index = minIndexesBuffer[number];
            min = minValuesBuffer[number];
        } else if (minValuesBuffer[number] == min) {
            if (index > minIndexesBuffer[number])
                index = minIndexesBuffer[number];
        }
    }

    for (uint32_t number = quarterPoints * 4; number < num_points; number++) {
        if (source[number] < min) {
            index = number;
            min = source[number];
        }
    }
    target[0] = (uint16_t)index;
}

#endif /*LV_HAVE_SSE*/


#ifdef LV_HAVE_GENERIC

static inline void
volk_32f_index_min_16u_generic(uint16_t* target, const float* source, uint32_t num_points)
{
    num_points = (num_points > USHRT_MAX) ? USHRT_MAX : num_points;

    float min = source[0];
    uint16_t index = 0;

    for (uint32_t i = 1; i < num_points; ++i) {
        if (source[i] < min) {
            index = i;
            min = source[i];
        }
    }
    target[0] = index;
}

#endif /*LV_HAVE_GENERIC*/


#endif /*INCLUDED_volk_32f_index_min_16u_a_H*/


#ifndef INCLUDED_volk_32f_index_min_16u_u_H
#define INCLUDED_volk_32f_index_min_16u_u_H

#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <volk/volk_common.h>

#ifdef LV_HAVE_AVX
#include <immintrin.h>

static inline void
volk_32f_index_min_16u_u_avx(uint16_t* target, const float* source, uint32_t num_points)
{
    num_points = (num_points > USHRT_MAX) ? USHRT_MAX : num_points;
    const uint32_t eighthPoints = num_points / 8;

    float* inputPtr = (float*)source;

    __m256 indexIncrementValues = _mm256_set1_ps(8);
    __m256 currentIndexes = _mm256_set_ps(-1, -2, -3, -4, -5, -6, -7, -8);

    float min = source[0];
    float index = 0;
    __m256 minValues = _mm256_set1_ps(min);
    __m256 minValuesIndex = _mm256_setzero_ps();
    __m256 compareResults;
    __m256 currentValues;

    __VOLK_ATTR_ALIGNED(32) float minValuesBuffer[8];
    __VOLK_ATTR_ALIGNED(32) float minIndexesBuffer[8];

    for (uint32_t number = 0; number < eighthPoints; number++) {

        currentValues = _mm256_loadu_ps(inputPtr);
        inputPtr += 8;
        currentIndexes = _mm256_add_ps(currentIndexes, indexIncrementValues);

        compareResults = _mm256_cmp_ps(currentValues, minValues, _CMP_LT_OS);

        minValuesIndex = _mm256_blendv_ps(minValuesIndex, currentIndexes, compareResults);
        minValues = _mm256_blendv_ps(minValues, currentValues, compareResults);
    }

    // Calculate the smallest value from the remaining 4 points
    _mm256_storeu_ps(minValuesBuffer, minValues);
    _mm256_storeu_ps(minIndexesBuffer, minValuesIndex);

    for (uint32_t number = 0; number < 8; number++) {
        if (minValuesBuffer[number] < min) {
            index = minIndexesBuffer[number];
            min = minValuesBuffer[number];
        } else if (minValuesBuffer[number] == min) {
            if (index > minIndexesBuffer[number])
                index = minIndexesBuffer[number];
        }
    }

    for (uint32_t number = eighthPoints * 8; number < num_points; number++) {
        if (source[number] < min) {
            index = number;
            min = source[number];
        }
    }
    target[0] = (uint16_t)index;
}

#endif /*LV_HAVE_AVX*/

#endif /*INCLUDED_volk_32f_index_min_16u_u_H*/
