/**
* Copyright (c) 2016-present, Facebook, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

// This is a simple translation from the old Caffe math interfaces. We aim to
// still keep it simple, so all platforms would be able to support it fairly
// easily.

#ifdef USE_MKLML_FOR_BLAS
// when USE_MKLML is defined, use MKLML cblas for GEMM
#include "mkl_cblas.h"
#define CBLAS_ENUM_DEFINED_H
#else
// We include the cblas header here so that we can obtain the macros from cblas.
extern "C" {
#include "core/framework/cblas.h"
}
#endif

#include "core/common/common.h"
#include "core/framework/tensor.h"

namespace onnxruntime {
namespace concurrency {
class ThreadPool;
}

enum StorageOrder {
  UNKNOWN = 0,
  NHWC = 1,
  NCHW = 2,
};

namespace math {

template <typename T, class Provider>
void Exp(int N, const T* x, T* y, Provider* provider);
template <typename T, class Provider>
void Log(int N, const T* x, T* y, Provider* provider);
template <typename T, class Provider>
void Sqr(int N, const T* x, T* y, Provider* provider);


#define DECLARE_BINARY_OP(name)                                                     \
  template <typename T, class Provider>                                             \
  void name(int N, const T* a, const T* b, T* y, Provider* provider);               \
  template <typename T, class Provider>                                             \
  void name##ToRow(int M, int N, const T* a, const T* b, T* y, Provider* provider); \
  template <typename T, class Provider>                                             \
  void name##ToRow(int M, int N, const T* x, T* y, Provider* provider);             \
  template <typename T, class Provider>                                             \
  void name##ToCol(int M, int N, const T* x, T* y, Provider* provider);

DECLARE_BINARY_OP(Add);
DECLARE_BINARY_OP(Sub);
DECLARE_BINARY_OP(Mul);
DECLARE_BINARY_OP(Div);

#undef DECLARE_BINARY_OP

// Compute the row-wise max of a N*D matrix X, and write it to a N
// dimensional vector y.
template <typename T, class Provider>
void RowwiseMax(int N, int D, const T* x, T* y,
                Provider* provider);

// Compute the row-wise sum of a N*D matrix X, and write it to a N
// dimensional vector y.
template <typename T, class Provider>
void RowwiseSum(int N, int D, const T* x, T* y,
                Provider* provider);

// Sum of vector x, and writes the result to a single value y.
template <typename T, class Provider>
void Sum(int N, const T* x, T* y, Provider* provider,
         Tensor* scratch_ptr = nullptr);

template <typename T, class Provider>
void Scale(int N, float alpha, const T* x, T* y, Provider* provider);

// Different from the Scale function above, if alpha is passed in
// as a pointer, we will assume that it lives on the correct execution provider,
// for example on GPU.
template <typename T, class Provider>
void Scale(int N, const float* alpha, const T* x, T* y, Provider* provider);

template <typename T>
void MatMul(
    int M,
    int N,
    int K,
    const T* A,
    const T* B,
    T* C, concurrency::ThreadPool* threadpool);

// Decaf gemm provides a simpler interface to the gemm functions, with the
// limitation that the data has to be contiguous in memory.
template <typename T, class Provider>
void Gemm(
    CBLAS_TRANSPOSE TransA,
    CBLAS_TRANSPOSE TransB,
    int64_t M,
    int64_t N,
    int64_t K,
    T alpha,
    const T* A,
    const T* B,
    T beta,
    T* C,
    Provider*);

// We also provide a gemm that has explicit lda, ldb and ldc specified.
// In most cases you probably want to use the function above, though.
template <typename T, class Provider>
void GemmEx(
    CBLAS_TRANSPOSE TransA,
    CBLAS_TRANSPOSE TransB,
    int M,
    int N,
    int K,
    T alpha,
    const T* A,
    int lda,
    const T* B,
    int ldb,
    T beta,
    T* C,
    int ldc,
    Provider*);

// Gemv always takes in a M*N matrix A, and depending on whether we set TransA
// to Trans, the output is:
// CblasNoTrans: x is an N dim vector and y is an M dim vector.
// CblasTrans:   x is an M dim vector and y is an N dim vector.
template <typename T, class Provider>
void Gemv(
    CBLAS_TRANSPOSE TransA,
    int M,
    int N,
    float alpha,
    const T* A,
    const T* x,
    float beta,
    T* y,
    Provider* provider);

template <typename T, class Provider>
void Set(int64_t N, T alpha, T* X, Provider* provider);

template <typename T, class Provider>
void Dot(int N, const T* a, const T* b, T* y, Provider* provider);

template <typename T, class Provider>
void Axpy(int N, float alpha, const T* x, T* y, Provider* provider);

// Different from the Axpy function above, if alpha is passed in
// as a pointer, we will assume that it lives on the correct execution provider,
// for example on GPU.
template <typename T, class Provider>
void Axpy(int N, const float* alpha, const T* x, T* y, Provider* provider);

template <typename T, int order>
struct Im2col {
  void operator()(
      const T* data_im,
      int64_t channels,
      int64_t height,
      int64_t width,
      int64_t kernel_h,
      int64_t kernel_w,
      int64_t dilation_h,
      int64_t dilation_w,
      int64_t pad_t,
      int64_t pad_l,
      int64_t pad_b,
      int64_t pad_r,
      int64_t stride_h,
      int64_t stride_w,
      T* data_col,
      T padding_value = 0);
};

template <typename T>
struct Im2col<T, StorageOrder::NCHW> {
  void operator()(
      const T* data_im,
      int64_t channels,
      int64_t height,
      int64_t width,
      int64_t kernel_h,
      int64_t kernel_w,
      int64_t dilation_h,
      int64_t dilation_w,
      int64_t pad_t,
      int64_t pad_l,
      int64_t pad_b,
      int64_t pad_r,
      int64_t stride_h,
      int64_t stride_w,
      T* data_col,
      T padding_value = 0);
};

template <typename T>
struct Im2col<T, StorageOrder::NHWC> {
  void operator()(
      const T* data_im,
      int64_t channels,
      int64_t height,
      int64_t width,
      int64_t kernel_h,
      int64_t kernel_w,
      int64_t dilation_h,
      int64_t dilation_w,
      int64_t pad_t,
      int64_t pad_l,
      int64_t pad_b,
      int64_t pad_r,
      int64_t stride_h,
      int64_t stride_w,
      T* data_col,
      T padding_value = 0);
  void operator()(
      const T* data_im,
      int64_t channels,
      int64_t input_h,
      int64_t input_w,
      int64_t kernel_h,
      int64_t kernel_w,
      int64_t dilation_h,
      int64_t dilation_w,
      int64_t pad_t,
      int64_t pad_l,
      int64_t stride_h,
      int64_t stride_w,
      int64_t output_w,
      int64_t output_start,
      int64_t output_count,
      T* data_col,
      T padding_value = 0);
};

template <typename T, int order>
struct Im2colNd {
  void operator()(
      const T* data_img,
      const int64_t* im_shape,
      const int64_t* col_shape,
      int64_t img_size,
      int64_t col_size,
      const int64_t* kernel_shape,
      const int64_t* stride,
      const int64_t* dilation,
      const int64_t* pad,
      int64_t N,
      T* data_col,
      bool accumulate_output = false,
      T padding_value = 0);
};

template <typename T>
struct Im2colNd<T, StorageOrder::NCHW> {
  void operator()(const T* data_img, const int64_t* im_shape, const int64_t* col_shape, int64_t /*img_size*/,
                  int64_t /*col_size*/, const int64_t* kernel_shape, const int64_t* stride, const int64_t* dilation,
                  const int64_t* pad, int64_t N, T* data_col, bool accumulate_output = false,
                  T padding_value = 0) {
    int64_t kernel_size = 1;
    for (int64_t i = 0; i < N; ++i) {
      kernel_size *= kernel_shape[i];
    }
    int64_t channels_col = col_shape[0];
    std::vector<int64_t> d_offset(N, 0);
    std::vector<int64_t> d_iter(N, 0);
    for (int64_t c_col = 0; c_col < channels_col; ++c_col) {
      // Loop over spatial axes in reverse order to compute a per-axis offset.
      int64_t offset = c_col;
      for (int64_t d_i = N - 1; d_i >= 0; --d_i) {
        if (d_i < N - 1) {
          offset /= kernel_shape[d_i + 1];
        }
        d_offset[d_i] = offset % kernel_shape[d_i];
      }
      for (bool incremented = true; incremented;) {
        // Loop over spatial axes in forward order to compute the indices in the
        // image and column, and whether the index lies in the padding.
        int64_t index_col = c_col;
        int64_t index_im = c_col / kernel_size;
        bool is_padding = false;
        for (int64_t d_i = 0; d_i < N; ++d_i) {
          int64_t d = d_iter[d_i];
          int64_t d_im = d * stride[d_i] - pad[d_i] + d_offset[d_i] * dilation[d_i];
          is_padding |= d_im < 0 || d_im >= im_shape[d_i + 1];
          index_col *= col_shape[d_i + 1];
          index_col += d;
          index_im *= im_shape[d_i + 1];
          index_im += d_im;
        }
        if (!accumulate_output) {
          if (is_padding) {
            data_col[index_col] = padding_value;
          } else {
            data_col[index_col] = data_img[index_im];
          }
        } else if (!is_padding) {  // col2im
          data_col[index_im] += data_img[index_col];
        }
        // Loop over spatial axes in reverse order to choose an index,
        // like counting.
        incremented = false;
        for (int64_t d_i = N - 1; d_i >= 0; --d_i) {
          int64_t d_max = col_shape[d_i + 1];
          ORT_ENFORCE(d_iter[d_i] < d_max);
          if (d_iter[d_i] == d_max - 1) {
            d_iter[d_i] = 0;
          } else {  // d_iter[d_i] < d_max - 1
            ++d_iter[d_i];
            incremented = true;
            break;
          }
        }
      }  // while(incremented) {
    }    // for (int c = 0; c < channels_col; ++c) {
  }
};

template <typename T, class Provider, int order>
void Col2imNd(
    const T* data_col,
    const int64_t* img_shape,
    const int64_t* col_shape,
    int64_t img_size,
    int64_t col_size,
    const int64_t* kernel_shape,
    const int64_t* stride,
    const int64_t* dilation,
    const int64_t* pad,
    int64_t N,
    T* data_img,
    Provider* provider);

template <typename T, class Provider, int order>
void Col2im(
    const T* data_col,
    int64_t channels,
    int64_t height,
    int64_t width,
    int64_t patch_h,
    int64_t patch_w,
    int64_t dilation_h,
    int64_t dilation_w,
    int64_t pad_t,
    int64_t pad_l,
    int64_t pad_b,
    int64_t pad_r,
    int64_t stride_h,
    int64_t stride_w,
    T* data_im,
    Provider* provider);

template <typename T, typename TypedCopy>
void CopyMatrix(
    int M,
    int N,
    const T* A,
    int lda,
    T* B,
    int ldb,
    TypedCopy copy) {
  {
    if (lda == N && ldb == N) {
      copy(A, B, static_cast<size_t>(N * M));
      return;
    }

    for (int i = 0; i < M; ++i) {
      copy(A + lda * i, B + ldb * i, static_cast<size_t>(N));
    }
  }
}

template <typename T, class Provider>
void CopyVector(int N, const T* A, T* B, Provider* provider);

// Function uses casting from int64_t to uint64_t to compare if value of
// parameter a is greater or equal to zero and lower than value of
// parameter b. The b parameter is of type signed and is always
// positive,
// therefore its value is always lower than 0x800... where casting
// negative value of a parameter converts it to value higher than
// 0x800...
// The casting allows to use one condition instead of two.
inline bool is_a_ge_zero_and_a_lt_b(int64_t a, int64_t b) {
  return static_cast<uint64_t>(a) < static_cast<uint64_t>(b);
}

// Calculates ceil(a / b). User must be careful to ensure that there
// is no overflow or underflow in the calculation.
template <typename T>
constexpr T divUp(T a, T b) {
  return (a + b - (T)1) / b;
}

// Rounds a up to the next highest multiple of b. User must be careful
// to ensure that there is no overflow or underflow in the calculation
// of divUp.
template <typename T>
constexpr T roundUp(T a, T b) {
  return divUp<T>(a, b) * b;
}

// Returns true if the given integer type is a power-of-2 (positive only)
// Note(jiayq): windows reported an error per
//     https://github.com/caffe2/caffe2/issues/997
// and as a result will make it a macro.
#ifdef _MSC_VER
#define integerIsPowerOf2(v) ((v) && !((v) & ((v)-1)))
#else   // _MSC_VER
template <typename T>
constexpr bool integerIsPowerOf2(T v) {
  return (v && !(v & (v - 1)));
}
#endif  // _MSC_VER

// Returns log2(n) for a positive integer type
template <typename T>
constexpr int integerLog2(T n, int p = 0) {
  return (n <= 1) ? p : integerLog2(n / 2, p + 1);
}

// Returns the next highest power-of-2 for an integer type
template <typename T>
constexpr T integerNextHighestPowerOf2(T v) {
  return (integerIsPowerOf2(v) ? (T)2 * v : ((T)1 << (integerLog2(v) + 1)));
}

// Rounds a up to the next highest multiple of b, which is power-of-2. User must be careful
// to ensure that there is no overflow or underflow in the calculation
// of divUp.
template <typename T, T b>
constexpr T roundUpPow2(T a) {
  return (a + (b - 1)) & (~(b - 1));
}

uint16_t floatToHalf(float f);

uint16_t doubleToHalf(double f);

float halfToFloat(uint16_t h);

}  // namespace math
}  // namespace onnxruntime
