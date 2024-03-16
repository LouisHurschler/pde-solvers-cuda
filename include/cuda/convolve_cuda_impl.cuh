#ifndef CONVOLVE_CUDA_IMPL_H_
#define CONVOLVE_CUDA_IMPL_H_

#include <zisa/memory/array.hpp>
#include <stdio.h>
#include <iostream>

#ifndef THREAD_DIMS
#define THREAD_DIMS 1024
#endif

template <typename Scalar>
__global__ void
convolve_cuda_kernel(zisa::array_view<Scalar, 2> dst,
                     zisa::array_const_view<Scalar, 2> src,
                     zisa::array_const_view<Scalar, 2> kernel) {
  const int ghost_x = kernel.shape(0) / 2;
  const int ghost_y = kernel.shape(1) / 2;

  const int Nx = src.shape(0) - 2 * ghost_x;
  const int Ny = src.shape(1) - 2 * ghost_y;

  const int linear_idx = threadIdx.x + blockIdx.x * THREAD_DIMS;
  if (linear_idx >= Nx * Ny) {
    return;
  }
  const int x_idx = ghost_x +  linear_idx / Ny;
  const int y_idx = ghost_y + linear_idx % Ny;
  printf("convolve_cuda_kernel reached with x_idx = %u, y_idx = %u\n", x_idx, y_idx);
  return;
  dst(x_idx, y_idx) = 0.;
  for (int di = -ghost_x; di <= ghost_x; di++) {
    for (int dj = -ghost_y; dj <= ghost_y; dj++) {
      if (kernel(ghost_x + di, ghost_y + dj) != 0) {
        dst(x_idx, y_idx) += kernel(ghost_x + di, ghost_y + dj) * src(x_idx + di, y_idx + dj);
      }
    }
  }
  printf("convolve_cuda_kernel reached with x_idx = %u, y_idx = %u\n", x_idx, y_idx);
}

template <typename Scalar>
void convolve_cuda(zisa::array_view<Scalar, 2> dst,
                   const zisa::array_const_view<Scalar, 2> &src,
                   const zisa::array_const_view<Scalar, 2> &kernel) {
#if CUDA_AVAILABLE
  const int thread_dims = THREAD_DIMS;
  const int block_dims = std::ceil((double)(src.shape(0) * src.shape(1)) / thread_dims);
  convolve_cuda_kernel<<<block_dims, thread_dims>>>(dst, src, kernel);
  const auto error = cudaDeviceSynchronize();
  if (error != cudaSuccess) {
    std::cout << "Error in convolve_cuda: " << cudaGetErrorString(error) << std::endl;
  }
#endif // CUDA_AVAILABLE
}
#endif // CONVOLVE_CUDA_IMPL_H_
