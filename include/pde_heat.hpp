#ifndef PDE_HEAT_HPP_
#define PDE_HEAT_HPP_

#include "periodic_bc.hpp"
#include "zisa/memory/device_type.hpp"
#include <pde_base.hpp>

template <typename Scalar, typename Function, int n_coupled = 1>
class PDEHeat : public virtual PDEBase<Scalar> {
public:
  PDEHeat(unsigned Nx, unsigned Ny, const zisa::device_type memory_location,
          BoundaryCondition bc, Function f, Scalar dx, Scalar dy)
      : PDEBase<Scalar, n_coupled>(Nx, Ny, memory_location, bc, dx, dy),
        func_(f) {}

  void read_values(const std::string &filename,
                   const std::string &tag_data = "initial_data",
                   const std::string &tag_sigma = "sigma",
                   const std::string &tag_bc = "bc") {

    zisa::HDF5SerialReader reader(filename);
    read_data(reader, this->data_, tag_data);
    read_data(reader, this->sigma_values_, tag_sigma);

    if (this->bc_ == BoundaryCondition::Neumann) {
      read_data(reader, this->bc_neumann_values_, tag_bc);
    } else if (this->bc_ == BoundaryCondition::Dirichlet) {
      // do noching as long as data on boundary does not change
    } else if (this->bc_ == BoundaryCondition::Periodic) {
      periodic_bc<Scalar, n_coupled>(this->data_.view());
    }
    this->ready_ = true;
  }

  void read_values(zisa::array_const_view<Scalar, 2> data,
                   zisa::array_const_view<Scalar, 2> sigma,
                   zisa::array_const_view<Scalar, 2> bc) {
    zisa::copy(this->data_, data);
    zisa::copy(this->sigma_values_, sigma);
    if (this->bc_ == BoundaryCondition::Neumann) {
      zisa::copy(this->bc_neumann_values_, bc);
    } else if (this->bc_ == BoundaryCondition::Dirichlet) {
      // do noching as long as data on boundary does not change
    } else if (this->bc_ == BoundaryCondition::Periodic) {
      periodic_bc<Scalar, n_coupled>(this->data_.view());
    }
    this->ready_ = true;
  }

  void apply(Scalar dt) override {
    if (!this->ready_) {
      std::cerr << "Heat solver is not ready yet! Read data first" << std::endl;
      return;
    }

    zisa::array<Scalar, 2> tmp(this->data_.shape(), this->data_.device());
    const Scalar del_x_2 = 1. / (this->dx_ * this->dy_);
    convolve_sigma_add_f<n_coupled>(tmp.view(), this->data_.const_view(),
                                    this->sigma_values_.const_view(), del_x_2,
                                    func_);

    // euler update of data
    add_arrays_interior<n_coupled>(this->data_.view(), tmp.const_view(), dt);
    PDEBase<Scalar>::add_bc();
  }

protected:
  Function func_;
};

#endif // PDE_HEAT_HPP_
