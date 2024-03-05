#include <iostream>
#include <pde_base.hpp>
#include <zisa/memory/array.hpp>
#include <zisa/memory/device_type.hpp>
#include <heat_kernel.hpp>
#include <nc_writer.hpp>

enum BoundaryCondition { Dirichlet, Neumann, Periodic };

int main() {
  // Set up the finite difference kernel for the heat equation
  // heat_kernel_cpu -> zisa::copy
  // zisa::array<float, 2> heat_kernel_cpu(zisa::shape_t<2>(3, 3), zisa::device_type::cpu);
  // float scaling = 0.1; // k / dt^2
  // heat_kernel_cpu(0, 0) = 0;
  // heat_kernel_cpu(0, 1) = 1 * scaling;
  // heat_kernel_cpu(0, 2) = 0;
  // heat_kernel_cpu(1, 0) = 1 * scaling;
  // heat_kernel_cpu(1, 1) = 1 - 4 * scaling;
  // heat_kernel_cpu(1, 2) = 1 * scaling;
  // heat_kernel_cpu(2, 0) = 0;
  // heat_kernel_cpu(2, 1) = 1 * scaling;
  // heat_kernel_cpu(2, 2) = 0;

  BoundaryCondition bc = BoundaryCondition::Dirichlet;

  #if CUDA_AVAILABLE
  // zisa::array<float, 2> heat_kernel_gpu(zisa::shape_t<2>(3, 3), zisa::device_type::cuda);
  // zisa::copy(heat_kernel_gpu, heat_kernel_cpu);
  // Construct a PDE based on the given kernel
  const HeatKernel<float> heat_kernel_gpu(1., 0.1, ziza::device_type::cuda);
  std::cout << "case_gpu" << std::endl;
  PDEBase pde(128, 128, heat_kernel_gpu, bc);
  #else
  // Construct a PDE based on the given kernel
  const HeatKernel<float> heat_kernel_cpu(1., 0.1, zisa::device_type::cpu);
  std::cout << "case_cpu" << std::endl;
  PDEBase pde(128, 128, heat_kernel_cpu, bc);
  #endif

  float data[128];
  for (int i = 0; i < 128; i++) {
    data[i] = i;
  }
  if (store_array_to_nc_file("data.nc", 128, data) != 0) {
    std::cout << "error in storing float array" << std::endl;
  }

  float data_from_file[128];
  if (get_array_from_nc_file("data.nc", 128, data_from_file) != 0) {
    std::cout << "error in reading float array" << std::endl;
  }

  for (int i = 0; i < 128; i++) {
    std::cout << data_from_file[i] << " ";
  }
  std::cout << std::endl;

  // TODO: apply initial conditions
  // Do some things with it...
  pde.apply();
  // Do some more things...
  // TODO
  return 0;
}
