///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// METTrackFilterTool.cxx 
// Implementation file for class METTrackFilterTool
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo
/////////////////////////////////////////////////////////////////// 

// METReconstruction includes
#include "METReconstruction/METTrackFilterTool.h"

// MET EDM
#include "xAODMissingET/MissingETComposition.h"
#include "xAODMissingET/MissingETAuxContainer.h"
#include "xAODMissingET/MissingETAuxComponentMap.h"

// Tracking EDM
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

// Calo EDM
#include "xAODCaloEvent/CaloClusterContainer.h"

// Egamma EDM
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODEgamma/Electron.h"

// Track errors
#include "EventPrimitives/EventPrimitivesHelpers.h"

// ConstDV
#include "AthContainers/ConstDataVector.h"

namespace met {

  using std::vector;
  //
  using namespace xAOD;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 

  // Constructors
  ////////////////
  METTrackFilterTool::METTrackFilterTool(const std::string& name) : 
    AsgTool(name),
    METRefinerTool(name)
  {
    declareProperty( "DoPVSel",           m_trk_doPVsel = true              );
    // declareProperty( "TrackD0Max",      m_trk_d0Max = 1.5                    );
    // declareProperty( "TrackZ0Max",      m_trk_z0Max = 1.5                    );
    declareProperty( "InputPVKey",        m_pv_inputkey = "PrimaryVertices" );
    declareProperty( "DoEoverPSel",       m_trk_doEoverPsel = false             );
    declareProperty( "InputClusterKey",   m_cl_inputkey = "CaloCalTopoClusters" );
    declareProperty( "InputElectronKey",  m_el_inputkey = "Electrons"       );
    declareProperty( "InputMuonKey",      m_mu_inputkey = "Muons"           );

    declareProperty( "DoVxSep",           m_doVxSep = false                 );
    declareProperty( "TrackSelectorTool", m_trkseltool                      );
    declareProperty( "TrackVxAssocTool",  m_trkToVertexTool                 );
    declareProperty( "DoLepRecovery",     m_doLepRecovery=false             );
  }

  // Destructor
  ///////////////
  METTrackFilterTool::~METTrackFilterTool()
  {}

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode METTrackFilterTool::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");
    ATH_CHECK(m_trkseltool.retrieve());
    ATH_CHECK(m_trkToVertexTool.retrieve());

    if(m_doVxSep) ATH_MSG_INFO("Building TrackMET for each vertex");

    return StatusCode::SUCCESS;
  }

  StatusCode METTrackFilterTool::finalize()
  {
    ATH_MSG_INFO ("Finalizing " << name() << "...");

    return StatusCode::SUCCESS;
  }

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /////////////////////////////////////////////////////////////////// 
  // Protected methods: 
  /////////////////////////////////////////////////////////////////// 

  // Implement for now, but should move to common tools when possible
  // bool METTrackFilterTool::isPVTrack(const xAOD::TrackParticle* /*trk*/,
  // 				     const xAOD::Vertex* /*pv*/) const
  // {
  //   if(!trk || !pv) return false;
  //   if(fabs(trk->d0())>m_trk_d0Max) return false;
  //   if(fabs(trk->z0()+trk->vz() - pv->z()) > m_trk_z0Max) return false;
  //   return true;
  // }

  bool METTrackFilterTool::isGoodEoverP(const xAOD::TrackParticle* trk,
					const std::vector<const xAOD::IParticle*>& trkList,
					const xAOD::CaloClusterContainer* clusters) const {

    if( (fabs(trk->eta())<1.5 && trk->pt()>200e3) ||
    	(fabs(trk->eta())>=1.5 && trk->pt()>120e3) ) {

      // Get relative error on qoverp
      float Rerr = Amg::error(trk->definingParametersCovMatrix(),4)/fabs(trk->qOverP());
      ATH_MSG_VERBOSE( "Track momentum error (%): " << Rerr*100 );

      // first compute track and calo isolation variables
      float ptcone20 = 0;
      for(std::vector<const IParticle*>::const_iterator iTrk=trkList.begin();
	  iTrk!=trkList.end(); ++iTrk) {
	const TrackParticle* testtrk = dynamic_cast<const TrackParticle*>(*iTrk);
	if(testtrk==trk) continue;
	if(testtrk->p4().DeltaR(trk->p4()) < 0.2) {
	  ptcone20 += testtrk->pt();
	}
      }
      float isolfrac = ptcone20 / trk->pt();
      ATH_MSG_VERBOSE( "Track isolation fraction: " << isolfrac );

      float etcone10 = 0.;
      for(CaloClusterContainer::const_iterator iClus=clusters->begin();
	  iClus!=clusters->end(); ++iClus) {
	if((*iClus)->p4().DeltaR(trk->p4()) < 0.1) {
	  etcone10 += (*iClus)->pt();
	}
      }
      float EoverP = etcone10/trk->pt();
      ATH_MSG_VERBOSE( "Track E/P: " << EoverP );

      if(isolfrac<0.1) {
	// isolated track cuts
	if(Rerr>0.4) return false;
	else if (EoverP<0.65 && (EoverP>0.1 || Rerr>0.1)) return false;
      } else {
	// non-isolated track cuts
	float trkptsum = ptcone20+trk->pt();
	if(EoverP/trkptsum<0.6 && ptcone20/trkptsum<0.6) return false;
      }

    }

    return true;
  }

  StatusCode METTrackFilterTool::executeTool(xAOD::MissingET* metTerm, xAOD::MissingETComponentMap* metMap) {

    ATH_MSG_DEBUG ("In execute: " << name() << "...");

    std::vector<const xAOD::Electron*> selElectrons;
    std::vector<const xAOD::Muon*> selMuons;

    if(m_doLepRecovery)
    {
      const ElectronContainer* elCont;
      const MuonContainer* muCont;
      ATH_CHECK(evtStore()->retrieve(elCont,m_el_inputkey));
      ATH_CHECK(evtStore()->retrieve(muCont,m_mu_inputkey));

      selectElectrons(*elCont, selElectrons);
      selectMuons(*muCont, selMuons);
    }     

    // const CaloClusterContainer* cl_cont = 0;
    // if(m_trk_doEoverPsel) {
    //   if( evtStore()->retrieve( cl_cont, m_cl_inputkey).isFailure() ) {
    //     ATH_MSG_WARNING("Unable to retrieve input calocluster container");
    //     return StatusCode::FAILURE;
    //   }
    // }

    MissingETComponentMap::iterator iter = MissingETComposition::find(metMap,metTerm);
    if(iter==metMap->end()) {
      ATH_MSG_WARNING("Could not find current METComponent in MET Map!");
      return StatusCode::FAILURE;
    }
    MissingETComponent* newComp = *iter;
    newComp->setStatusWord(MissingETBase::Status::Tags::correctedTerm(MissingETBase::Status::Nominal,
								      MissingETBase::Status::PileupTrack));

    // Extract the component corresponding to the Track SoftTerms
    MissingETBase::Types::bitmask_t src_ST_idTrk = MissingETBase::Source::SoftEvent | MissingETBase::Source::idTrack();
    MissingETBase::Types::bitmask_t src_ST_refTrk = MissingETBase::Source::softEvent() | MissingETBase::Source::track();
    metTerm->setSource(src_ST_refTrk);

    MissingETComponentMap::const_iterator citer = MissingETComposition::find(metMap,src_ST_idTrk);
    if(citer==metMap->end()) {
      ATH_MSG_WARNING("Could not find Soft ID Track component in MET Map!");
      return StatusCode::FAILURE;
    }
    vector<const TrackParticle*> softTracks;
    softTracks.reserve((*citer)->size());
    for(const auto& obj : (*citer)->objects()) {
      const TrackParticle* trk = dynamic_cast<const TrackParticle*>(obj);
      if(trk) {softTracks.push_back(trk);}
      else {ATH_MSG_WARNING("Track filter given an object of type " << obj->type());}
    }

    const Vertex* pv=0;
    const VertexContainer* vxCont = 0;
    vector<const Vertex*> vertices;
    if(m_trk_doPVsel) {
      if( evtStore()->retrieve( vxCont, m_pv_inputkey).isFailure() ) {
        ATH_MSG_WARNING("Unable to retrieve input primary vertex container");
        return StatusCode::FAILURE;
      }
      if(vxCont->size()>0) {
	vertices.reserve(vxCont->size());
	for(const auto& vx : *vxCont) {
	  if(vx->vertexType()==VxType::PriVtx) {pv = vx;}
	  vertices.push_back(vx);
	}
	ATH_MSG_DEBUG("Main primary vertex has z = " << pv->z());
      } else{
	ATH_MSG_WARNING("Event has no primary vertices");
	return StatusCode::FAILURE;
      }
      if(!pv) {
	ATH_MSG_WARNING("Did not find a primary vertex in the container.");
	return StatusCode::FAILURE;
      }
    }

    if(m_doVxSep) {
      xAOD::TrackVertexAssociationMap trktovxmap=m_trkToVertexTool->getUniqueMatchMap(softTracks, vertices);

      // initialize metContainer and metTerm
      MissingETContainer* metCont = static_cast<MissingETContainer*>( metTerm->container() );

      bool firstVx(true);
      std::string basename = metTerm->name()+"_vx";
      for(const auto& vx : *vxCont){
	if(vx->vertexType()==VxType::PriVtx || vx->vertexType()==VxType::PileUp) {
	  MissingET *met_vx = metTerm;
	  if(!firstVx) {
	    met_vx = new MissingET(0. ,0. ,0. );
	    metCont->push_back(met_vx);
	    met_vx->setSource(metTerm->source());
	    MissingETComposition::add(metMap, met_vx);
	  }
	  met_vx->setName(basename+std::to_string(vx->index()));
	  ATH_MSG_VERBOSE("Building " << met_vx->name());

	  ATH_MSG_VERBOSE("Number of tracks associated to vertex " << vx->index() << ": "<< (trktovxmap[vx]).size());
	  
	  ATH_CHECK( buildTrackMET(metMap,metTerm,vx,selElectrons,selMuons,trktovxmap[vx]) );
	  firstVx = false;
	}
      }
    } else {
      ATH_CHECK( buildTrackMET(metMap,metTerm,pv,selElectrons,selMuons,softTracks) );
    }

    return StatusCode::SUCCESS;
  }

  StatusCode METTrackFilterTool::buildTrackMET(xAOD::MissingETComponentMap* const metMap,
					       xAOD::MissingET* const metTerm,
					       const xAOD::Vertex* const pv,
					       const std::vector<const xAOD::Electron*>& selElectrons,
					       const std::vector<const xAOD::Muon*>& selMuons,
					       const std::vector<const xAOD::TrackParticle*>& softTracks) const {

    vector<const IParticle*> dummyList;

    const MissingETComponent* metComp = MissingETComposition::getComponent(metMap,metTerm);
    if(!metComp) {
      ATH_MSG_WARNING("Failed to find MissingETComponent for MET term " << metTerm->name());
      return StatusCode::FAILURE;
    }
    // Loop over the tracks and select only good ones
    for( const auto& trk : softTracks ) {
      MissingETBase::Types::weight_t trackWeight = metComp->weight(trk);
      // Could/should use common implementation of addToMET here -- derive builder and refiner from a common base tool?
      bool passFilters = true;
      //      if(m_trk_doPVsel && !isPVTrack(trk,(*vxCont)[0])) passFilters = false;
      //      if(m_trk_doEoverPsel && !isGoodEoverP(trk,softTracks,cl_cont)) passFilters = false;
      if(m_trk_doPVsel) {
	if(!(m_trkseltool->accept( *trk, pv ))) passFilters=false;
      } else {
	if(!(m_trkseltool->accept( trk ))) passFilters=false;
      }

      bool isMuon=false;
      bool isElectron=false;
      size_t el_index=-1;

      if(m_doLepRecovery) {
        isMuon=isMuTrack(*trk,selMuons);
        isElectron=isElTrack(*trk,selElectrons, el_index);
      }

      bool isLepton=(isMuon||isElectron);

      if(passFilters || (m_doLepRecovery && isLepton)) {
        if(!passFilters && isElectron && m_doLepRecovery) {
	  //electron track fails, replace with electron pt
          const Electron* el = selElectrons[el_index];

          metTerm->add(el->pt()*cos(trk->phi())*trackWeight.wpx(),
              el->pt()*sin(trk->phi())*trackWeight.wpy(),
              el->pt()*trackWeight.wet());
          MissingETComposition::insert(metMap,metTerm,el,dummyList,trackWeight);
        } else {
	  ATH_MSG_VERBOSE("Add track with pt " << trk->pt() <<" to MET.");
	  metTerm->add(trk->pt()*cos(trk->phi())*trackWeight.wpx(),
		       trk->pt()*sin(trk->phi())*trackWeight.wpy(),
		       trk->pt()*trackWeight.wet());
	  MissingETComposition::insert(metMap,metTerm,trk,dummyList,trackWeight);
        }
      }
    }
    return StatusCode::SUCCESS;
  }

  bool METTrackFilterTool::isElTrack(const xAOD::TrackParticle &trk, const std::vector<const xAOD::Electron*>& electrons, size_t &el_index) const
  {
    for(unsigned int eli=0; eli<electrons.size(); ++eli) {
      const xAOD::Electron *el=electrons.at(eli);
      if(&trk==xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF(el->trackParticle())) {
	el_index=eli;
	return true;
      }
    }
    return false;
  }

  bool METTrackFilterTool::isMuTrack(const xAOD::TrackParticle &trk, const std::vector<const xAOD::Muon*>& muons) const
  {
    for(unsigned mui=0;mui<muons.size();mui++) {
      //        if(((muon_list.at(mui))->trackParticle(xAOD::Muon::InnerDetectorTrackParticle))->pt()==trk->pt())
      if(((muons.at(mui))->trackParticle(xAOD::Muon::InnerDetectorTrackParticle))==&trk) {
	return true;
      }
    }
    return false;
  }

  void METTrackFilterTool::selectElectrons(const xAOD::ElectronContainer &elCont, std::vector<const xAOD::Electron*>& electrons) const
  {
    for(unsigned int eli=0; eli< elCont.size(); eli++) {
      const xAOD::Electron *el=elCont.at(eli);
      if( (el)->author()&0x1  //electron author
	  && (el)->pt()>10000   // electron pt
	  && fabs(el->eta())<2.47  // electron eta
          ) {
	if(!((fabs((el)->eta())>1.37) && (fabs((el)->eta())<1.52) )) {
	  // crack region
	  electrons.push_back(el);
	}
      }
    }
  }
  

  void METTrackFilterTool::selectMuons(const xAOD::MuonContainer &muCont, std::vector<const xAOD::Muon*>& muons) const
  {
    for(unsigned int mui=0; mui<muCont.size();mui++) {
      const xAOD::Muon *mu=muCont.at(mui);
      if( (mu->muonType()==xAOD::Muon::Combined)
	  && (mu->pt()>6000.)
	  && fabs(mu->eta())<2.5
	  ) {
	muons.push_back(mu);
      }
    }
  }


  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

}

