/*
  This file is part of MADNESS.

  Copyright (C) 2007,2010 Oak Ridge National Laboratory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

  For more information please contact:

  Robert J. Harrison
  Oak Ridge National Laboratory
  One Bethel Valley Road
  P.O. Box 2008, MS-6367

  email: harrisonrj@ornl.gov
  tel:   865-241-3937
  fax:   865-572-0680
*/

/// \file SCFOperators.h
/// \brief Operators for the molecular HF and DFT code
/// \defgroup chem The molecular density functional and Hartree-Fock code


#ifndef MADNESS_CHEM_SCFOPERATORS_H_
#define MADNESS_CHEM_SCFOPERATORS_H_

#include <madness.h>
#include <chem/exchangeoperator.h>

using namespace madness;

namespace madness {

// forward declaration
class SCF;
class Nemo;
class NuclearCorrelationFactor;
class XCfunctional;

typedef std::vector<real_function_3d> vecfuncT;

template<typename T, std::size_t NDIM>
class Kinetic {
    typedef DistributedMatrix<T> distmatT;
    typedef Function<T,NDIM> functionT;
    typedef std::vector<functionT> vecfuncT;
    typedef Tensor<T> tensorT;

public:
    Kinetic(World& world) : world(world) {
        gradop = gradient_operator<T,NDIM>(world);
    }

    functionT operator()(const functionT& ket) const {
        MADNESS_EXCEPTION("do not apply the kinetic energy operator on a function!",1);
        return ket;
    }

    vecfuncT operator()(const vecfuncT& vket) const {
        MADNESS_EXCEPTION("do not apply the kinetic energy operator on a function!",1);
        return vket;
    }

    T operator()(const functionT& bra, const functionT& ket) const {
        vecfuncT vbra(1,bra), vket(1,ket);
        Tensor<T> tmat=this->operator()(vbra,vket);
        return tmat(0l,0l);
    }

    tensorT operator()(const vecfuncT& vbra, const vecfuncT& vket) const {
        distmatT dkinetic;
        if (&vbra==&vket) {
            dkinetic = kinetic_energy_matrix(world,vbra);
        } else {
            dkinetic = kinetic_energy_matrix(world,vbra,vket);
        }
        tensorT kinetic(vbra.size(),vket.size());
        dkinetic.copy_to_replicated(kinetic);
        return kinetic;
    }

private:
    World& world;
    std::vector< std::shared_ptr<Derivative<T,NDIM> > > gradop;

    distmatT kinetic_energy_matrix(World & world, const vecfuncT & v) const;
    distmatT kinetic_energy_matrix(World & world, const vecfuncT & vbra,
            const vecfuncT & vket) const;

};


template<typename T, std::size_t NDIM>
class DerivativeOperator {
    typedef Function<T,NDIM> functionT;
    typedef std::vector<functionT> vecfuncT;
    typedef Tensor<T> tensorT;

public:

    DerivativeOperator(World& world, const int axis1) : world(world), axis(axis1) {
        gradop = free_space_derivative<T,NDIM>(world, axis);
    }

    functionT operator()(const functionT& ket) const {
        vecfuncT vket(1,ket);
        return this->operator()(vket)[0];
    }

    vecfuncT operator()(const vecfuncT& vket) const {
        vecfuncT dvket=apply(world, gradop, vket, false);
        world.gop.fence();
        return dvket;
    }

    T operator()(const functionT& bra, const functionT& ket) const {
        vecfuncT vbra(1,bra), vket(1,ket);
        Tensor<T> tmat=this->operator()(vbra,vket);
        return tmat(0l,0l);
    }

    tensorT operator()(const vecfuncT& vbra, const vecfuncT& vket) const {
        const auto bra_equiv_ket = &vbra == &vket;
        vecfuncT dvket=this->operator()(vket);
        return matrix_inner(world,vbra,dvket, bra_equiv_ket);
    }

private:
    World& world;
    int axis;
    Derivative<T,NDIM> gradop;

};


/// the Laplacian operator: \sum_i \nabla^2_i

/// note that the application of the Laplacian operator is in general
/// unstable and very sensitive to noise and cusps in the argument.
///
/// !!! BE SURE YOU KNOW WHAT YOU ARE DOING !!!
///
/// For computing matrix elements, which is reasonably stable, we refer
template<typename T, std::size_t NDIM>
class Laplacian {
    typedef Function<T,NDIM> functionT;
    typedef std::vector<functionT> vecfuncT;
    typedef Tensor<T> tensorT;

public:

    Laplacian(World& world, const double e=0.0) : world(world), eps(e) {
        gradop = gradient_operator<T,NDIM>(world);
    }

    functionT operator()(const functionT& ket) const {
        vecfuncT vket(1,ket);
        return this->operator()(vket)[0];
    }

    vecfuncT operator()(const vecfuncT& vket) const;

    T operator()(const functionT& bra, const functionT ket) const {
        vecfuncT vbra(1,bra), vket(1,ket);
        Tensor<T> tmat=this->operator()(vbra,vket);
        return tmat(0l,0l);
    }

    tensorT operator()(const vecfuncT& vbra, const vecfuncT& vket) const {
        Kinetic<T,NDIM> t(world);
        return -2.0*t(vbra,vket);
    }

private:
    World& world;
    std::vector< std::shared_ptr< Derivative<T,NDIM> > > gradop;
    double eps;
};



class Coulomb {
public:

    /// default empty ctor
    Coulomb(World& world) : world(world) {};

    /// ctor with an SCF calculation providing the MOs and density
    Coulomb(World& world, const SCF* calc);

    /// ctor with a Nemo calculation providing the MOs and density
    Coulomb(World& world, const Nemo* nemo);

    auto set_poisson(World& world, const double lo, const double econv=FunctionDefaults<3>::get_thresh()) {
        return std::shared_ptr<real_convolution_3d>(CoulombOperatorPtr(world, lo, econv));
    };

    void set_metric(const real_function_3d& metric) {
    	R_square=copy(metric);
    }

    template<typename T, std::size_t NDIM>
    Function<T,NDIM> operator()(const Function<T,NDIM>& ket) const {
        return (vcoul*ket).truncate();
    }

    template<typename T, std::size_t NDIM>
    std::vector<Function<T,NDIM> > operator()(const std::vector<Function<T,NDIM> >& vket) const {
        std::vector<Function<T,NDIM> > tmp=mul(world,vcoul,vket);
        truncate(world,tmp);
        return tmp;
    }

    template<typename T, std::size_t NDIM>
    T operator()(const Function<T,NDIM>& bra, const Function<T,NDIM>& ket) const {
        return inner(bra,vcoul*ket);
    }

    template<typename T, std::size_t NDIM>
    Tensor<T> operator()(const std::vector<Function<T,NDIM> >& vbra,
    		const std::vector<Function<T,NDIM> >& vket) const {
        const auto bra_equiv_ket = &vbra == &vket;
        std::vector<Function<T,NDIM> > vJket;
        for (std::size_t i=0; i<vket.size(); ++i) {
            vJket.push_back(this->operator()(vket[i]));
        }
        return matrix_inner(world,vbra,vJket,bra_equiv_ket);
    }

    /// getter for the Coulomb potential
    const real_function_3d& potential() const {return vcoul;}

    /// setter for the Coulomb potential
    real_function_3d& potential() {return vcoul;}

    real_function_3d compute_density(const SCF* calc) const;

    /// given a density compute the Coulomb potential

    /// this function uses a newly constructed Poisson operator. Note that
    /// the accuracy parameters must be consistent with the exchange operator.
    template<typename T, std::size_t NDIM>
    Function<T,NDIM> compute_potential(const Function<T,NDIM>& density) const {
    	return (*poisson)(density).truncate();
    }

    /// given a set of MOs in an SCF calculation, compute the Coulomb potential

    /// this function uses the Poisson operator of the SCF calculation
    real_function_3d compute_potential(const SCF* calc) const;

    /// given a set of MOs in an SCF calculation, compute the Coulomb potential

    /// this function uses the Poisson operator of the SCF calculation
    real_function_3d compute_potential(const Nemo* nemo) const;

private:
    World& world;
    std::shared_ptr<real_convolution_3d> poisson;
    double lo=1.e-4;
    real_function_3d vcoul; ///< the coulomb potential
    real_function_3d R_square;    ///< square of the nuclear correlation factor, if any
};


class Nuclear {
public:

    Nuclear(World& world, const SCF* calc);

    Nuclear(World& world, const Nemo* nemo);

    Nuclear(World& world, std::shared_ptr<NuclearCorrelationFactor> ncf)
        : world(world), ncf(ncf) {}


    template<typename T, std::size_t NDIM>
    Function<T,NDIM> operator()(const Function<T,NDIM>& ket) const {
        std::vector<Function<T,NDIM> > vket(1,ket);
        return this->operator()(vket)[0];
    }

    template<typename T, std::size_t NDIM>
    std::vector<Function<T,NDIM> > operator()(const std::vector<Function<T,NDIM> >& vket) const;

    template<typename T, std::size_t NDIM>
    T operator()(const Function<T,NDIM>& bra, const Function<T,NDIM>& ket) const {
        return inner(bra,this->operator()(ket));
    }

    template<typename T, std::size_t NDIM>
    Tensor<T> operator()(const  std::vector<Function<T,NDIM> >& vbra,
    		const  std::vector<Function<T,NDIM> >& vket) const {
        const auto bra_equiv_ket = &vbra == &vket;
        std::vector<Function<T,NDIM> > vVket=this->operator()(vket);
        return matrix_inner(world,vbra,vVket,bra_equiv_ket);
    }

private:
    World& world;
    std::shared_ptr<NuclearCorrelationFactor> ncf;

};


/// the z component of the angular momentum

/// takes real and complex functions as input, will return complex functions
class Lz {
private:
    World& world;
public:

	bool use_bsplines=true;

	Lz(World& world, bool use_bspline_derivative=true) : world(world), use_bsplines(use_bspline_derivative) {};

	template<typename T, std::size_t NDIM>
    complex_function_3d operator()(const Function<T,NDIM> ket) const {
		std::vector<Function<T,NDIM> > vket(1,ket);
        return this->operator()(vket)[0];
    }

	template<typename T, std::size_t NDIM>
    std::vector<complex_function_3d> operator()(const std::vector<Function<T,NDIM> >& vket) const {

		// the operator in cartesian components as
		// L_z =  - i (x del_y - y del_x)

		if (vket.size()==0) return std::vector<complex_function_3d>(0);

	    real_function_3d x=real_factory_3d(world).functor([] (const coord_3d& r) {return r[0];});
	    real_function_3d y=real_factory_3d(world).functor([] (const coord_3d& r) {return r[1];});

	    Derivative<T,NDIM> Dx = free_space_derivative<T,NDIM>(world, 0);
		Derivative<T,NDIM> Dy = free_space_derivative<T,NDIM>(world, 1);
		if (use_bsplines) {
			Dx.set_bspline1();
			Dy.set_bspline1();
		}

		reconstruct(world,vket,true);
	    std::vector<Function<T,NDIM> > delx=apply(world,Dx,vket,false);
	    std::vector<Function<T,NDIM> > dely=apply(world,Dy,vket,true);

	    std::vector<Function<T,NDIM> > result1=x*dely - y*delx;
	    std::vector<complex_function_3d> cresult1=convert<T,double_complex,NDIM>(world,result1);
	    std::vector<complex_function_3d> result=double_complex(0.0,-1.0)*cresult1;
		return result;
	}

	template<typename T, std::size_t NDIM>
    double_complex operator()(const Function<T,NDIM>& bra, const Function<T,NDIM>& ket) const {
        return inner(bra,this->operator()(ket));
    }

	template<typename T, std::size_t NDIM>
    Tensor<double_complex> operator()(const std::vector<Function<T,NDIM> >& vbra,
    		const std::vector<Function<T,NDIM> >& vket) const {
        const auto bra_equiv_ket = &vbra == &vket;
        std::vector<complex_function_3d> vVket=this->operator()(vket);
        return matrix_inner(world,vbra,vVket,bra_equiv_ket);
    }

};



/// derivative of the (regularized) nuclear potential wrt nuclear displacements
class DNuclear {
public:

    DNuclear(World& world, const SCF* calc, const int iatom, const int iaxis);

    DNuclear(World& world, const Nemo* nemo, const int iatom, const int iaxis);

    DNuclear(World& world, std::shared_ptr<NuclearCorrelationFactor> ncf,
            const int iatom, const int iaxis)
        : world(world), ncf(ncf), iatom(iatom), iaxis(iaxis) {}


    real_function_3d operator()(const real_function_3d& ket) const {
        vecfuncT vket(1,ket);
        return this->operator()(vket)[0];
    }

    vecfuncT operator()(const vecfuncT& vket) const;

    double operator()(const real_function_3d& bra, const real_function_3d& ket) const {
        return inner(bra,this->operator()(ket));
    }

    Tensor<double> operator()(const vecfuncT& vbra, const vecfuncT& vket) const {
        const auto bra_equiv_ket = &vbra == &vket;
        vecfuncT vVket=this->operator()(vket);
        return matrix_inner(world,vbra,vVket,bra_equiv_ket);
    }

private:
    World& world;
    std::shared_ptr<NuclearCorrelationFactor> ncf;
    int iatom;  ///< index of the atom which is displaced
    int iaxis;  ///< x,y,z component of the atom

};


/// operator class for the handling of DFT exchange-correlation functionals
class XCOperator {
public:

    /// default ctor without information about the XC functional
    XCOperator(World& world) : world(world), nbeta(0), ispin(0),
        extra_truncation(FunctionDefaults<3>::get_thresh()*0.01) {}

    /// custom ctor with information about the XC functional
    XCOperator(World& world, std::string xc_data, const bool spin_polarized,
            const real_function_3d& arho, const real_function_3d& brho,
            std::string deriv="abgv");

    /// ctor with an SCF calculation, will initialize the necessary intermediates
    XCOperator(World& world, const SCF* scf, int ispin=0, std::string deriv="abgv");

    /// ctor with a Nemo calculation, will initialize the necessary intermediates
    XCOperator(World& world, const Nemo* nemo, int ispin=0);

    /// ctor with an SCF calculation, will initialize the necessary intermediates
    XCOperator(World& world, const SCF* scf, const real_function_3d& arho,
            const real_function_3d& brho, int ispin=0, std::string deriv="abgv");

    /// ctor with an Nemo calculation, will initialize the necessary intermediates
    XCOperator(World& world, const Nemo* scf, const real_function_3d& arho,
            const real_function_3d& brho, int ispin=0);

    XCOperator& set_extra_truncation(const double& fac) {
        extra_truncation=fac;
        if (world.rank()==0)
            print("set extra truncation in XCOperator to", extra_truncation);
        return *this;
    }

    /// set the spin state this operator is acting on
    void set_ispin(const int i) const {ispin=i;}

    /// apply the xc potential on a set of orbitals
    template<typename T>
    std::vector<Function<T,3> > operator()(const std::vector<Function<T,3> >& vket) const;

    /// apply the xc potential on an orbitals
    template<typename T>
    Function<T,3> operator()(const Function<T,3>& ket) const {
    	std::vector<Function<T,3> > vket(1,ket);
    	std::vector<Function<T,3> > vKket=this->operator()(vket);
        return vKket[0];
    }

    /// compute the xc energy using the precomputed intermediates vf and delrho
    double compute_xc_energy() const;

    /// return the local xc potential
    real_function_3d make_xc_potential() const;

    /// construct the xc kernel and apply it directly on the (response) density

    /// the xc kernel is the second derivative of the xc functions wrt the density
    /// @param[in]  density the (response) density on which the kernel is applied
    /// @return     kernel * density
    real_function_3d apply_xc_kernel(const real_function_3d& density,
            const vecfuncT grad_dens_pt=vecfuncT()) const;

private:

    /// the world
    World& world;

    /// which derivative operator to use
    std::string dft_deriv;

public:
    /// interface to the actual XC functionals
    std::shared_ptr<XCfunctional> xc;

private:
    /// number of beta orbitals
    int nbeta;

    /// the XC functionals depend on the spin of the orbitals they act on
    mutable int ispin;

    /// additional truncation for the densities in the XC kernel

    /// the densities in the DFT kernal are processed as their inverses,
    /// so noise in the small density regions might amplify and lead to inaccurate
    /// results. Extra truncation will tighten the truncation threshold by a
    /// specified factor, default is 0.01.
    double extra_truncation;

    /// the nuclear correlation factor, if it exists, for computing derivatives for GGA
    std::shared_ptr<NuclearCorrelationFactor> ncf;

    /// functions that are need for the computation of the XC operator

    /// the ordering of the intermediates is fixed, but the code can handle
    /// non-initialized functions, so if e.g. no GGA is requested, all the
    /// corresponding vector components may be left empty.
    /// For the ordering of the intermediates see xcfunctional::xc_arg
    mutable vecfuncT xc_args;

    /// compute the intermediates for the XC functionals

    /// @param[in]  arho    density of the alpha orbitals
    /// @param[in]  brho    density of the beta orbitals (necessary only if spin-polarized)
    /// @return xc_args vector of intermediates as described above
    vecfuncT prep_xc_args(const real_function_3d& arho, const real_function_3d& brho) const;

    /// compute the intermediates for the XC functionals

    /// @param[in]  dens_pt     perturbed densities from CPHF or TDDFT equations
    /// @param[in,out] xc_args   vector of intermediates as described above
    /// @param[out] ddens_pt    xyz-derivatives of dens_pt
    void prep_xc_args_response(const real_function_3d& dens_pt,
            vecfuncT& xc_args, vecfuncT& ddens_pt) const;

    /// check if the intermediates are initialized
    bool is_initialized() const {
        return (xc_args.size()>0);
    }

    /// simple structure to take the pointwise logarithm of a function, shifted by +14
    struct logme{
        typedef double resultT;
        struct logme1 {
            double operator()(const double& val) {return log(std::max(1.e-14,val))+14.0;}
        };
        Tensor<double> operator()(const Key<3>& key, const Tensor<double>& val) const {
            Tensor<double> result=copy(val);
            logme1 op;
            return result.unaryop(op);
        }

        template <typename Archive>
        void serialize(Archive& ar) {}
    };

    /// simple structure to take the pointwise exponential of a function, shifted by +14
    struct expme{
        typedef double resultT;
        struct expme1 {
            double operator()(const double& val) {return exp(val-14.0);}
        };
        Tensor<double> operator()(const Key<3>& key, const Tensor<double>& val) const {
            Tensor<double> result=copy(val);
            expme1 op;
            return result.unaryop(op);
        }

        template <typename Archive>
        void serialize(Archive& ar) {}

    };
};

/// Computes matrix representation of the Fock operator
class Fock {
public:
    /// \param[in] scale_K scaling factor for the Hartree-Fock exchange operator (the default is 1, i.e. include
    ///            the full exchange; setting scale_K to 0 excludes the exchange operator, and its computation is skipped)
    Fock(World& world, const SCF* calc,
         double scale_K = 1);
    Fock(World& world, const Nemo* nemo,
         double scale_K = 1);

    real_function_3d operator()(const real_function_3d& ket) const {
      MADNESS_EXCEPTION("Fock(ket) not yet implemented",1);
      real_function_3d result;
      return result;
    }
    double operator()(const real_function_3d& bra, const real_function_3d& ket) const {
        const auto compute_K = (scale_K != 0.0);
        double J_00 = J(bra,ket);
        double K_00 = compute_K ? (scale_K == 1. ? K(bra,ket) : scale_K * K(bra,ket)) : 0;
        double T_00 = T(bra,ket);
        double V_00 = V(bra,ket);
        return T_00 + J_00 - K_00 + V_00;
    }

    Tensor<double> operator()(const vecfuncT& vbra, const vecfuncT& vket) const {
        const auto compute_K = (scale_K != 0.0);
        double wtime=-wall_time(); double ctime=-cpu_time();
        Tensor<double> kmat= compute_K ? (scale_K == 1. ? K(vbra,vket) : scale_K * K(vbra,vket)) : Tensor<double>(vbra.size(), vket.size());
        Tensor<double> jmat=J(vbra,vket);
        Tensor<double> tmat=T(vbra,vket);
        Tensor<double> vmat=V(vbra,vket);
        Tensor<double> fock=tmat+jmat-kmat+vmat;
        wtime+=wall_time(); ctime+=cpu_time();
        if (world.rank()==0) printf("timer: %20.20s %8.2fs %8.2fs\n", "fock matrix", wtime, ctime);
        return fock;
    }


private:
    World& world;
    Coulomb J;
    Exchange<double,3> K;
    Kinetic<double,3> T;
    Nuclear V;
    const double scale_K;
};

}
#endif /* MADNESS_CHEM_SCFOPERATORS_H_ */
