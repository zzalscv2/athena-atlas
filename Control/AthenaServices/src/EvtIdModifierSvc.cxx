///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// EvtIdModifierSvc.cxx 
// Implementation file for class EvtIdModifierSvc
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// AthenaServices includes
#include "EvtIdModifierSvc.h"

// STL includes
#include <algorithm>
#include <set>

// FrameWork includes
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/StoreGateSvc.h"

// EventInfo includes
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"

#define NPLETSZ 6
#define SHIFT_RUNNBR (1<<0)
#define SHIFT_EVTNBR (1<<1)
#define SHIFT_TIMESTAMP (1<<2)
#define SHIFT_LBKNBR (1<<3)

typedef IEvtIdModifierSvc::number_type number_type;

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
EvtIdModifierSvc::EvtIdModifierSvc( const std::string& name, 
                                    ISvcLocator* pSvcLocator ) : 
  ::AthService( name, pSvcLocator ),
  m_evtCounter (1),
  m_current()
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

  declareProperty("Modifiers",
                  m_evtNpletsProp,
                  "A list of n-uplets " \
                  "(RunNumber,EvtNbr,TimeStamp,LumiBlock,Nevents,ModBit).");

  declareProperty("EvtStoreName",
                  m_evtStoreName = "StoreGateSvc",
                  "Name of the event store whose EventIDs will be modified.");

  declareProperty("SkipEvents", 
                  m_firstEvtIdx = 0,
                  "Number of events to skip before modifying EventInfos.");
  declareProperty("SkippedEvents",
                  m_skippedEvents = 0,
                  "Number of events skipped in the EventSelector.");
  m_allEvtsCounter = 0;
}

// Destructor
///////////////
EvtIdModifierSvc::~EvtIdModifierSvc()
{}

// Athena Service's Hooks
////////////////////////////
StatusCode EvtIdModifierSvc::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");

  std::vector<uint64_t>& val = m_evtNpletsProp;
  if (val.size()) {
    // they should be Nplets...
    if (val.size() % NPLETSZ == 0) {
      const std::size_t nbr_plets = val.size() / NPLETSZ;
      m_evtNplets.resize(nbr_plets);
      for (std::size_t i = 0; i != nbr_plets; ++i) {
        m_evtNplets[i] = ItemModifier();
        m_evtNplets[i].runnbr =      val[(i*NPLETSZ)+0];
        m_evtNplets[i].evtnbr =      val[(i*NPLETSZ)+1];
        m_evtNplets[i].timestamp =   val[(i*NPLETSZ)+2];
        m_evtNplets[i].lbknbr =      val[(i*NPLETSZ)+3];
        m_evtNplets[i].nevts  =      val[(i*NPLETSZ)+4];
        m_evtNplets[i].mod_bit = int(val[(i*NPLETSZ)+5]);
      }
    } else {
      ATH_MSG_ERROR("invalid list of n-plets (not divisible by "
                    << NPLETSZ << ")" << endmsg
                    << "check your joboptions !");
      return StatusCode::FAILURE;
    }
  }

  // initialize running total of nevts
  m_numEvtTotals.clear();
  EventID::event_number_t sum = 0;
  for (const auto& elem : m_evtNplets) {
    sum += elem.nevts;
    m_numEvtTotals.push_back(sum);
  }


  if (msgLvl(MSG::DEBUG)) {
    msg(MSG::DEBUG) << "store being modified: ["
                    << m_evtStoreName << "]" << endmsg
                    << "evtid-modifiers: [ ";
    for (ModDb_t::iterator
           itr  = m_evtNplets.begin(), 
           iend = m_evtNplets.end();
         itr != iend;
         ++itr ) {
      const ItemModifier& i = *itr;
      msg(MSG::DEBUG) << "["  << i.runnbr 
                      << ", " << i.evtnbr
                      << ", " << i.timestamp
                      << ", " << i.lbknbr
                      << ", " << i.nevts 
                      << ", mod_bit=0x" << std::hex << i.mod_bit << std::dec
                      << "], ";
    }
    msg(MSG::DEBUG) << "]" << endmsg;
  }

  // initialize our cursor
  m_cursor = m_evtNplets.begin();
  if (m_cursor != m_evtNplets.end()) {
    m_current = *m_cursor;
  } else {
    m_current.runnbr    = 0;
    m_current.evtnbr    = 0;
    m_current.timestamp = 0;
    m_current.lbknbr    = 0;
    m_current.nevts     = 0;
    m_current.mod_bit   = 0;
  }
  m_evtCounter = 1;

  return StatusCode::SUCCESS;
}

StatusCode EvtIdModifierSvc::finalize()
{
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

// Query the interfaces.
//   Input: riid, Requested interface ID
//          ppvInterface, Pointer to requested interface
//   Return: StatusCode indicating SUCCESS or FAILURE.
// N.B. Don't forget to release the interface after use!!!
StatusCode 
EvtIdModifierSvc::queryInterface(const InterfaceID& riid, void** ppvInterface) 
{
  if ( IEvtIdModifierSvc::interfaceID().versionMatch(riid) ) {
    *ppvInterface = dynamic_cast<IEvtIdModifierSvc*>(this);
  } else {
    // Interface is not directly available : try out a base class
    return ::AthService::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}

/////////////////////////////////////////////////////////////////// 
// Const methods: 
///////////////////////////////////////////////////////////////////

/** @brief return the current run-nbr (after modification)
 */
number_type
EvtIdModifierSvc::run_number() const
{
  return m_current.runnbr;
}

/** @brief return the current evt-nbr (after modification)
 */
EventID::event_number_t
EvtIdModifierSvc::event_number() const
{
  return m_current.evtnbr;
}

/** @brief return the current time-stamp (after modification)
 */
number_type
EvtIdModifierSvc::time_stamp() const
{
  return m_current.timestamp;
}

/** @brief return the current lbk-nbr (after modification)
 */
number_type 
EvtIdModifierSvc::lumi_block() const
{
  return m_current.lbknbr;
}

/** @brief tell if the svc modified the run-nbr (for *this* event!)
 */
bool
EvtIdModifierSvc::has_modified_run_number() const
{
  return m_current.mod_bit & SHIFT_RUNNBR;
}

/** @brief tell if the svc modified the evt-nbr (for *this* event!)
 */
bool
EvtIdModifierSvc::has_modified_event_number() const
{
  return m_current.mod_bit & SHIFT_EVTNBR;
}

/** @brief tell if the svc modified the time-stamp (for *this* event!)
 */
bool
EvtIdModifierSvc::has_modified_time_stamp() const
{
  return m_current.mod_bit & SHIFT_TIMESTAMP;
}

/** @brief tell if the svc modified the lbk-nbr (for *this* event!)
 */
bool
EvtIdModifierSvc::has_modified_lumi_block() const
{
  return m_current.mod_bit & SHIFT_LBKNBR;
}

/** @brief return the (sorted) list of run-numbers which will be modified.
 */
std::vector<number_type> 
EvtIdModifierSvc::run_number_list() const
{
  std::set<number_type> runs;

  for (ModDb_t::const_iterator 
         itr = m_evtNplets.begin(), 
         iend= m_evtNplets.end();
       itr != iend;
       ++itr ) {
    if (itr->mod_bit & SHIFT_RUNNBR) {
      runs.insert(itr->runnbr);
    }
  }
  return std::vector<number_type>(runs.begin(), runs.end());
}

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

/** @brief modify an `EventID`'s content.
 */
void
EvtIdModifierSvc::modify_evtid(EventID*& evt_id, EventID::event_number_t evt_index, bool consume_stream)
{
  // Left in to match old observable behaviour:
  // only when consuming stream is required do we check for a matching
  // current StoreGate name (ie: typically the case of being called from a T/P cnv)
  if (consume_stream) {
    StoreGateSvc *active = StoreGateSvc::currentStoreGate();
    if (!active) {
      ATH_MSG_INFO("could not retrieve the active evtstore - bailing out");
      return;
    }

    const std::string& evtStoreName = active->name();
    ATH_MSG_DEBUG("active store: [" << evtStoreName << "]");
    if (evtStoreName != m_evtStoreName) {
      return;
    }

    m_allEvtsCounter++;
  }

  ATH_MSG_DEBUG
    ( "evtid before massaging: "
      << "("  << evt_id->run_number()
      << ", " << evt_id->event_number()
      << ", " << evt_id->time_stamp()
      << ", " << evt_id->lumi_block()
      << ")" );

  // event skipping
  std::int64_t idx = std::int64_t(evt_index) + m_skippedEvents - std::int64_t(m_firstEvtIdx);
  std::int64_t idx_looped = idx % m_numEvtTotals.back();
  ATH_MSG_DEBUG("Got event idx " << evt_index << " --(account for skipping)--> " << idx << " --(modulo #modifiers)--> " << idx_looped);
  if (idx < 0) {
    ATH_MSG_DEBUG("skip event");
    return;
  }

  // Account for events skipped in
  std::size_t mod_idx =
      std::upper_bound(m_numEvtTotals.cbegin(), m_numEvtTotals.cend(), idx_looped) -
      m_numEvtTotals.cbegin();
  auto current = m_evtNplets.at(mod_idx);
  ATH_MSG_DEBUG("Unique modifier index " << mod_idx << " (LB: " << current.lbknbr << ")");
  if (mod_idx >= m_numEvtTotals.size()) {
    // Shouldn't happen
    ATH_MSG_ERROR("Somehow run out of modifiers");
    return;
  }

  if (current.mod_bit & SHIFT_RUNNBR) {
    evt_id->set_run_number(current.runnbr);
  }
  if (current.mod_bit & SHIFT_EVTNBR) {
    evt_id->set_event_number(current.evtnbr);
  }
  if (current.mod_bit & SHIFT_TIMESTAMP) {
    evt_id->set_time_stamp(current.timestamp);
  }
  if (current.mod_bit & SHIFT_LBKNBR) {
    evt_id->set_lumi_block(current.lbknbr);
  }

  m_current = current;

  ATH_MSG_DEBUG
    ( "evtid after  massaging: "
      << "("  << evt_id->run_number()
      << ", " << evt_id->event_number()
      << ", " << evt_id->time_stamp()
      << ", " << evt_id->lumi_block()
      << ")" );
}
