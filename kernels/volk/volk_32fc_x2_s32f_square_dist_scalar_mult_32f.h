/* -*- c++ -*- */
/*
 * Copyright 2012, 2014, 2019 Free Software Foundation, Inc.
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
 * \page volk_32fc_x2_s32f_square_dist_scalar_mult_32f
 *
 * \b Overview
 *
 * Calculates the square distance between a single complex input for each
 * point in a complex vector scaled by a scalar value.
 *
 * <b>Dispatcher Prototype</b>
 * \code
 * void volk_32fc_x2_s32f_square_dist_scalar_mult_32f(float* target, lv_32fc_t* src0, lv_32fc_t* points, float scalar, unsigned int num_points)
 * \endcode
 *
 * \b Inputs
 * \li src0: The complex input. Only the first point is used.
 * \li points: A complex vector of reference points.
 * \li scalar: A float to scale the distances by
 * \li num_points: The number of data points.
 *
 * \b Outputs
 * \li target: A vector of distances between src0 and the vector of points.
 *
 * \b Example
 * Calculate the distance between an input and reference points in a square
 * 16-qam constellation. Normalize distances by the area of the constellation.
 * \code
 *   int N = 16;
 *   unsigned int alignment = volk_get_alignment();
 *   lv_32fc_t* constellation  = (lv_32fc_t*)volk_malloc(sizeof(lv_32fc_t)*N, alignment);
 *   lv_32fc_t* rx  = (lv_32fc_t*)volk_malloc(sizeof(lv_32fc_t)*N, alignment);
 *   float* out = (float*)volk_malloc(sizeof(float)*N, alignment);
 *   float const_vals[] = {-3, -1, 1, 3};
 *
 *   unsigned int jj = 0;
 *   for(unsigned int ii = 0; ii < N; ++ii){
 *       constellation[ii] = lv_cmake(const_vals[ii%4], const_vals[jj]);
 *       if((ii+1)%4 == 0) ++jj;
 *   }
 *
 *   *rx = lv_cmake(0.5f, 2.f);
 *   float scale = 1.f/64.f; // 1 / constellation area
 *
 *   volk_32fc_x2_s32f_square_dist_scalar_mult_32f(out, rx, constellation, scale, N);
 *
 *   printf("Distance from each constellation point:\n");
 *   for(unsigned int ii = 0; ii < N; ++ii){
 *       printf("%.4f  ", out[ii]);
 *       if((ii+1)%4 == 0) printf("\n");
 *   }
 *
 *   volk_free(rx);
 *   volk_free(constellation);
 *   volk_free(out);
 * \endcode
 */

#ifndef INCLUDED_volk_32fc_x2_s32f_square_dist_scalar_mult_32f_a_H
#define INCLUDED_volk_32fc_x2_s32f_square_dist_scalar_mult_32f_a_H

#include<volk/volk_complex.h>


static inline void
calculate_scaled_distances(float* target, const lv_32fc_t symbol, const lv_32fc_t* points,
                           const float scalar, const unsigned int num_points)
{
  lv_32fc_t diff;
  for(unsigned int i = 0; i < num_points; ++i) {
    /*
     * Calculate: |y - x|^2 * SNR_lin
     * Compare C++: *target++ = scalar * std::norm(symbol - *constellation++);
     */
    diff = symbol - *points++;
    *target++ = scalar * (lv_creal(diff) * lv_creal(diff) + lv_cimag(diff) * lv_cimag(diff));
  }
}


#ifdef LV_HAVE_AVX2
#include<immintrin.h>

static inline void
volk_32fc_x2_s32f_square_dist_scalar_mult_32f_a_avx2(float* target, lv_32fc_t* src0, 
                                                     lv_32fc_t* points, float scalar, 
                                                     unsigned int num_points)
{
  const unsigned int num_bytes = num_points*8;
  __m128 xmm0, xmm9, xmm10, xmm11;
  __m256 xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

  lv_32fc_t diff;
  memset(&diff, 0x0, 2*sizeof(float));

  int bound = num_bytes >> 6;
  int leftovers0 = (num_bytes >> 5) & 1;
  int leftovers1 = (num_bytes >> 4) & 1;
  int leftovers2 = (num_bytes >> 3) & 1;
  int i = 0;

  __m256i idx = _mm256_set_epi32(7,6,3,2,5,4,1,0);
  xmm1 = _mm256_setzero_ps();
  xmm2 = _mm256_load_ps((float*)&points[0]);
  xmm8 = _mm256_set1_ps(scalar);
  xmm11 = _mm256_extractf128_ps(xmm8,1);
  xmm0 = _mm_load_ps((float*)src0);
  xmm0 = _mm_permute_ps(xmm0, 0b01000100);
  xmm1 = _mm256_insertf128_ps(xmm1, xmm0, 0);
  xmm1 = _mm256_insertf128_ps(xmm1, xmm0, 1);
  xmm3 = _mm256_load_ps((float*)&points[4]);

  for(; i < bound; ++i) {
    xmm4 = _mm256_sub_ps(xmm1, xmm2);
    xmm5 = _mm256_sub_ps(xmm1, xmm3);
    points += 8;
    xmm6 = _mm256_mul_ps(xmm4, xmm4);
    xmm7 = _mm256_mul_ps(xmm5, xmm5);

    xmm2 = _mm256_load_ps((float*)&points[0]);

    xmm4 = _mm256_hadd_ps(xmm6, xmm7);
    xmm4 = _mm256_permutevar8x32_ps(xmm4, idx);

    xmm3 = _mm256_load_ps((float*)&points[4]);

    xmm4 = _mm256_mul_ps(xmm4, xmm8);

    _mm256_store_ps(target, xmm4);

    target += 8;
  }

  for(i = 0; i < leftovers0; ++i) {
    xmm2 = _mm256_load_ps((float*)&points[0]);

    xmm4 = _mm256_sub_ps(xmm1, xmm2);

    points += 4;

    xmm6 = _mm256_mul_ps(xmm4, xmm4);

    xmm4 = _mm256_hadd_ps(xmm6, xmm6);
    xmm4 = _mm256_permutevar8x32_ps(xmm4, idx);

    xmm4 = _mm256_mul_ps(xmm4, xmm8);

    xmm9 = _mm256_extractf128_ps(xmm4,1);
    _mm_store_ps(target,xmm9);

    target += 4;
  }

  for(i = 0; i < leftovers1; ++i) {
    xmm9 = _mm_load_ps((float*)&points[0]);

    xmm10 = _mm_sub_ps(xmm0, xmm9);

    points += 2;

    xmm9 = _mm_mul_ps(xmm10, xmm10);

    xmm10 = _mm_hadd_ps(xmm9, xmm9);

    xmm10 = _mm_mul_ps(xmm10, xmm11);

    _mm_storeh_pi((__m64*)target, xmm10);

    target += 2;
  }

  calculate_scaled_distances(target, src0[0], points, scalar, leftovers2);
}

#endif /*LV_HAVE_AVX2*/


#ifdef LV_HAVE_AVX
#include <immintrin.h>

static inline void
volk_32fc_x2_s32f_square_dist_scalar_mult_32f_a_avx(float *target, lv_32fc_t *src0, 
                                                    lv_32fc_t *points, float scalar, 
                                                    unsigned int num_points) {
  static const unsigned int work_size = 8;
  unsigned int avx_work_size = num_points / work_size * work_size;

  for (int i = 0; i < avx_work_size; i += work_size) {
    lv_32fc_t src = *src0;
    float src_real = lv_creal(src);
    float src_imag = lv_cimag(src);
    __m256 source = _mm256_setr_ps(src_real, src_imag, src_real, src_imag, src_real, src_imag, src_real, src_imag);
    __m256 points_low = _mm256_load_ps((const float *) points);
    __m256 points_high = _mm256_load_ps((const float *) (points + work_size / 2));
    __m256 difference_low = _mm256_sub_ps(source, points_low);
    __m256 difference_high = _mm256_sub_ps(source, points_high);

    difference_low = _mm256_mul_ps(difference_low, difference_low);
    difference_high = _mm256_mul_ps(difference_high, difference_high);

    __m256 magnitudes_squared = _mm256_hadd_ps(difference_low, difference_high);
    __m128 lower_magnitudes_squared_bottom = _mm256_extractf128_ps(magnitudes_squared, 0);
    __m128 upper_magnitudes_squared_top = _mm256_extractf128_ps(magnitudes_squared, 1);
    __m256 lower_magnitudes_squared = _mm256_castps128_ps256(lower_magnitudes_squared_bottom);

    lower_magnitudes_squared = _mm256_insertf128_ps(
            lower_magnitudes_squared, _mm_permute_ps(lower_magnitudes_squared_bottom, 0x4E), 1
    );

    __m256 upper_magnitudes_squared = _mm256_castps128_ps256(upper_magnitudes_squared_top);

    upper_magnitudes_squared = _mm256_insertf128_ps(upper_magnitudes_squared, upper_magnitudes_squared_top, 1);
    upper_magnitudes_squared_top = _mm_permute_ps(upper_magnitudes_squared_top, 0x4E);
    upper_magnitudes_squared = _mm256_insertf128_ps(upper_magnitudes_squared, upper_magnitudes_squared_top, 0);

    __m256 ordered_magnitudes_squared = _mm256_blend_ps(lower_magnitudes_squared, upper_magnitudes_squared, 0xCC);
    __m256 scalars = _mm256_set1_ps(scalar);
    __m256 output = _mm256_mul_ps(ordered_magnitudes_squared, scalars);

    _mm256_store_ps(target, output);
    target += work_size;
    points += work_size;
  }

  calculate_scaled_distances(target, src0[0], points, scalar, num_points - avx_work_size);
}

#endif /* LV_HAVE_AVX */


#ifdef LV_HAVE_SSE3
#include<xmmintrin.h>
#include<pmmintrin.h>

static inline void
volk_32fc_x2_s32f_square_dist_scalar_mult_32f_a_sse3(float* target, lv_32fc_t* src0, 
                                                     lv_32fc_t* points, float scalar, 
                                                     unsigned int num_points)
{
  const unsigned int num_bytes = num_points*8;

  __m128 xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

  int bound = num_bytes >> 5;
  int leftovers0 = (num_bytes >> 4) & 1;
  int leftovers1 = (num_bytes >> 3) & 1;

  xmm1 = _mm_setzero_ps();
  xmm1 = _mm_loadl_pi(xmm1, (__m64*)src0);
  xmm2 = _mm_load_ps((float*)&points[0]);
  xmm8 = _mm_load1_ps(&scalar);
  xmm1 = _mm_movelh_ps(xmm1, xmm1);
  xmm3 = _mm_load_ps((float*)&points[2]);

  for(int i = 0; i < bound - 1; ++i) {
    xmm4 = _mm_sub_ps(xmm1, xmm2);
    xmm5 = _mm_sub_ps(xmm1, xmm3);
    points += 4;
    xmm6 = _mm_mul_ps(xmm4, xmm4);
    xmm7 = _mm_mul_ps(xmm5, xmm5);

    xmm2 = _mm_load_ps((float*)&points[0]);

    xmm4 = _mm_hadd_ps(xmm6, xmm7);

    xmm3 = _mm_load_ps((float*)&points[2]);

    xmm4 = _mm_mul_ps(xmm4, xmm8);

    _mm_store_ps(target, xmm4);

    target += 4;
  }

  xmm4 = _mm_sub_ps(xmm1, xmm2);
  xmm5 = _mm_sub_ps(xmm1, xmm3);

  points += 4;
  xmm6 = _mm_mul_ps(xmm4, xmm4);
  xmm7 = _mm_mul_ps(xmm5, xmm5);

  xmm4 = _mm_hadd_ps(xmm6, xmm7);

  xmm4 = _mm_mul_ps(xmm4, xmm8);

  _mm_store_ps(target, xmm4);

  target += 4;

  for(int i = 0; i < leftovers0; ++i) {
    xmm2 = _mm_load_ps((float*)&points[0]);

    xmm4 = _mm_sub_ps(xmm1, xmm2);

    points += 2;

    xmm6 = _mm_mul_ps(xmm4, xmm4);

    xmm4 = _mm_hadd_ps(xmm6, xmm6);

    xmm4 = _mm_mul_ps(xmm4, xmm8);

    _mm_storeh_pi((__m64*)target, xmm4);

    target += 2;
  }

  calculate_scaled_distances(target, src0[0], points, scalar, leftovers1);
}

#endif /*LV_HAVE_SSE3*/


#ifdef LV_HAVE_GENERIC
static inline void
volk_32fc_x2_s32f_square_dist_scalar_mult_32f_generic(float* target, lv_32fc_t* src0, 
                                                      lv_32fc_t* points, float scalar, 
                                                      unsigned int num_points)
{
  const lv_32fc_t symbol = *src0;
  calculate_scaled_distances(target, symbol, points, scalar, num_points);
}

#endif /*LV_HAVE_GENERIC*/


#endif /*INCLUDED_volk_32fc_x2_s32f_square_dist_scalar_mult_32f_a_H*/

#ifndef INCLUDED_volk_32fc_x2_s32f_square_dist_scalar_mult_32f_u_H
#define INCLUDED_volk_32fc_x2_s32f_square_dist_scalar_mult_32f_u_H

#include<inttypes.h>
#include<stdio.h>
#include<volk/volk_complex.h>
#include <string.h>

#ifdef LV_HAVE_AVX2
#include<immintrin.h>

static inline void
volk_32fc_x2_s32f_square_dist_scalar_mult_32f_u_avx2(float* target, lv_32fc_t* src0, 
                                                     lv_32fc_t* points, float scalar, 
                                                     unsigned int num_points)
{
  const unsigned int num_bytes = num_points*8;
  __m128 xmm0, xmm9;
  __m256 xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

  int bound = num_bytes >> 6;
  int leftovers0 = (num_bytes >> 5) & 1;
  int leftovers1 = (num_bytes >> 3) & 0b11;
  int i = 0;

  __m256i idx = _mm256_set_epi32(7,6,3,2,5,4,1,0);
  xmm1 = _mm256_setzero_ps();
  xmm2 = _mm256_loadu_ps((float*)&points[0]);
  xmm8 = _mm256_set1_ps(scalar);
  xmm0 = _mm_loadu_ps((float*)src0);
  xmm0 = _mm_permute_ps(xmm0, 0b01000100);
  xmm1 = _mm256_insertf128_ps(xmm1, xmm0, 0);
  xmm1 = _mm256_insertf128_ps(xmm1, xmm0, 1);
  xmm3 = _mm256_loadu_ps((float*)&points[4]);

  for(; i < bound; ++i) {
    xmm4 = _mm256_sub_ps(xmm1, xmm2);
    xmm5 = _mm256_sub_ps(xmm1, xmm3);
    points += 8;
    xmm6 = _mm256_mul_ps(xmm4, xmm4);
    xmm7 = _mm256_mul_ps(xmm5, xmm5);

    xmm2 = _mm256_loadu_ps((float*)&points[0]);

    xmm4 = _mm256_hadd_ps(xmm6, xmm7);
    xmm4 = _mm256_permutevar8x32_ps(xmm4, idx);

    xmm3 = _mm256_loadu_ps((float*)&points[4]);

    xmm4 = _mm256_mul_ps(xmm4, xmm8);

    _mm256_storeu_ps(target, xmm4);

    target += 8;
  }

  for(i = 0; i < leftovers0; ++i) {
    xmm2 = _mm256_loadu_ps((float*)&points[0]);

    xmm4 = _mm256_sub_ps(xmm1, xmm2);

    points += 4;

    xmm6 = _mm256_mul_ps(xmm4, xmm4);

    xmm4 = _mm256_hadd_ps(xmm6, xmm6);
    xmm4 = _mm256_permutevar8x32_ps(xmm4, idx);

    xmm4 = _mm256_mul_ps(xmm4, xmm8);

    xmm9 = _mm256_extractf128_ps(xmm4,1);
    _mm_storeu_ps(target,xmm9);

    target += 4;
  }

  calculate_scaled_distances(target, src0[0], points, scalar, leftovers1);
}

#endif /*LV_HAVE_AVX2*/

#endif /*INCLUDED_volk_32fc_x2_s32f_square_dist_scalar_mult_32f_u_H*/
