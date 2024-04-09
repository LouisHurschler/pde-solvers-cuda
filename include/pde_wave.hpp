#ifndef PDE_WAVE_HPP_
#define PDE_WAVE_HPP_

// TODO: add initial derivative data, correct apply function

#include "periodic_bc.hpp"
#include "zisa/io/hdf5_writer.hpp"
#include <pde_base.hpp>

template <typename Scalar, typename Function>
class PDEWave : public virtual PDEBase<Scalar> {
public:
  // TODO: add derivative
  PDEWave(unsigned Nx, unsigned Ny, const zisa::device_type memory_location,
          BoundaryCondition bc, Function f, Scalar dx, Scalar dy)
      : PDEBase<Scalar>(Nx, Ny, memory_location, bc), func_(f),
        deriv_data_(zisa::shape_t<2>(Nx + 2, Ny + 2)) {}

  void apply(Scalar dt) override {
    if (!this->ready_) {
      std::cerr << "Wave solver is not ready yet! Read data first" << std::endl;
      return;
    }

    zisa::array<Scalar, 2> tmp(this->data_.shape(), this->data_.device());
    const Scalar del_x_2 = 1. / (this->dx_ * this->dy_);
    convolve_sigma_add_f(tmp.view(), this->data_.const_view(),
                         this->sigma_values_.const_view(), del_x_2, func_);
    add_arrays_interior(this->data_.view(), tmp.const_view());
    PDEBase<Scalar>::add_bc();
  }

  void read_values(const std::string &filename,
                   const std::string &tag_data = "initial_data",
                   const std::string &tag_sigma = "sigma",
                   const std::string &tag_initial_derivative = "init_deriv") {
    zisa::HDF5SerialReader reader(filename);

    read_data(reader, this->data_, tag_data);
    read_data(reader, this->sigma_values_, tag_sigma);
    read_data(reader, this->deriv_data_, tag_initial_derivative);

    if (this->bc_ == BoundaryCondition::Neumann) {
      zisa::copy(this->bc_neumann_values_, this->deriv_data_);
    } else if (this->bc_ == BoundaryCondition::Dirichlet) {
      // do noching as long as data on boundary does not change
    } else if (this->bc_ == BoundaryCondition::Periodic) {
      periodic_bc(this->data_.view());
    }
    this->ready_ = true;
  }

protected:
  Function func_;
  // add
  zisa::array<Scalar, 2> deriv_data_;
};

#endif // PDE_WAVE_HPP_
