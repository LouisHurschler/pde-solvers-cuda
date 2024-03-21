#ifndef HELPERS_HPP_
#define HELPERS_HPP_

#include "zisa/memory/device_type.hpp"
#include <zisa/memory/array.hpp>

template <typename Scalar>
void add_arrays_cpu(zisa::array_view<Scalar, 2> dst,
                    zisa::array_const_view<Scalar, 2> src, Scalar scaling) {
  // TODO: Optimize
  for (int i = 0; i < dst.shape(0); i++) {
    for (int j = 0; j < dst.shape(1); j++) {
      dst(i, j) += scaling * src(i, j);
    }
  }
}

// PRE: dimensions of src and dst match, both are stored on same device type
// POST: dst(i, j) = dst(i, j) + scaling * src(i, j)
template <typename Scalar>
void add_arrays(zisa::array_view<Scalar, 2> dst,
                zisa::array_const_view<Scalar, 2> src, Scalar scaling) {
  const zisa::device_type memory_dst = dst.memory_location();
  if (memory_dst != src.memory_location()) {
    std::cerr << "Error in add_arrays: dst and src have to have the same "
                 "memory location"
              << std::endl;
    exit(1);
  }
  if (dst.shape() != src.shape()) {
    std::cerr << "Error in add_arrays: dst and src have to have the same size"
              << std::endl;
    exit(1);
  }
  if (memory_dst == zisa::device_type::cpu) {
    add_arrays_cpu(dst, src, scaling);
  }
#if CUDA_AVAILABLE
  else if (memory_dst == zisa::device_type::cuda) {
    // TODO
    std::cout << "add arrays on cuda have to be implemented" << std::endl;
    // add_arrays_cuda(dst, src);
  }
#endif // CUDA_AVAILABLE
  else {
    std::cerr << "Add arrays: Unknown device_type of inputs\n";
    exit(1);
  }
}

template <typename Scalar>
inline void print_matrix(const zisa::array_const_view<Scalar, 2> &array) {
#if CUDA_AVAILABLE
  zisa::array<float, 2> cpu_data(array.shape());
  zisa::copy(cpu_data, array);
  for (int i = 0; i < array.shape(0); i++) {
    for (int j = 0; j < array.shape(1); j++) {
      std::cout << cpu_data(i, j) << "\t";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
#else
  for (int i = 0; i < array.shape(0); i++) {
    for (int j = 0; j < array.shape(1); j++) {
      std::cout << array(i, j) << "\t";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
#endif // CUDA_AVAILABLE
}

// make shure that the dimensions of the data to read and the preallocated
// data array match
template <typename Scalar>
inline void read_data(zisa::HierarchicalReader &reader,
                      zisa::array<Scalar, 2> &data, const std::string &tag) {
#if CUDA_AVAILABLE
  zisa::array<float, 2> cpu_data(data.shape());
  zisa::load_impl<Scalar, 2>(reader, cpu_data, tag,
                             zisa::default_dispatch_tag{});
  zisa::copy(data, cpu_data);
#else
  zisa::load_impl(reader, data, tag, zisa::bool_dispatch_tag{});
#endif // CUDA_AVAILABLE
}

#endif // HELPERS_HPP_
