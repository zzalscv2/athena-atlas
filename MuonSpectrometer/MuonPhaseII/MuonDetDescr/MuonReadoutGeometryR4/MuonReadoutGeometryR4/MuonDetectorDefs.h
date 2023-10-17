/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELR4_MUONDETECTORDEFS_H
#define MUONGEOMODELR4_MUONDETECTORDEFS_H

#include <GeoPrimitives/GeoPrimitives.h>
///
#include <MuonReadoutGeometry/ArrayHelper.h>
#include <MuonReadoutGeometry/GlobalUtilities.h>

#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <ActsGeometryInterfaces/RawGeomAlignStore.h>

#include <GeoModelKernel/GeoVAlignmentStore.h>
#include <Identifier/Identifier.h>
#include <Identifier/IdentifierHash.h>

#include <functional>

#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Surfaces/PlanarBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
//// This header contains common helper utilities and definitions
namespace MuonGMR4 {   
    
    /// Checks whether the linear part of the transformation rotates or stetches
    /// any of the basis vectors. Returns false that happens
    bool doesNotDeform(const Amg::Transform3D& trans);
    /// Checks whether the transformation is the Identity transformation
    bool isIdentity(const Amg::Transform3D& trans);

        /** @brief: Comparison struct to construct sets of Acts::Surface bounds with unique elements. Two elements are considered to be identical if
                all of their parameters match within epsilon. Combined with the native insert mechanism of the set, one can easily propagate the same
                bound object to multiple surfaces. 
     **/


    template <typename boundType>
    struct BoundComparer{
         inline bool operator()(const std::shared_ptr<boundType>& a, const std::shared_ptr<boundType>& b) const{          
          const std::vector<double> &avalues{a->values()};
          const std::vector<double> &bvalues{b->values()};
          std::size_t size = avalues.size();
          for(std::size_t i=0; i<size-1; ++i){
            if(std::abs(avalues[i]- bvalues[i]) > std::numeric_limits<double>::epsilon()){
               return avalues[i] < bvalues[i];
            }
          }
          return avalues[size-1] < bvalues[size-1];
        }

    };
  template<class BoundType> using SurfaceBoundSet = std::set<std::shared_ptr<BoundType>, BoundComparer<BoundType>>;
  template<class BoundType> using SurfaceBoundSetPtr = std::shared_ptr<SurfaceBoundSet<BoundType>>;


}  // namespace MuonGMR4

#endif