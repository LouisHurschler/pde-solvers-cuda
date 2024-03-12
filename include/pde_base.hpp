#ifndef PDE_BASE_HPP_
#define PDE_BASE_HPP_

#include "zisa/io/file_manipulation.hpp"
#include "zisa/memory/memory_location.hpp"
#include "zisa/memory/shape.hpp"
#include <dirichlet_bc.hpp>
#include <neumann_bc.hpp>
#include <periodic_bc.hpp>
#include <convolution.hpp>
#include <zisa/io/hdf5_serial_writer.hpp>
#include <zisa/memory/array.hpp>
#include <zisa/memory/device_type.hpp>

template <typename Scalar, typename BoundaryCondition> class PDEBase {
public:
  using scalar_t = Scalar;

  PDEBase(unsigned Nx, unsigned Ny,
          const zisa::array_const_view<Scalar, 2> &kernel, BoundaryCondition bc)
      : data_(zisa::shape_t<2>(Nx + 2 * (kernel.shape(0) / 2),
                               Ny + 2 * (kernel.shape(1) / 2)),
              kernel.memory_location()),
        kernel_(kernel), bc_(bc) {}

  // make shure that the file exists with the right group name and tag,
  // otherwise this will crash Additionally, the data has to be stored as a
  // 2-dimensional array with the right amount of entries
  void read_initial_data(const std::string &filename,
                         const std::string &group_name,
                         const std::string &tag) {
    unsigned x_disp = num_ghost_cells_x();
    unsigned y_disp = num_ghost_cells_y();

    unsigned Nx = data_.shape(0) - 2 * num_ghost_cells_x();
    unsigned Ny = data_.shape(1) - 2 * num_ghost_cells_y();

    // read data from file
    Scalar return_data[Nx][Ny];
    zisa::HDF5SerialReader serial_reader(filename);
    serial_reader.open_group(group_name);
    serial_reader.read_array(return_data, zisa::erase_data_type<Scalar>(), tag);

    // copy return_data to data_
    // TODO: Optimize
    if (kernel_.memory_location() == zisa::device_type::cpu) {
      for (int i = 0; i < Nx; i++) {
        for (int j = 0; j < Ny; j++) {
          data_(x_disp + i, y_disp + j) = return_data[i][j];
        }
      }
    } else if (kernel_.memory_location() == zisa::device_type::cuda) {
      zisa::array<Scalar, 2> tmp(
          zisa::shape_t<2>(data_.shape(0), data_.shape(1)),
          zisa::device_type::cpu);
      for (int i = 0; i < Nx; i++) {
        for (int j = 0; j < Ny; j++) {
          tmp(x_disp + i, y_disp + j) = return_data[i][j];
        }
      }
      zisa::copy(data_, tmp);
    } else {
      std::cout << "only data on cpu and cuda supported" << std::endl;
    }
    add_bc();
  }

  void apply() {
    zisa::array<scalar_t, 2> tmp(data_.shape(), data_.device());
    convolve(tmp.view(), data_.const_view(), this->kernel_);
    zisa::copy(data_, tmp);
    add_bc();
  }

  unsigned num_ghost_cells(unsigned dir) { return kernel_.shape(dir) / 2; }
  unsigned num_ghost_cells_x() { return num_ghost_cells(0); }
  unsigned num_ghost_cells_y() { return num_ghost_cells(1); }

  // for testing/debugging
  void print() {
    int x_size = data_.shape(0);
    int y_size = data_.shape(1);
    std::cout << "data has size x: " << x_size << ", y: " << y_size
              << std::endl;
    std::cout << "border sizes are x: " << num_ghost_cells_x()
              << ", y: " << num_ghost_cells_y() << std::endl;
    // weird segmentation fault if using cuda
    // how is it possible to print an array on gpus?
    #if CUDA_AVAILABLE
      zisa::array<float, 2> cpu_data(zisa::shape_t<2>(x_size, y_size));
      zisa::copy(cpu_data, data_);
    for (int i = 0; i < x_size; i++) {
      for (int j = 0; j < y_size; j++) {
        std::cout << cpu_data(i, j) << "\t";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
      return;
    #endif
    for (int i = 0; i < x_size; i++) {
      for (int j = 0; j < y_size; j++) {
        std::cout << data_(i, j) << "\t";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

protected:
  void add_bc() {
    if (bc_ == BoundaryCondition::Dirichlet) {
      // TODO: only do this when initialize data, or if boundary values change
      dirichlet_bc<Scalar>(data_, num_ghost_cells_x(), num_ghost_cells_y(), 0.,
                   kernel_.memory_location());
    } else if (bc_ == BoundaryCondition::Neumann) {
      neumann_bc(data_, num_ghost_cells_x(), num_ghost_cells_y(), kernel_.memory_location());
      // TODO: add boundary conditions
    } else if (bc_ == BoundaryCondition::Periodic) {
      periodic_bc(data_, num_ghost_cells_x(), num_ghost_cells_y(), kernel_.memory_location());
      // TODO: add boundary conditions
    } else {
      std::cout << "boundary condition not implemented yet!" << std::endl;
    }
  }


  zisa::array<Scalar, 2> data_;
  const zisa::array_const_view<Scalar, 2> kernel_;
  const BoundaryCondition bc_;
};

#endif // PDE_BASE_HPP_
