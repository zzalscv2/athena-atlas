/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackParticleMerger.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef DERIVATIONFRAMEWORK_TrackParticleMerger_H
#define DERIVATIONFRAMEWORK_TrackParticleMerger_H

#include <string>
#include <map>
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthContainers/ConstDataVector.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "ExpressionEvaluation/ExpressionParserUser.h"

#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"

namespace DerivationFramework {

  /** @brief Class-algorithm for track particle collection merging*/
  class TrackParticleMerger : public AthAlgTool, public IAugmentationTool 
    {
      
    public:
      
      ///////////////////////////////////////////////////////////////////
      /** @brief Standard Algotithm methods:                           */
      ///////////////////////////////////////////////////////////////////

      TrackParticleMerger(const std::string& t, const std::string& n, const IInterface* p);
      virtual ~TrackParticleMerger() = default;
      virtual StatusCode initialize() override;
      virtual StatusCode addBranches() const override;
 
    protected:

      ///////////////////////////////////////////////////////////////////
      /** @brief Protected data:                                       */
      ///////////////////////////////////////////////////////////////////
      SG::ReadHandleKeyArray<xAOD::TrackParticleContainer>      m_trackParticleLocation; /** Vector of track collections to be merged. */
      SG::WriteHandleKey<ConstDataVector<xAOD::TrackParticleContainer>>          m_outtrackParticleLocation  ;  /** Combined track collection.   */
      SG::WriteHandleKey<xAOD::TrackParticleAuxContainer>       m_outtrackParticleAuxLocation  ;  /** Combined track collection.   */

      ///////////////////////////////////////////////////////////////////
      /** @brief Protected methods:                                    */
      ///////////////////////////////////////////////////////////////////

      /** @brief A routine that merges the track collections. */
      void mergeTrackParticle(const xAOD::TrackParticleContainer* trackParticleCol,
                              ConstDataVector<xAOD::TrackParticleContainer>* outputCol) const;

    private:
      Gaudi::Property<bool>  m_createViewCollection{this, "CreateViewColllection", true};     //!< option to create a view collection and not deep-copy tracks 
    };
    
}
#endif // DERIVATIONFRAMEWORK_TrackParticleMerger_H
