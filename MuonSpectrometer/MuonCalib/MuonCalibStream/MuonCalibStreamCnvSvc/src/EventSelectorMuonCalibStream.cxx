/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//====================================================================
//	EventSelectorMuonCalibStream.cxx
//      Event loop for calibration stream
//====================================================================
//
// Include files.
#include "MuonCalibStreamCnvSvc/EventSelectorMuonCalibStream.h"

#include "xAODEventInfo/EventInfo.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/StatusCode.h"
#include "MuonCalibStreamCnvSvc/EventContextMuonCalibStream.h"
#include "MuonCalibStreamCnvSvc/IMuonCalibStreamDataProviderSvc.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamAddress.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamInputSvc.h"

// Constructor.
EventSelectorMuonCalibStream::EventSelectorMuonCalibStream(const string &name, ISvcLocator *svcloc) :
    AthService(name, svcloc),
    m_beginIter(nullptr),
    m_endIter(nullptr),
    m_eventSource(nullptr),
    m_dataProvider(nullptr),
    m_SkipEvents(0),
    m_NumEvents(0) {
    ATH_MSG_DEBUG("EventSelectorMuonCalibStream constructor");
    m_eventSourceName = "MuonCalibStreamFileInputSvc";
    m_SkipEvents = 0;
    declareProperty("MuonCalibStreamInputSvc", m_eventSourceName);
    declareProperty("SkipEvents", m_SkipEvents);
}

// Destructor.
EventSelectorMuonCalibStream::~EventSelectorMuonCalibStream() {
    // if(m_eventSource) m_eventSource->release();
    if (m_beginIter) delete m_beginIter;
    if (m_endIter) delete m_endIter;
}

// EventSelectorMuonCalibStream::initialize().
StatusCode EventSelectorMuonCalibStream::initialize() {
    ATH_MSG_INFO("EventSelectorMuonCalibStream::initialize");
    // Check MuonCalibStreamCnvSvc
    IService *svc;
    ATH_CHECK(serviceLocator()->getService(m_eventSourceName, svc));

    m_eventSource = dynamic_cast<MuonCalibStreamInputSvc *>(svc);
    if (m_eventSource == 0) {
        ATH_MSG_ERROR("Cannot cast to MuonCalibStreamInputSvc");
        return StatusCode::FAILURE;
    }
    m_eventSource->addRef();

    ATH_CHECK(service("MuonCalibStreamDataProviderSvc", m_dataProvider));

    // Create the begin and end iterators for this selector.
    m_beginIter = new EventContextMuonCalibStream(this);
    // increment to get the new event in.
    //     ++(*m_beginIter);   ???
    m_endIter = new EventContextMuonCalibStream(0);

    return StatusCode::SUCCESS;
}

StatusCode EventSelectorMuonCalibStream::createContext(IEvtSelector::Context *&it) const {
    it = new EventContextMuonCalibStream(this);
    return (StatusCode::SUCCESS);
}

// Implementation of IEvtSelector::next().
StatusCode EventSelectorMuonCalibStream::next(IEvtSelector::Context &it) const {
    ATH_MSG_DEBUG(" EventSelectorMuonCalibStream::next m_NumEvents=" << m_NumEvents);
    for (;;) {
        const LVL2_MUON_CALIBRATION::CalibEvent *pre = m_eventSource->nextEvent();
        if (!pre) {
            // End of file
            it = *m_endIter;
            return StatusCode::FAILURE;
        }
        ++m_NumEvents;

        // Check if we are skipping events
        if (m_NumEvents > m_SkipEvents) {
            break;
        } else {
            ATH_MSG_DEBUG(" Skipping event " << m_NumEvents - 1);
        }
    }

    return StatusCode::SUCCESS;
}

// Implementation of IEvtSelector::next() with a "jump" parameter
// (to skip over a certain number of events?)
StatusCode EventSelectorMuonCalibStream::next(IEvtSelector::Context &ctxt, int jump) const {
    ATH_MSG_DEBUG(" EventSelectorMuonCalibStream::next skipping events ==" << jump);
    if (jump > 0) {
        for (int i = 0; i < jump; ++i) {
            StatusCode status = next(ctxt);
            if (!status.isSuccess()) { return status; }
        }
        return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::previous(IEvtSelector::Context & /*it*/) const {
    ATH_MSG_ERROR("EventSelectorMuonCalibStream::previous() not implemented");
    return (StatusCode::FAILURE);
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::previous(IEvtSelector::Context &it, int /*jump*/) const { return (previous(it)); }

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::last(IEvtSelector::Context &it) const {
    if (it.identifier() == m_endIter->identifier()) {
        ATH_MSG_DEBUG("last(): Last event in InputStream.");
        return (StatusCode::SUCCESS);
    }
    return (StatusCode::FAILURE);
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::resetCriteria(const std::string & /*criteria*/, IEvtSelector::Context & /*ctxt*/) const {
    return (StatusCode::SUCCESS);
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::rewind(IEvtSelector::Context & /*it*/) const {
    ATH_MSG_ERROR("EventSelectorMuonCalibStream::rewind() not implemented");
    return (StatusCode::FAILURE);
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::createAddress(const IEvtSelector::Context & /*it*/, IOpaqueAddress *&iop) const {
    ATH_MSG_DEBUG("EventSelectorMuonCalibStream::createAddress");
    const LVL2_MUON_CALIBRATION::CalibEvent *pre = m_eventSource->currentEvent();
    m_dataProvider->setNextEvent(pre);
    ATH_MSG_DEBUG("Calib Event cached in Data Provider ");

    iop = new MuonCalibStreamAddress(ClassID_traits<xAOD::EventInfo>::ID(), "EventInfo", ""); // change to xAOD::EventInfo key
    //iop = new MuonCalibStreamAddress(ClassID_traits<xAOD::EventInfo>::ID(), "MuonCalibStreamEventInfo", ""); // old key which need the conversion afterwards
    ATH_MSG_DEBUG("MuonCalibStreamAddress for MuonCalibStreamEventInfo created ");

    return (StatusCode::SUCCESS);
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::releaseContext(IEvtSelector::Context *& /*it*/) const { return (StatusCode::SUCCESS); }

// Implementation of IInterface::queryInterface.
StatusCode EventSelectorMuonCalibStream::queryInterface(const InterfaceID &riid, void **ppvInterface) {
    if (riid == IEvtSelector::interfaceID()) {
        *ppvInterface = (IEvtSelector *)this;
    } else if (riid == IProperty::interfaceID()) {
        *ppvInterface = (IProperty *)this;
    } else {
        return AthService::queryInterface(riid, ppvInterface);
    }

    addRef();
    return StatusCode::SUCCESS;
}
