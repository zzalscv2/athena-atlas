/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration 
*/
// -------------------------------------------------------------
// File: GeneratorFilters/MuDstarFilter.cxx
// Description:
//
//   Allows the user to search for (mu D*) combinations
//   with both same and opposite charges.
//   For D*+-, the decay D*+ -> D0 pi_s+ is selected
//   with D0 in the nominal decay mode, D0 -> K- pi+ (Br = 3.947%),
//   and, if "D0Kpi_only=false", 14 main other decays to 2 or 3 particles (except nu_e and nu_mu)
//   in case they can immitate the nominal decay:
//   D0 -> K-mu+nu, K-e+nu, pi-mu+nu, pi-e+nu,
//         K-mu+nu pi0, K-e+nu pi0, pi-mu+nu pi0, pi-e+nu pi0,
//         pi-pi+, K-K+, K-pi+pi0, K-K+pi0, pi-pi+pi0, K-pi+gamma
//         Doubly Cabbibo supressed modes are also considered
//   Requirements for non-nominal decay modes:
//         D*+ -> ("K-" "pi+") pi_s+ (+c.c.) charges
//         mKpiMin < m("K" "pi") < mKpiMax
//         m("K" "pi" pi_s) - m("K" "pi") < delta_m_Max
//
// AuthorList:         
//   L K Gladilin (gladilin@mail.cern.ch)  March 2023
// Versions:
// 1.0.0; 2023.04.01 (no jokes) - baseline version
// 1.0.1; 2023.04.14 - allow photons for "m_D0Kpi_only" for a case of Photos
// 1.0.2; 2023.04.17 - skip history Photos lines (status=3)
//

// Header for this module:-

#include "GeneratorFilters/MuDstarFilter.h"

// Framework Related Headers:-
#include "GaudiKernel/MsgStream.h"


// Other classes used by this class:-
#include <math.h>
#include <TMath.h>
#include "CLHEP/Vector/LorentzVector.h"
#include "TLorentzVector.h"

//#include <iostream>

using HepMC::GenVertex;
using HepMC::GenParticle;

//--------------------------------------------------------------------------
MuDstarFilter::MuDstarFilter(const std::string& name, 
      ISvcLocator* pSvcLocator): GenFilter(name,pSvcLocator) {
  //--------------------------------------------------------------------------    
  declareProperty("PtMinMuon",m_PtMinMuon = 2500.);
  declareProperty("PtMaxMuon",m_PtMaxMuon = 1e9);  
  declareProperty("EtaRangeMuon",m_EtaRangeMuon = 2.7);
  //
  declareProperty("PtMinDstar",m_PtMinDstar = 4500.);
  declareProperty("PtMaxDstar",m_PtMaxDstar = 1e9);  
  declareProperty("EtaRangeDstar",m_EtaRangeDstar = 2.7);
  declareProperty("RxyMinDstar",m_RxyMinDstar = -1e9);
  //
  declareProperty("PtMinPis",m_PtMinPis = 450.);
  declareProperty("PtMaxPis",m_PtMaxPis = 1e9);  
  declareProperty("EtaRangePis",m_EtaRangePis = 2.7);
  //
  declareProperty("PtMinKpi",m_PtMinKpi = 900.);
  declareProperty("PtMaxKpi",m_PtMaxKpi = 1e9);  
  declareProperty("EtaRangeKpi",m_EtaRangeKpi = 2.7);
  //
  declareProperty("D0Kpi_only",m_D0Kpi_only = false);
  //
  declareProperty("mKpiMin",m_mKpiMin = 1665.);
  declareProperty("mKpiMax",m_mKpiMax = 2065.);
  //
  declareProperty("delta_m_Max",m_delta_m_Max = 220.);
  //
  declareProperty("DstarMu_m_Max",m_DstarMu_m_Max = 12000.);
}

//--------------------------------------------------------------------------
 MuDstarFilter::~MuDstarFilter(){
//--------------------------------------------------------------------------

}

//---------------------------------------------------------------------------
StatusCode MuDstarFilter::filterInitialize() {
//---------------------------------------------------------------------------

 MsgStream log(messageService(), name());   
 log << MSG:: INFO << "MuDstarFilter v1.02 INITIALISING "  << endreq;   

 log << MSG:: INFO << "PtMinMuon = " << m_PtMinMuon << endreq;
 log << MSG:: INFO << "PtMaxMuon = " << m_PtMaxMuon << endreq;
 log << MSG:: INFO << "EtaRangeMuon = " << m_EtaRangeMuon << endreq;

 log << MSG:: INFO << "PtMinDstar = " << m_PtMinDstar << endreq;
 log << MSG:: INFO << "PtMaxDstar = " << m_PtMaxDstar << endreq;
 log << MSG:: INFO << "EtaRangeDstar = " << m_EtaRangeDstar << endreq;

 log << MSG:: INFO << "RxyMinDstar = " << m_RxyMinDstar << endreq;

 log << MSG:: INFO << "PtMinPis = " << m_PtMinPis << endreq;
 log << MSG:: INFO << "PtMaxPis = " << m_PtMaxPis << endreq;
 log << MSG:: INFO << "EtaRangePis = " << m_EtaRangePis << endreq;

 log << MSG:: INFO << "D0Kpi_only = " << m_D0Kpi_only << endreq;

 log << MSG:: INFO << "PtMinKpi = " << m_PtMinKpi << endreq;
 log << MSG:: INFO << "PtMaxKpi = " << m_PtMaxKpi << endreq;
 log << MSG:: INFO << "EtaRangeKpi = " << m_EtaRangeKpi << endreq;

 log << MSG:: INFO << "mKpiMin = " << m_mKpiMin << endreq;
 log << MSG:: INFO << "mKpiMax = " << m_mKpiMax << endreq;

 log << MSG:: INFO << "delta_m_Max = " << m_delta_m_Max << endreq;

 log << MSG:: INFO << "DstarMu_m_Max = " << m_DstarMu_m_Max << endreq;

 return StatusCode::SUCCESS;}

//---------------------------------------------------------------------------
StatusCode MuDstarFilter::filterFinalize() {
//---------------------------------------------------------------------------
 return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
StatusCode MuDstarFilter::filterEvent() {
//---------------------------------------------------------------------------

// Loop over all events in McEventCollection 
  MsgStream log(messageService(), name());
  McEventCollection::const_iterator itr;
  
  log << MSG:: DEBUG << " MuDstarFilter filtering "  << endreq;

  for (itr = events_const()->begin(); itr!=events_const()->end(); ++itr) {

    double primx = 0.;
    double primy = 0.;

    HepMC::GenVertex* vprim;

    vprim = (*((*itr)->vertices_begin()));

    primx = vprim->position().x();
    primy = vprim->position().y();
    
    log << MSG::DEBUG << "MuDstarFilter: PV x, y = " << primx << " , " << primy << endreq;

    int NumMuons=0;
    int NumDstars=0;

    std::vector<HepMC::GenParticle*> Muons;

    // Loop over all particles in the event
    const HepMC::GenEvent* genEvt = (*itr);
    for(HepMC::GenEvent::particle_const_iterator pitr=genEvt->particles_begin();
	pitr!=genEvt->particles_end(); ++pitr ){

      if ( (*pitr)->status() == 3 ) continue; // photos history line
      
      // muons
      if (std::abs((*pitr)->pdg_id()) == 13) {
        if( ((*pitr)->momentum().perp() >= m_PtMinMuon)
         && ((*pitr)->momentum().perp() <  m_PtMaxMuon)
	    && (std::abs((*pitr)->momentum().eta()) < m_EtaRangeMuon) ) {
  	  ++NumMuons;
          Muons.push_back((*pitr));
        }
      }
    }
    
    log << MSG::DEBUG << "MuDstarFilter: NumMuons = " << NumMuons << endreq;
  
    if ( NumMuons == 0 ) break;
  
    for(HepMC::GenEvent::particle_const_iterator pitr=genEvt->particles_begin();
      pitr!=genEvt->particles_end(); ++pitr ){

      if ( (*pitr)->status() == 3 ) continue; // photos history line

      // Dstars
      if (std::abs((*pitr)->pdg_id()) == 413) {
        if( ((*pitr)->momentum().perp() >= m_PtMinDstar)
         && ((*pitr)->momentum().perp() <  m_PtMaxDstar)
	    && (std::abs((*pitr)->momentum().eta()) < m_EtaRangeDstar) ) {

	 //Check if has end_vertex
	 if(!((*pitr)->end_vertex())) continue; 

	 double Rxy = sqrt(pow((*pitr)->end_vertex()->position().x()-primx,2)+pow((*pitr)->end_vertex()->position().y()-primy,2));
    
         log << MSG::DEBUG << "MuDstarFilter: R_xy(Dstar) = " << Rxy << endreq;

	 if ( Rxy < m_RxyMinDstar ) continue;
	 
	 // Child
	 HepMC::GenVertex::particle_iterator firstChild =
	   (*pitr)->end_vertex()->particles_begin(HepMC::children);
	 HepMC::GenVertex::particle_iterator endChild =
	   (*pitr)->end_vertex()->particles_end(HepMC::children);
	 HepMC::GenVertex::particle_iterator thisChild = firstChild;

	 if( (*firstChild)->pdg_id() == (*pitr)->pdg_id() ) continue; 

	 TLorentzVector p4_K;
	 TLorentzVector p4_pi;
	 TLorentzVector p4_pis;
	 TLorentzVector p4_D0;
	 TLorentzVector p4_Dstar;
	 TLorentzVector p4_Mu;
	 TLorentzVector p4_DstarMu;

	 int pis_pdg = 0;
	 int K_pdg = 0;
	 
         int NumD0=0;
         int NumPis=0;
	
  	 for(; thisChild != endChild; ++thisChild){

           if ( (*thisChild)->status() == 3 ) continue; // photos history line

	   if( std::abs((*thisChild)->pdg_id()) == 211 ) {
             if( ((*thisChild)->momentum().perp() >= m_PtMinPis)
              && ((*thisChild)->momentum().perp() <  m_PtMaxPis)
		 && (std::abs((*thisChild)->momentum().eta()) < m_EtaRangePis) ) {
  	      ++NumPis;
              p4_pis.SetPtEtaPhiM((*thisChild)->momentum().perp(),(*thisChild)->momentum().eta(),(*thisChild)->momentum().phi(),PionMass);
	      pis_pdg = (*thisChild)->pdg_id();
	     }
	   }
	 }// thisChild
    
         log << MSG::DEBUG << "MuDstarFilter: NumPis = " << NumPis << endreq;

	 if ( NumPis == 0 ) continue;

         HepMC::GenParticle* D0Child1=0;
         HepMC::GenParticle* D0Child2=0;
         HepMC::GenParticle* D0ChildMu=0;

	 int NumChildD0=0;
	 int NumChildD0Charged=0;
	 int NumChildD0neutrinos=0;
	 int NumChildD0gammas=0;
	 int ChargeD0Child1=0;
	 int ChargeD0Child2=0;
	 int NumChildD0K=0;
	 int NumChildD0pi=0;
	 int NumChildD0mu=0;
	
  	 for(thisChild = firstChild; thisChild != endChild; ++thisChild){

           if ( (*thisChild)->status() == 3 ) continue; // photos history line

	   if( std::abs((*thisChild)->pdg_id()) == 421 ) {
             if (((*thisChild)->end_vertex())) {
	     //
 	       HepMC::GenVertex::particle_iterator firstChild1 =
	       (*thisChild)->end_vertex()->particles_begin(HepMC::children);
	       HepMC::GenVertex::particle_iterator endChild1 =
	       (*thisChild)->end_vertex()->particles_end(HepMC::children);
	       HepMC::GenVertex::particle_iterator thisChild1 = firstChild1;

	       for(; thisChild1 != endChild1; ++thisChild1){

                 if ( (*thisChild1)->status() == 3 ) continue; // photos history line

		 if ( std::abs((*thisChild1)->pdg_id()) ==  11 || std::abs((*thisChild1)->pdg_id()) ==  13 ||
		      std::abs((*thisChild1)->pdg_id()) == 211 || std::abs((*thisChild1)->pdg_id()) == 321 ) {

	           ++NumChildD0;
		   
	           if( ((*thisChild1)->momentum().perp() >= m_PtMinKpi)
                    && ((*thisChild1)->momentum().perp() <  m_PtMaxKpi)
		    && (std::abs((*thisChild1)->momentum().eta()) < m_EtaRangeKpi) ) {
  	             ++NumChildD0Charged;

  	             if ( NumChildD0Charged == 1 ) {
		       D0Child1 = (*thisChild1);
		       if ( (*thisChild1)->pdg_id() == -11 || (*thisChild1)->pdg_id() == -13 ||
		            (*thisChild1)->pdg_id() == 211 || (*thisChild1)->pdg_id() == 321 ) {
	                 ChargeD0Child1=+1;
		       } else {
	                 ChargeD0Child1=-1;
		       }
		     }
		     if ( NumChildD0Charged == 2 ) {
		       D0Child2 = (*thisChild1);
		       if ( (*thisChild1)->pdg_id() == -11 || (*thisChild1)->pdg_id() == -13 ||
		            (*thisChild1)->pdg_id() == 211 || (*thisChild1)->pdg_id() == 321 ) {
	                 ChargeD0Child2=+1;
		       } else {
	                 ChargeD0Child2=-1;
		       }
		     }
  	             if ( std::abs((*thisChild1)->pdg_id()) == 13 ) {
		       ++NumChildD0mu;
		       D0ChildMu = (*thisChild1);
		     }
  	             if ( std::abs((*thisChild1)->pdg_id()) == 211 )  ++NumChildD0pi;
  	             if ( std::abs((*thisChild1)->pdg_id()) == 321 ) {
		       ++NumChildD0K;
		       K_pdg = (*thisChild1)->pdg_id();
		     }
		   }

		 } else if ( std::abs((*thisChild1)->pdg_id()) ==  111 ) {
		    ++NumChildD0;
		 } else if ( std::abs((*thisChild1)->pdg_id()) ==  12 || std::abs((*thisChild1)->pdg_id()) ==  14 ) {
		    ++NumChildD0neutrinos;
		 } else if ( std::abs((*thisChild1)->pdg_id()) ==  22 ) {
		    ++NumChildD0gammas;
		 } else if ( std::abs((*thisChild1)->pdg_id()) ==  311 || std::abs((*thisChild1)->pdg_id()) ==  130 || std::abs((*thisChild1)->pdg_id()) ==  310 ) {
		   ++NumChildD0;
		   ++NumChildD0;
                 } else if (((*thisChild1)->end_vertex())) {

		   //
   	           HepMC::GenVertex::particle_iterator firstChild2 =
	           (*thisChild1)->end_vertex()->particles_begin(HepMC::children);
	           HepMC::GenVertex::particle_iterator endChild2 =
	           (*thisChild1)->end_vertex()->particles_end(HepMC::children);
	           HepMC::GenVertex::particle_iterator thisChild2 = firstChild2;

	           for(; thisChild2 != endChild2; ++thisChild2){

                     if ( (*thisChild2)->status() == 3 ) continue; // photos history line

		     if ( std::abs((*thisChild2)->pdg_id()) ==  11 || std::abs((*thisChild2)->pdg_id()) ==  13 ||
		          std::abs((*thisChild2)->pdg_id()) == 211 || std::abs((*thisChild2)->pdg_id()) == 321 ) {

 	               ++NumChildD0;
		   
	               if( ((*thisChild2)->momentum().perp() >= m_PtMinKpi)
                        && ((*thisChild2)->momentum().perp() <  m_PtMaxKpi)
			&& (std::abs((*thisChild2)->momentum().eta()) < m_EtaRangeKpi) ) {
  	                 ++NumChildD0Charged;

  	                 if ( NumChildD0Charged == 1 ) {
		           D0Child1 = (*thisChild2);
		           if ( (*thisChild2)->pdg_id() == -11 || (*thisChild2)->pdg_id() == -13 ||
		                (*thisChild2)->pdg_id() == 211 || (*thisChild2)->pdg_id() == 321 ) {
	                     ChargeD0Child1=+1;
		           } else {
	                     ChargeD0Child1=-1;
		           }
		         }
		         if ( NumChildD0Charged == 2 ) {
		           D0Child2 = (*thisChild2);
		           if ( (*thisChild2)->pdg_id() == -11 || (*thisChild2)->pdg_id() == -13 ||
		                (*thisChild2)->pdg_id() == 211 || (*thisChild2)->pdg_id() == 321 ) {
	                     ChargeD0Child2=+1;
		           } else {
	                     ChargeD0Child2=-1;
		           }
		         }
  	                 if ( std::abs((*thisChild2)->pdg_id()) == 13 ) {
		           ++NumChildD0mu;
			   D0ChildMu = (*thisChild2);
			 }
		       }

		     } else if ( std::abs((*thisChild2)->pdg_id()) ==  111 ) {
		       ++NumChildD0;
		     } else if ( std::abs((*thisChild2)->pdg_id()) ==  12 || std::abs((*thisChild2)->pdg_id()) ==  14 ) {
		        ++NumChildD0neutrinos;
		     } else if ( std::abs((*thisChild2)->pdg_id()) ==  22 ) {
		        ++NumChildD0gammas;
		     } else if ( std::abs((*thisChild2)->pdg_id()) ==  311 || std::abs((*thisChild2)->pdg_id()) ==  130 || std::abs((*thisChild2)->pdg_id()) ==  310 ) {
		       ++NumChildD0;
		       ++NumChildD0;
	             } else if (((*thisChild2)->end_vertex())) {
 	               ++NumChildD0;
 	               ++NumChildD0;
		     } else {
		       ++NumChildD0;
		       ++NumChildD0;
                       log << MSG::DEBUG << "MuDstarFilter: unexpected D0 granddaughter = " << (*thisChild2)->pdg_id() << endreq;
		     }
		   }// thisChild2
		   
		 } else {
		   ++NumChildD0;
		   ++NumChildD0;
                   log << MSG::DEBUG << "MuDstarFilter: unexpected D0 daughter = " << (*thisChild1)->pdg_id() << endreq;
		 }
	       }// thisChild1
    
               log << MSG::DEBUG << "MuDstarFilter: NumChildD0, NumChildD0Charged = " << NumChildD0 << " , " << NumChildD0Charged << endreq;
	       
	       if ( NumChildD0 <= 3 && NumChildD0Charged == 2 && ChargeD0Child1*ChargeD0Child2 < 0) {
		 if ( m_D0Kpi_only ) {
		   if ( NumChildD0 == 2 && NumChildD0K == 1 && NumChildD0pi == 1 ) ++NumD0;
		 } else {
		   ++NumD0;
		 }
	       }	       
	     }// enVertex
	   }// 421
    
           log << MSG::DEBUG << "MuDstarFilter: NumD0 = " << NumD0 << endreq;

	   if ( NumD0 == 1 ) {
	     
	     if ( pis_pdg * ChargeD0Child1 < 0 ) {

               p4_K.SetPtEtaPhiM(D0Child1->momentum().perp(),D0Child1->momentum().eta(),D0Child1->momentum().phi(),KaonMass);
               p4_pi.SetPtEtaPhiM(D0Child2->momentum().perp(),D0Child2->momentum().eta(),D0Child2->momentum().phi(),PionMass);
	
	     } else {

               p4_K.SetPtEtaPhiM(D0Child2->momentum().perp(),D0Child2->momentum().eta(),D0Child2->momentum().phi(),KaonMass);
               p4_pi.SetPtEtaPhiM(D0Child1->momentum().perp(),D0Child1->momentum().eta(),D0Child1->momentum().phi(),PionMass);		 
	     }
	     
	     p4_D0 = p4_K + p4_pi;
	     double mKpi = p4_D0.M();
    
             log << MSG::DEBUG << "MuDstarFilter: mKpi = " << mKpi << endreq;
	     
	     if ( mKpi >= m_mKpiMin && mKpi <= m_mKpiMax ) {

	       p4_Dstar = p4_D0 + p4_pis;

	       double delta_m = p4_Dstar.M()-mKpi;
    
               log << MSG::DEBUG << "MuDstarFilter: delta_m = " << delta_m << endreq;

	       if ( delta_m <= m_delta_m_Max ) {
 	         ++NumDstars;
    
                 log << MSG::DEBUG << "MuDstarFilter: NumDstars = " << NumDstars << endreq;

	  	 for(size_t i=0; i < Muons.size(); ++i){

		   if ( NumChildD0mu == 1 ) {
                     log << MSG::DEBUG << "MuDstarFilter: Mu(pT), D0Mu(pT) = " << Muons[i]->momentum().perp() << " , " << D0ChildMu->momentum().perp() << endreq;
		     if ( Muons[i]->momentum().perp() == D0ChildMu->momentum().perp() ) continue;
                     log << MSG::DEBUG << "MuDstarFilter: Mu(pT), D0Mu(pT) = " << Muons[i]->momentum().perp() << " , " << D0ChildMu->momentum().perp() << endreq ;
		   }
		   
		   p4_Mu.SetPtEtaPhiM(Muons[i]->momentum().perp(),Muons[i]->momentum().eta(),Muons[i]->momentum().phi(),MuonMass);

		   p4_DstarMu = p4_Dstar + p4_Mu;
    
                   log << MSG::DEBUG << "MuDstarFilter: p4_DstarMu.M() = " << p4_DstarMu.M() << endreq;

		   if ( p4_DstarMu.M() <= m_DstarMu_m_Max ) {
    
		     log << MSG::INFO << "MuDstarFilter: MuDstar candidate found" << endreq;
		     log << MSG::INFO << "MuDstarFilter: p4_DstarMu.M() = " << p4_DstarMu.M() << endreq;
		     log << MSG::INFO << "MuDstarFilter: NumChildD0, NumChildD0Charged = " << NumChildD0 << " , " << NumChildD0Charged << endreq;
		     log << MSG::INFO << "MuDstarFilter: NumChildD0K, NumChildD0pi, NumChildD0mu = " << NumChildD0K << " , " << NumChildD0pi << " , " << NumChildD0mu << endreq;

		     if ( NumChildD0mu == 1 ) {

		       log << MSG::DEBUG << "MuDstarFilter: Mu(pT), D0Mu(pT) = " << Muons[i]->momentum().perp() << " , " << D0ChildMu->momentum().perp() << endreq;
		     }

		     log << MSG::INFO << "MuDstarFilter: NumChildD0neutrinos, NumChildD0gammas = " << NumChildD0neutrinos << " , " << NumChildD0gammas << endreq;
		     log << MSG::INFO << "MuDstarFilter: pis_pdg, K_pdg, ChargeD0Child1, ChargeD0Child2 = " << pis_pdg << " , " << K_pdg << " , " << ChargeD0Child1 << " , " << ChargeD0Child2 << endreq;

		     setFilterPassed(true);
                     return StatusCode::SUCCESS;
		   }
		 }// for i
		 
	       }//delta_m		 
	     }// mKpi
	   }// NumD0	     
         }// thisChild
        }// PtMinDstar
      }// 413
    }// pitr
  }// itr

  //
  // if we get here we have failed
  //
  setFilterPassed(false);
  return StatusCode::SUCCESS;
}
