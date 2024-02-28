#ifndef CONVOLUTION_HPP_
#define CONVOLUTION_HPP_

#include <iostream>
#include <zisa/memory/array.hpp>

template <typename Scalar>
void convolve_cpu(zisa::array_view<Scalar, 2> dst,
                  const zisa::array_const_view<Scalar, 2> &src,
                  const zisa::array_const_view<Scalar, 2> &kernel) {
  std::cout << "hello from cpu" << std::endl;
  // TODO: Optimize
  const int ghost_x = kernel.shape(0) / 2;
  const int ghost_y = kernel.shape(1) / 2;
  const int Nx = src.shape(0) - 2 * ghost_x;
  const int Ny = src.shape(1) - 2 * ghost_y;
  for (int i = ghost_x; i < Nx + ghost_x; ++i) {
    for (int j = ghost_y; j < Ny + ghost_y; ++j) {
      // shouldn't dst(i, j) = src(i, j)?
      dst(i, j) = 0;
      for (int di = -ghost_x; di <= ghost_x; ++di) {
        for (int dj = -ghost_y; dj <= ghost_y; ++dj) {
          dst(i, j) += kernel(ghost_x + di, ghost_y + dj) * src(i + di, j + dj);
        }
      }
    }
  }
}


template <typename Scalar>
void convolve_cuda(zisa::array_view<Scalar, 2> dst,
                   const zisa::array_const_view<Scalar, 2> &src,
                   const zisa::array_const_view<Scalar, 2> &kernel) {
  #if ENABLE_CUDA
  const int thread_dims = 1024;
  const int block_dims = std::ceil((src.shape(0) * src.shape(1)) / thread_dims);
  convolve_cuda_kernel<<<block_dims, thread_dims>>>(dst, src, kernel);
  cudaDeviceSynchronize();
  #endif // ENABLE_CUDA
}

template <typename Scalar>
void convolve(zisa::array_view<Scalar, 2> dst,
              const zisa::array_const_view<Scalar, 2> &src,
              const zisa::array_const_view<Scalar, 2> &kernel) {
  const zisa::device_type memory_dst = dst.memory_location();
  const zisa::device_type memory_src = src.memory_location();
  const zisa::device_type memory_kernel = kernel.memory_location();

  if (!(memory_dst == memory_src && memory_src == memory_kernel)) {
    std::cerr << "Convolution: Inputs must be located on the same hardware\n";
    exit(1);
  }
  if (dst.shape() != src.shape()) {
    std::cerr
        << "Convolution: Input and output array must have the same shape\n";
    exit(1);
  }

  if (memory_dst == zisa::device_type::cpu) {
    convolve_cpu(dst, src, kernel);
  } else if (memory_dst == zisa::device_type::cuda) {
    convolve_cuda(dst, src, kernel);
  } else {
    std::cerr << "Convolution: Unknown device_type of inputs\n";
    exit(1);
  }
}

#endif
