/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODCREATORALGS_XAODTRUTHREADER_H
#define XAODCREATORALGS_XAODTRUTHREADER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthPileupEvent.h"

namespace xAODReader {


  /// @short Algorithm demonstrating reading of xAOD truth, and printing to screen
  /// @author James Catmore <James.Catmore@cern.ch>
  /// @author Andy Buckley <Andy.Buckley@cern.ch>
  class xAODTruthReader : public AthReentrantAlgorithm {
  public:

    /// Regular algorithm constructor
    xAODTruthReader(const std::string& name, ISvcLocator* svcLoc);

    /// Function initialising the algorithm
    virtual StatusCode initialize();

    /// Function executing the algorithm
    virtual StatusCode execute(const EventContext& ctx) const;


  private:

    /// The keys for the input xAOD truth containers
    SG::ReadHandleKey<xAOD::TruthEventContainer> m_xaodTruthEventContainerKey{ 
      this, "xAODTruthEventContainerKey", "TruthEvents", "The input TruthEventContainer"};
    // SG::ReadHandleKey<xAOD::TruthPileupEventContainer> m_xaodTruthPUEventContainerKey{ 
    //   this, "xAODTruthPileupEventContainerKey", "TruthPileupEvents", "The input TruthEventContainer for pileup"};
    Gaudi::Property<bool> m_doPUEventPrintout{this, "DoPUEventPrintout", false};

    /// Flag to printout in pt,eta,phi instead of px,py,pz
    Gaudi::Property<bool> m_do4momPtEtaPhi{this, "Do4momPtEtaPhi", false};


    static void printEvent(const xAOD::TruthEventBase* evt,    bool do4momPtEtaPhi);
    static void printVertex(const xAOD::TruthVertex* vtx,      bool do4momPtEtaPhi);
    static void printParticle(const xAOD::TruthParticle* part, bool do4momPtEtaPhi);

  }; // class xAODTruthReader


} // namespace xAODReader

#endif // XAODCREATORALGS_XAODTRUTHREADER_H
