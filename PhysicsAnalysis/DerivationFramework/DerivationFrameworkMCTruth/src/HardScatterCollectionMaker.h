/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// HardScatterCollectionMaker.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_HARDSCATTERCOLLECTIONMAKER_H
#define DERIVATIONFRAMEWORK_HARDSCATTERCOLLECTIONMAKER_H

// Base classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "xAODTruth/TruthParticleContainer.h"


namespace DerivationFramework {

  class HardScatterCollectionMaker : public AthAlgTool, public IAugmentationTool {
    public: 
      HardScatterCollectionMaker(const std::string& t, const std::string& n, const IInterface* p);
      ~HardScatterCollectionMaker();
      StatusCode initialize();
      virtual StatusCode addBranches() const;

    private:
      Gaudi::Property<std::string> m_collectionName{this, "NewCollectionName", "" }; //!< Output collection name stem
      Gaudi::Property<int> m_generations{this, "Generations", 1,
      "Number of generations after the particle in question to keep (-1 for all)"};
      ServiceHandle<StoreGateSvc> m_metaStore{this, "MetaDataStore", "MetaDataStore"}; //!< Handle on the metadata store for init

      SG::ReadHandleKey<xAOD::TruthEventContainer> m_eventsKey{this, "TruthEventKey", "TruthEvents",
                                                      "Input particle collection (navigates to the vertices)"};
      SG::WriteHandleKey<xAOD::TruthVertexContainer> m_outVtxKey{this, "OutVtxContainer", "" };
      SG::WriteHandleKey<xAOD::TruthParticleContainer> m_outPartKey{this, "OutPartContainer", "" };
  }; 
}

#endif // DERIVATIONFRAMEWORK_HARDSCATTERCOLLECTIONMAKER_H
