/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CosmicEventFilterTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_TOOLS_COSMICEVENTFILTERTOOL_H
#define ISF_TOOLS_COSMICEVENTFILTERTOOL_H 1

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKeyArray.h"
// EDM
#include "TrackRecord/TrackRecordCollection.h"

// ISF includes
#include "ISF_Interfaces/IEventFilterTool.h"

#include <atomic>

namespace ISF {

  /** @class CosmicEventFilterTool

      the code used in this implementation is kindly stolen from:
      atlasoff:: Simulation/G4Extentions/G4CosmicFilter/

      @author John Chapman -at- cern.ch
     */
  class CosmicEventFilterTool : public extends<AthAlgTool, IEventFilterTool> {

  public:
    /** Constructor with parameters */
    CosmicEventFilterTool( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    ~CosmicEventFilterTool();

    // Athena algtool's Hooks
    virtual StatusCode  initialize() override final;
    virtual StatusCode  finalize() override final;

    /** Check that the current event passes this filter */
    virtual bool eventPassesFilter() const override final;

  private:
    mutable std::atomic<unsigned int> m_ntot;
    mutable std::atomic<unsigned int> m_npass;
    SG::ReadHandleKeyArray<TrackRecordCollection> m_VolumeNames;
    bool   m_useANDFilter;
    int    m_magicID;
    double m_ptMin;
    double m_ptMax;
  };

}


#endif //> !ISF_TOOLS_COSMICEVENTFILTERTOOL_H
