#ifndef PERIODIC_BC_CUDA_IMPL_H_
#define PERIODIC_BC_CUDA_IMPL_H_

#include <zisa/memory/array.hpp>

#ifndef THREAD_DIMS
#define THREAD_DIMS 1024
#endif

template <typename Scalar>
__global__ void
periodic_bc_cuda_kernel(zisa::array<Scalar, 2> &data,
                     unsigned n_ghost_cells_x,
                     unsigned n_ghost_cells_y, unsigned data_size) {
  const unsigned idx = blockIdx.x * THREAD_DIMS + threadIdx.x;
  if (idx < data_size) {
    if (idx < n_ghost_cells_x * data.shape(1)) {
      // upper boundary
      const unsigned x_idx = idx / data.shape(1);
      const unsigned y_idx = idx - data.shape(1) * x_idx;
      const unsigned x_idx_to_copy = data.shape(0) - 2 * n_ghost_cells_x + x_idx;
      data(x_idx, y_idx) = data(x_idx_to_copy, y_idx);
      return;
    }
    else if (id < n_ghost_cells_x * data.shape(1) + n_ghost_cells_y * (data.shape(0) - 2 * n_ghost_cells_y) * 2) {
      // left or right boundary
    }
  }
}

template <typename Scalar>
void periodic_bc_cuda(zisa::array<Scalar, 2> &data,
                   unsigned n_ghost_cells_x,
                   unsigned n_ghost_cells_y) {
#if CUDA_AVAILABLE
  const unsigned thread_dims = THTHREAD_DIMS;
  // size of whole boundary where periodic bc has to be applied
  const unsigned data_size = data.shape(1) * n_ghost_cells_x * 2 + (data.shape(0) - 2 * n_ghost_cells_x) * n_ghost_cells_y * 2;
  const unsigned block_dims = std::ceil(data_size) / thread_dims);
  periodic_bc_cuda_kernel<<<block_dims, thread_dims>>>(data, n_ghost_cells_x, n_ghost_cells_y, data_size);
  cudaDeviceSynchronize();
#endif // CUDA_AVAILABLE
}
#endif // PERIODIC_BC_CUDA_H_
