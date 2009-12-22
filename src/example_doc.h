
/*! 
  \file src/example_doc.h
  \brief Refer to this file to understand how to document source
  \defgroup exampledoc Example documentation with doxygen

  This module illustrates how to document your files with
  doxygen. Please use a regular editor to view example_doc.h and
  example_doc.cc in order to understand how this documentation was
  generated.

  Every file needs \c \\file and \c \\brief near the top, and usually
  a \c \\defgroup or \c \\ingroup. If you have put the file in a group
  then the file-level documentation acts as documentation for that
  module or group, otherwise it acts as documentation for the file.
  Note that doxygen is really picky about placement and association of
  comments so you always have to check what was generated.

  Links to known classes (such as ExampleClass1), functions (such as
  example_func()), and files (such as example_doc.h) are made
  automatically.  

  \par Subsection title

  Use the \c \\par directive to make subsections with an
  optional heading.  Doxygen's section and subsection directives
  should not be used for now.
*/

// @{

/// Every class should be documented (this is the brief line)

/// This is the full text ... no need to be verbose but without
/// documentation your class is nearly useless.
class ExampleClass1 {
public:
    int datum; ///< Each member datum needs at least a brief raison d'etre

    /// Each member function should be documented (this is the brief line)

    /// Full documentation on member functions
    /// that computes \f$ e^2 \frac{hope}{despair} \f$.
    ///
    /// You should document all arguments and return values.
    /// @param[in] hope Optimism level
    /// @param[in] despair Pessimism level
    /// @return Cynicism level
    int reality(int hope, int despair) const;
};

/*!
  \brief Brief documentation of second example class in group
  
  Full documentation of second example class
*/
class ExampleClass2 {
public:
    /// Brief documentation on member function

    /// Full documentation on member function including parameters
    /// and return value.
    /// @param[in] fud The current level of fear, uncertainty and doubt
    /// @param[in,out] bs The current level of ...
    /// @return Your state of mind.
    int morose(int fud, int& bs) {};

    void fred(double mary);    /* Fred is documented in example_doc.cc */
};


/// \defgroup examplesub Example subgroup documentation
// @{
int stuff; ///< Global variable somewhere in example subgroup

/* Closing subgroup membership */
// @}

/// Global function in example group

/// @param[in] a The input argument is ...
/// @return The return value is ...
int example_func(int a);

/* Closing group membership */
// @}




  
