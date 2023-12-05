// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODROOTACCESS_TOOLS_TAUXVECTOR_H
#define XAODROOTACCESS_TOOLS_TAUXVECTOR_H

// EDM include(s):
#include "AthContainersInterfaces/IAuxTypeVector.h"
#include "AthContainersInterfaces/IAuxTypeVectorFactory.h"

// Forward declaration(s):
class TClass;
class TVirtualCollectionProxy;
namespace SG {
   class IAuxTypeVectorFactory;
}

namespace xAOD {

   /// Auxiliary vector type for types known to ROOT
   ///
   /// This class is used for types known to ROOT, which have not received a
   /// concrete auxiliary vector type yet. (By having been accessed explicitly.)
   ///
   /// The code is pretty much a copy of what Scott wrote for RootStorageSvc for
   /// the offline code.
   ///
   /// @author Scott Snyder <Scott.Snyder@cern.ch>
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TAuxVector : public SG::IAuxTypeVector {

   public:
      /// Constructor
      TAuxVector( const SG::IAuxTypeVectorFactory* factory,
                  SG::auxid_t auxid,
                  const ::TClass* cl, size_t size, size_t capacity );
      /// Copy constructor
      TAuxVector( const TAuxVector& parent );
      /// Destructor
      ~TAuxVector();

      /// Assignment operator
      TAuxVector& operator= ( const TAuxVector& other );

      /// @name Function implementing the SG::IAuxTypeVector interface
      /// @{

      /// Copy the managed vector
      virtual std::unique_ptr< SG::IAuxTypeVector > clone() const override;

      /// Return the auxid of the variable this vector represents.
      virtual SG::auxid_t auxid() const override;
      /// Return a pointer to the start of the vector's data
      virtual void* toPtr() override;
      const void* toPtr() const;
      /// Return a pointer to the STL vector itself
      virtual void* toVector() override;

      /// Return the size of the vector
      virtual size_t size() const override;

      /// Change the size of the vector
      virtual bool resize( size_t sz ) override;
      /// Change the capacity of the vector
      virtual void reserve( size_t sz ) override;
      /// Shift the elements of the vector
      virtual void shift( size_t pos, ptrdiff_t offs ) override;
      /// Insert a range of elements via move.
      virtual bool insertMove (size_t pos, void* beg, void* end) override;

      /// @}

   private:
      /// Function copying the payload of a range to a new location
      void copyRange( const void* src, void* dst, size_t n );
      // Function clearing the payload of a given range
      void clearRange( void* dst, size_t n );

      /// The parent factory object
      const SG::IAuxTypeVectorFactory* m_factory;
      /// ROOT's description of the vector type
      /// Cloned from the proxy held by the TClass and permanently bound
      /// to m_vec.  That makes things a bit more efficient, and prevents
      /// potential thread-safety problems.
      std::unique_ptr<::TVirtualCollectionProxy> m_proxy;
      /// Pointer to the vector object
      void* m_vec;
      /// ID of the variable we represent.
      SG::auxid_t m_auxid;

   }; // class TAuxVector

} // namespace xAOD

#endif // XAODROOTACCESS_TOOLS_TAUXVECTOR_H
