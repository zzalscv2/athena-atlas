/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

/**
 * Simple algorithm used to filter events by Event Number
 */
class EventNumberFilterAlgorithm : public AthReentrantAlgorithm {

  public:
    EventNumberFilterAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ) : AthReentrantAlgorithm(name,pSvcLocator) {}

    virtual ~EventNumberFilterAlgorithm()=default;
    virtual StatusCode initialize() override {
        m_evtNumbersSet = std::set<unsigned long>(m_evtNumbers.value().begin(),m_evtNumbers.value().end());
        return AthReentrantAlgorithm::initialize();
    }
    virtual StatusCode execute(const EventContext& ctx) const override {
        setFilterPassed( m_evtNumbersSet.count(ctx.eventID().event_number())>0, ctx );
        return StatusCode::SUCCESS;
    }

    Gaudi::Property<std::vector<unsigned long>> m_evtNumbers{this,"EventNumbers",{},"List of event numbers to accept"};
    std::set<unsigned long> m_evtNumbersSet;


};