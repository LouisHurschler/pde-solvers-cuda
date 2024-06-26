#ifndef CONVOLVE_SIGMA_ADD_F_HPP_
#define CONVOLVE_SIGMA_ADD_F_HPP_

#include <zisa/memory/array.hpp>
#include <zisa/memory/device_type.hpp>
#if CUDA_AVAILABLE
#include <cuda/convolve_sigma_add_f_cuda.hpp>
#endif

#ifndef COUPLED_SLICE
#define COUPLED_SLICE(n_coupled, start_value, memory_location)                 \
  zisa::array_const_view<Scalar, 1> {                                          \
    zisa::shape_t<1>(n_coupled), &start_value, memory_location                 \
  }
#endif

template <int n_coupled, typename Scalar, typename Function>
void convolve_sigma_add_f_cpu(zisa::array_view<Scalar, 2> dst,
                              zisa::array_const_view<Scalar, 2> src,
                              zisa::array_const_view<Scalar, 2> sigma,
                              Scalar del_x_2 /* 1/(dx^2)*/, Function f) {
  const unsigned Nx = src.shape(0);
  const unsigned Ny = src.shape(1);
  for (int x = 1; x < Nx - 1; x++) {
    for (int y = 1; y < Ny - 1; y += n_coupled) {
      // does not work for generic function, calculates all f_1, f_2, ... , fn
      // in one run
      zisa::array<Scalar, 1> result_function =
          f(COUPLED_SLICE(n_coupled, src(x, y), zisa::device_type::cpu));
      for (int i = 0; i < n_coupled; i++) {
        dst(x, y + i) =
            del_x_2 *
                (sigma(2 * x - 1, (y - 1) / n_coupled) * src(x, y + i - 1) +
                 sigma(2 * x - 1, (y - 1) / n_coupled + 2) * src(x, y + i + 1) +
                 sigma(2 * x - 2, (y - 1) / n_coupled + 1) * src(x - 1, y + i) +
                 sigma(2 * x, (y - 1) / n_coupled + 1) * src(x + 1, y + i) -
                 (sigma(2 * x - 1, (y - 1) / n_coupled) +
                  sigma(2 * x - 1, (y - 1) / n_coupled + 2) +
                  sigma(2 * x - 2, (y - 1) / n_coupled + 1) +
                  sigma(2 * x, (y - 1) / n_coupled + 1)) *
                     src(x, y + i)) +
            result_function(i);
      }
    }
  }
}

// TODO: add coupled function(s)
// Function is a general function taking a Scalar returning a Scalar
template <int n_coupled = 1, typename Scalar, typename Function>
void convolve_sigma_add_f(zisa::array_view<Scalar, 2> dst,
                          zisa::array_const_view<Scalar, 2> src,
                          zisa::array_const_view<Scalar, 2> sigma,
                          Scalar del_x_2 /* 1/(dx^2)*/, Function f) {

  const zisa::device_type memory_dst = dst.memory_location();
  const zisa::device_type memory_src = src.memory_location();
  const zisa::device_type memory_sigma = sigma.memory_location();

  if (!(memory_dst == memory_src && memory_src == memory_sigma)) {
    std::cerr << "Convolve sigma add f: Inputs must be located on the same "
                 "hardware\n";
    exit(1);
  }
  if (dst.shape() != src.shape()) {
    std::cerr << "Convolve sigma add f: Input and output array must have the "
                 "same shape\n";
    exit(1);
  }

  if (memory_dst == zisa::device_type::cpu) {
    convolve_sigma_add_f_cpu<n_coupled>(dst, src, sigma, del_x_2, f);
  }
#if CUDA_AVAILABLE
  else if (memory_dst == zisa::device_type::cuda) {
    // convolve_sigma_add_f_cuda<n_coupled>(dst, src, sigma, del_x_2, f);
  }
#endif // CUDA_AVAILABLE
  else {
    std::cerr << "Convolution: Unknown device_type of inputs\n";
    exit(1);
  }
}

#endif // CONVOLVE_SIGMA_ADD_F_HPP_