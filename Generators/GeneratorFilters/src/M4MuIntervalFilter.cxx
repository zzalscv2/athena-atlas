/*
  Copyright (C) 2020-2022 CERN for the benefit of the ATLAS collaboration
*/

// Header for this module
#include "GeneratorFilters/M4MuIntervalFilter.h"
#include "TruthUtils/HepMCHelpers.h"

#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"

// Pt  High --> Low
namespace {
  class High2LowByPt {
  public:
    bool operator () (HepMC::FourVector t1, HepMC::FourVector t2) {
      return (t1.perp() > t2.perp());
    }
  };
} // namespace


M4MuIntervalFilter::M4MuIntervalFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name, pSvcLocator)
{
  declareProperty("MaxEta", m_maxEta = 5.0);
  declareProperty("MinPt", m_minPt = 1000);
  declareProperty("LowM4muProbability", m_prob2low = 1.0);
  declareProperty("MediumMj4muProbability", m_prob2medium = 0.5);
  declareProperty("HighM4muProbability", m_prob2high = 0.1);
  declareProperty("LowM4mu", m_m4mulow=11000);
  declareProperty("HighM4mu", m_m4muhigh=25000);
  declareProperty("ApplyReWeighting", m_ApplyReWeighting = true);
}

M4MuIntervalFilter::~M4MuIntervalFilter(){}

StatusCode M4MuIntervalFilter::filterInitialize() {

  CHECK(m_rndmSvc.retrieve());

  ATH_MSG_DEBUG( "MaxEta           "  << m_maxEta);
  ATH_MSG_DEBUG( "MinPt          "  << m_minPt);
  ATH_MSG_DEBUG( "LowM4muProbability         "  << m_prob2low);
  ATH_MSG_DEBUG( "MediumMj4muProbability         "  << m_prob2medium);
  ATH_MSG_DEBUG( "HighM4muProbability         "  << m_prob2high);
  ATH_MSG_DEBUG( "LowM4mu         "  << m_m4mulow);
  ATH_MSG_DEBUG( "HighM4mu         "  << m_m4muhigh);
  ATH_MSG_DEBUG( "ApplyReWeighting         "  << m_ApplyReWeighting);
  return StatusCode::SUCCESS;
}

StatusCode M4MuIntervalFilter::filterFinalize() {
  return StatusCode::SUCCESS;
}

StatusCode M4MuIntervalFilter::filterEvent() {
  // Get random number engine
  const EventContext& ctx = Gaudi::Hive::currentContext();
  CLHEP::HepRandomEngine* rndm = this->getRandomEngine(name(), ctx);
  if (!rndm) {
    ATH_MSG_ERROR("Failed to retrieve random number engine M4MuIntervalFilter");
    setFilterPassed(false);
    return StatusCode::FAILURE;
  }

  // Find overlap objects
  std::vector<HepMC::FourVector> MCTruthMuonList;
  
  for (McEventCollection::const_iterator itr = events()->begin(); itr != events()->end(); ++itr) {
    const HepMC::GenEvent* genEvt = (*itr);
    for (const auto& pitr: *genEvt){

	   // muon
	   if (MC::isMuon(pitr) && MC::isStable(pitr) &&
	      (pitr)->momentum().perp() >= m_minPt &&
	       std::abs((pitr)->momentum().pseudoRapidity()) <= m_maxEta) {
           HepMC::FourVector tmp((pitr)->momentum().px(), (pitr)->momentum().py(), (pitr)->momentum().pz(), (pitr)->momentum().e());
           MCTruthMuonList.push_back(tmp);
	       }
    }
  }
  
  std::sort(MCTruthMuonList.begin(), MCTruthMuonList.end(), High2LowByPt());

  if(MCTruthMuonList.size()<4){
    setFilterPassed(false);
    ATH_MSG_DEBUG("Less than 4 muons. The muon number is " << MCTruthMuonList.size());
    return StatusCode::SUCCESS;
  }
  
  if(m_ApplyReWeighting) {
    HepMC::FourVector vec4mu(MCTruthMuonList.at(0).px() + MCTruthMuonList.at(1).px() + MCTruthMuonList.at(2).px() + MCTruthMuonList.at(3).px(), 
                   MCTruthMuonList.at(0).py() + MCTruthMuonList.at(1).py() + MCTruthMuonList.at(2).py() + MCTruthMuonList.at(3).py(),
                   MCTruthMuonList.at(0).pz() + MCTruthMuonList.at(1).pz() + MCTruthMuonList.at(2).pz() + MCTruthMuonList.at(3).pz(),
                   MCTruthMuonList.at(0).e() + MCTruthMuonList.at(1).e() + MCTruthMuonList.at(2).e() + MCTruthMuonList.at(3).e() );
    
    double m4mu = vec4mu.m();
    double eventWeight = 1.0;
    eventWeight = getEventWeight(m4mu);
    double rnd = rndm->flat();
    if (1.0/eventWeight < rnd) {
      setFilterPassed(false);
      ATH_MSG_DEBUG("Event failed weighting. Weight is " << eventWeight);
      return StatusCode::SUCCESS;
    }

    // Get MC event collection for setting weight
    const McEventCollection* mecc = 0;
    if ( evtStore()->retrieve( mecc ).isFailure() || !mecc ){
      setFilterPassed(false);
      ATH_MSG_ERROR("Could not retrieve MC Event Collection - weight might not work");
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Event passed.  Will weight events " << eventWeight);
    McEventCollection* mec = const_cast<McEventCollection*> (&(*mecc));
    for (unsigned int i = 0; i < mec->size(); ++i) {
      if (!(*mec)[i]) continue;
      double existingWeight = (*mec)[i]->weights().size()>0 ? (*mec)[i]->weights()[0] : 1.;
      if ((*mec)[i]->weights().size()>0) {
        (*mec)[i]->weights()[0] = existingWeight*eventWeight;
      } else {
        (*mec)[i]->weights().push_back( eventWeight*existingWeight );
      }
    }
  }
  // Made it to the end - success!
  setFilterPassed(true);
  return StatusCode::SUCCESS;
}


double M4MuIntervalFilter::getEventWeight(double mass) const {
  double weight = 1.0;
  if (mass < m_m4mulow) {
       	weight /= m_prob2low;

  } else if (mass > m_m4muhigh) {
	       weight /= m_prob2high;

  } else {
         weight /= m_prob2medium;
    }
  ATH_MSG_DEBUG("WEIGHTING:: " << mass << "\t" << weight);
  return weight;
}


CLHEP::HepRandomEngine* M4MuIntervalFilter::getRandomEngine(const std::string& streamName,
                                                                const EventContext& ctx) const
{
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, streamName);
  std::string rngName = name()+streamName;
  rngWrapper->setSeed( rngName, ctx );
  return rngWrapper->getEngine(ctx);
}
