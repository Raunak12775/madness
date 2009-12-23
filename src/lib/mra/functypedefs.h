#ifndef MADNESS_MRA_FUNCTYPEDEFS_H__INCLUDED
#define MADNESS_MRA_FUNCTYPEDEFS_H__INCLUDED

/// \file mra/functypedefs.h
/// \brief Provides typedefs to hide use of templates and to increase interoperability

namespace madness {
    typedef Tensor<double> real_tensor;
    typedef Tensor<double_complex> complex_tensor;

    typedef Vector<double,1> coord_1d;
    typedef Vector<double,2> coord_2d;
    typedef Vector<double,3> coord_3d;
    typedef Vector<double,4> coord_4d;
    typedef Vector<double,5> coord_5d;
    typedef Vector<double,6> coord_6d;
    
    typedef Function<double,1> real_function_1d;
    typedef Function<double,2> real_function_2d;
    typedef Function<double,3> real_function_3d;
    typedef Function<double,4> real_function_4d;
    typedef Function<double,5> real_function_5d;
    typedef Function<double,6> real_function_6d;
    
    typedef Function<double_complex,1> complex_function_1d;
    typedef Function<double_complex,2> complex_function_2d;
    typedef Function<double_complex,3> complex_function_3d;
    typedef Function<double_complex,4> complex_function_4d;
    typedef Function<double_complex,5> complex_function_5d;
    typedef Function<double_complex,6> complex_function_6d;

    typedef std::vector<real_function_1d> vector_real_function_1d;
    typedef std::vector<real_function_2d> vector_real_function_2d;
    typedef std::vector<real_function_3d> vector_real_function_3d;
    typedef std::vector<real_function_4d> vector_real_function_4d;
    typedef std::vector<real_function_5d> vector_real_function_5d;
    typedef std::vector<real_function_6d> vector_real_function_6d;
    
    typedef FunctionFactory<double,1> real_factory_1d;
    typedef FunctionFactory<double,2> real_factory_2d;
    typedef FunctionFactory<double,3> real_factory_3d;
    typedef FunctionFactory<double,4> real_factory_4d;
    typedef FunctionFactory<double,5> real_factory_5d;
    typedef FunctionFactory<double,6> real_factory_6d;
    
    typedef FunctionFactory<double_complex,1> complex_factory_1d;
    typedef FunctionFactory<double_complex,2> complex_factory_2d;
    typedef FunctionFactory<double_complex,3> complex_factory_3d;
    typedef FunctionFactory<double_complex,4> complex_factory_4d;
    typedef FunctionFactory<double_complex,5> complex_factory_5d;
    typedef FunctionFactory<double_complex,6> complex_factory_6d;
    
    typedef SharedPtr< FunctionFunctorInterface<double,1> > real_functor_1d;
    typedef SharedPtr< FunctionFunctorInterface<double,2> > real_functor_2d;
    typedef SharedPtr< FunctionFunctorInterface<double,3> > real_functor_3d;
    typedef SharedPtr< FunctionFunctorInterface<double,4> > real_functor_4d;
    typedef SharedPtr< FunctionFunctorInterface<double,5> > real_functor_5d;
    typedef SharedPtr< FunctionFunctorInterface<double,6> > real_functor_6d;

    typedef SharedPtr< FunctionFunctorInterface<double_complex,1> > complex_functor_1d;
    typedef SharedPtr< FunctionFunctorInterface<double_complex,2> > complex_functor_2d;
    typedef SharedPtr< FunctionFunctorInterface<double_complex,3> > complex_functor_3d;
    typedef SharedPtr< FunctionFunctorInterface<double_complex,4> > complex_functor_4d;
    typedef SharedPtr< FunctionFunctorInterface<double_complex,5> > complex_functor_5d;
    typedef SharedPtr< FunctionFunctorInterface<double_complex,6> > complex_functor_6d;

    typedef SeparatedConvolution<double,1> real_convolution_1d;
    typedef SeparatedConvolution<double,2> real_convolution_2d;
    typedef SeparatedConvolution<double,3> real_convolution_3d;
    typedef SeparatedConvolution<double,4> real_convolution_4d;
    typedef SeparatedConvolution<double,5> real_convolution_5d;
    typedef SeparatedConvolution<double,6> real_convolution_6d;

    typedef SeparatedConvolution<double_complex,1> complex_convolution_1d;
    typedef SeparatedConvolution<double_complex,2> complex_convolution_2d;
    typedef SeparatedConvolution<double_complex,3> complex_convolution_3d;
    typedef SeparatedConvolution<double_complex,4> complex_convolution_4d;
    typedef SeparatedConvolution<double_complex,5> complex_convolution_5d;
    typedef SeparatedConvolution<double_complex,6> complex_convolution_6d;

    typedef SharedPtr<real_convolution_1d> real_convolution_1d_ptr;
    typedef SharedPtr<real_convolution_2d> real_convolution_2d_ptr;
    typedef SharedPtr<real_convolution_3d> real_convolution_3d_ptr;
    typedef SharedPtr<real_convolution_4d> real_convolution_4d_ptr;
    typedef SharedPtr<real_convolution_5d> real_convolution_5d_ptr;
    typedef SharedPtr<real_convolution_6d> real_convolution_6d_ptr;

    typedef SharedPtr<complex_convolution_1d> complex_convolution_1d_ptr;
    typedef SharedPtr<complex_convolution_2d> complex_convolution_2d_ptr;
    typedef SharedPtr<complex_convolution_3d> complex_convolution_3d_ptr;
    typedef SharedPtr<complex_convolution_4d> complex_convolution_4d_ptr;
    typedef SharedPtr<complex_convolution_5d> complex_convolution_5d_ptr;
    typedef SharedPtr<complex_convolution_6d> complex_convolution_6d_ptr;

    typedef SharedPtr< WorldDCPmapInterface< Key<1> > > pmap_1d;
    typedef SharedPtr< WorldDCPmapInterface< Key<2> > > pmap_2d;
    typedef SharedPtr< WorldDCPmapInterface< Key<3> > > pmap_3d;
    typedef SharedPtr< WorldDCPmapInterface< Key<4> > > pmap_4d;
    typedef SharedPtr< WorldDCPmapInterface< Key<5> > > pmap_5d;
    typedef SharedPtr< WorldDCPmapInterface< Key<6> > > pmap_6d;

    typedef FunctionImpl<double,1> real_funcimpl_1d;
    typedef FunctionImpl<double,2> real_funcimpl_2d;
    typedef FunctionImpl<double,3> real_funcimpl_3d;
    typedef FunctionImpl<double,4> real_funcimpl_4d;
    typedef FunctionImpl<double,5> real_funcimpl_5d;
    typedef FunctionImpl<double,6> real_funcimpl_6d;
    
    typedef FunctionImpl<double_complex,1> complex_funcimpl_1d;
    typedef FunctionImpl<double_complex,2> complex_funcimpl_2d;
    typedef FunctionImpl<double_complex,3> complex_funcimpl_3d;
    typedef FunctionImpl<double_complex,4> complex_funcimpl_4d;
    typedef FunctionImpl<double_complex,5> complex_funcimpl_5d;
    typedef FunctionImpl<double_complex,6> complex_funcimpl_6d;
} 

#endif  // MADNESS_MRA_MRA_H__INCLUDED