/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTHSIMALGS_NEWMERGEMCEVENTCOLLTOOL_H
#define MCTRUTHSIMALGS_NEWMERGEMCEVENTCOLLTOOL_H

#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteHandleKey.h"

#include "GeneratorObjects/McEventCollection.h"

#include "PileUpTools/PileUpMergeSvc.h"

#include "PileUpTools/PileUpToolBase.h"

/** @class NewMergeMcEventCollTool
 *  @brief a PileUpTool to merge MC truth collection in the overlay store
 *
 */
class NewMergeMcEventCollTool : public PileUpToolBase {
public:
  NewMergeMcEventCollTool(const std::string& type,
                 const std::string& name,
                 const IInterface* parent);
  virtual StatusCode initialize() override final;
  ///called before the subevts loop. Not (necessarily) able to access
  ///SubEvents
  virtual StatusCode prepareEvent(const EventContext& ctx, unsigned int nInputEvents) override final;
  ///called at the end of the subevts loop. Not (necessarily) able to access
  ///SubEvents
  virtual StatusCode mergeEvent(const EventContext& ctx) override final;
  ///called for each active bunch-crossing to process current SubEvents
  /// bunchXing is in ns
  virtual StatusCode
    processBunchXing(int /*bunchXing*/,
                     SubEventIterator bSubEvents,
                     SubEventIterator eSubEvents) override final;

  virtual StatusCode processAllSubEvents(const EventContext& ctx) override final;

private:
  //** Add the required information from the current GenEvent to the output McEventCollection
  StatusCode processEvent(const McEventCollection *pMcEvtColl, McEventCollection *outputMcEventCollection, int pileupType, long timeOffset=0);
  //** Print out detailed debug info if required.
  void printDetailsOfMergedMcEventCollection(McEventCollection *outputMcEventCollection) const;
  //** Handle for the PileUpMergeSvc (provides input McEventCollections)
  ServiceHandle<PileUpMergeSvc> m_pMergeSvc{this, "PileUpMergeSvc", "PileUpMergeSvc", ""};
  //** Name of input McEventCollection
  StringProperty m_truthCollInputKey{this,"TruthCollInputKey", "TruthEvent", ""};
  //** Depends on PileUpTimeEventIndex::PileUpType; provide one instance of this tool for each type
  Gaudi::Property<int> m_pileUpType{this, "PileUpType", -1, ""}; // initialise to PileUpTimeEventIndex::PileUpType::Unknown
  //** Writing to StoreGate safely in MT
  SG::WriteHandle<McEventCollection> m_outputMcEventCollection{};
  //** Writing to StoreGate safely in MT
  SG::WriteHandleKey<McEventCollection> m_truthCollOutputKey{this, "TruthCollOutputKey","TruthEvent",""};
};
#endif //MCTRUTHSIMALGS_NEWMERGEMCEVENTCOLLTOOL_H
