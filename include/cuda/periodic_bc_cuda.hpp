#ifndef PERIODIC_BC_CUDA_H_
#define PERIODIC_BC_CUDA_H_

#include <zisa/memory/array.hpp>

template <typename Scalar>
void periodic_bc_cuda(zisa::array_view<Scalar, 2> data);

#define PDE_SOLVERS_CUDA_INSTANCIATE_PERIODIC_BC_CUDA(TYPE)                    \
  extern template void periodic_bc_cuda<TYPE>(zisa::array_view<TYPE, 2>);

PDE_SOLVERS_CUDA_INSTANCIATE_PERIODIC_BC_CUDA(float)
PDE_SOLVERS_CUDA_INSTANCIATE_PERIODIC_BC_CUDA(double)

#undef PDE_SOLVERS_CUDA_INSTANCIATE_PERIODIC_BC_CUDA
#endif // PERIODIC_BC_CUDA_H_
