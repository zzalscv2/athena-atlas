/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * Created by Paula Martinez based on V Sorin and Joerg Stelzer.
 *
 * @brief algorithm calculates the sqr of the INVMASS and DeltaR between two lists and applies invmass and deltaR criteria
 * Events containing a pair of TGC muons with same charge are rejected
 *
 * @param NumberLeading
 *
 * For questions contact atlas-trig-l1topo-algcom@cern.ch.
**********************************/


#include "L1TopoAlgorithms/InvariantMassInclusiveDeltaRSqrIncl2Charge.h"

#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"

#include <cmath>

REGISTER_ALG_TCS(InvariantMassInclusiveDeltaRSqrIncl2Charge)


TCS::InvariantMassInclusiveDeltaRSqrIncl2Charge::InvariantMassInclusiveDeltaRSqrIncl2Charge(const std::string & name) : DecisionAlg(name)
{
   defineParameter("InputWidth1", 9);
   defineParameter("InputWidth2", 9);
   defineParameter("MaxTob1", 0);
   defineParameter("MaxTob2", 0);
   defineParameter("NumResultBits", 6);
   defineParameter("MinMSqr",   0, 0);
   defineParameter("MaxMSqr", 999, 0);
   defineParameter("MinMSqr",   0, 1);
   defineParameter("MaxMSqr", 999, 1);
   defineParameter("MinMSqr",   0, 2);
   defineParameter("MaxMSqr", 999, 2);
   defineParameter("MinMSqr",   0, 3);
   defineParameter("MaxMSqr", 999, 3);
   defineParameter("MinMSqr",   0, 4);
   defineParameter("MaxMSqr", 999, 4);
   defineParameter("MinMSqr",   0, 5);
   defineParameter("MaxMSqr", 999, 5);
   defineParameter("MinET1",0,0);
   defineParameter("MinET2",0,0);
   defineParameter("MinET1",0,1);
   defineParameter("MinET2",0,1);
   defineParameter("MinET1",0,2);
   defineParameter("MinET2",0,2);
   defineParameter("MinET1",0,3);
   defineParameter("MinET2",0,3);
   defineParameter("MinET1",0,4);
   defineParameter("MinET2",0,4);
   defineParameter("MinET1",0,5);
   defineParameter("MinET2",0,5);
   defineParameter("ApplyEtaCut", 0);
   defineParameter("MinEta1",  0);
   defineParameter("MaxEta1", 31);
   defineParameter("MinEta2",  0);
   defineParameter("MaxEta2", 31);
   defineParameter("DeltaRMin",  0, 0);
   defineParameter("DeltaRMax",  0, 0);
   defineParameter("DeltaRMin",  0, 1);
   defineParameter("DeltaRMax",  0, 1);
   defineParameter("DeltaRMin",  0, 2);
   defineParameter("DeltaRMax",  0, 2);
   defineParameter("DeltaRMin",  0, 3);
   defineParameter("DeltaRMax",  0, 3);
   defineParameter("DeltaRMin",  0, 4);
   defineParameter("DeltaRMax",  0, 4);
   defineParameter("DeltaRMin",  0, 5);
   defineParameter("DeltaRMax",  0, 5);

   setNumberOutputBits(6);
}

TCS::InvariantMassInclusiveDeltaRSqrIncl2Charge::~InvariantMassInclusiveDeltaRSqrIncl2Charge(){}


TCS::StatusCode
TCS::InvariantMassInclusiveDeltaRSqrIncl2Charge::initialize() {

   m_NumberLeading1 = parameter("InputWidth1").value();
   m_NumberLeading2 = parameter("InputWidth2").value();
   if(parameter("MaxTob1").value() > 0) m_NumberLeading1 = parameter("MaxTob1").value();
   if(parameter("MaxTob2").value() > 0) m_NumberLeading2 = parameter("MaxTob2").value();
   TRG_MSG_INFO("NumberLeading1 : " << m_NumberLeading1);
   TRG_MSG_INFO("NumberLeading2 : " << m_NumberLeading2);

   for(unsigned int i=0; i<numberOutputBits(); ++i) {

     m_InvMassMin[i] = parameter("MinMSqr", i).value();
     m_InvMassMax[i] = parameter("MaxMSqr", i).value();
     m_MinET1[i] = parameter("MinET1",i).value();
     m_MinET2[i] = parameter("MinET2",i).value();
     m_DeltaRMin[i] = parameter("DeltaRMin", i).value();
     m_DeltaRMax[i] = parameter("DeltaRMax", i).value();
     
     TRG_MSG_INFO("InvMassMin "<< i << "  : " << m_InvMassMin[i]);
     TRG_MSG_INFO("InvMassMax "<< i << "  : " << m_InvMassMax[i]);
     TRG_MSG_INFO("MinET1     "<< i << "  : " << m_MinET1[i]);
     TRG_MSG_INFO("MinET2     "<< i << "  : " << m_MinET2[i]);
     TRG_MSG_INFO("DeltaRMin  "<< i << "  : " << m_DeltaRMin[i]);
     TRG_MSG_INFO("DeltaRMax  "<< i << "  : " << m_DeltaRMax[i]);

   }

   m_ApplyEtaCut = parameter("ApplyEtaCut").value();
   m_MinEta1     = parameter("MinEta1"    ).value();
   m_MaxEta1     = parameter("MaxEta1"    ).value();
   m_MinEta2     = parameter("MinEta2"    ).value();
   m_MaxEta2     = parameter("MaxEta2"    ).value();
   TRG_MSG_INFO("ApplyEtaCut : "<<m_ApplyEtaCut );
   TRG_MSG_INFO("MinEta1     : "<<m_MinEta1     );
   TRG_MSG_INFO("MaxEta1     : "<<m_MaxEta1     );
   TRG_MSG_INFO("MinEta2     : "<<m_MinEta2     );
   TRG_MSG_INFO("MaxEta2     : "<<m_MaxEta2     );

   TRG_MSG_INFO("number output : " << numberOutputBits());

   // book histograms
   for(unsigned int i=0; i<numberOutputBits(); ++i) {
       std::string hname_accept = "hInvariantMassInclusiveDeltaRSqrIncl2Charge_accept_bit"+std::to_string(static_cast<int>(i));
       std::string hname_reject = "hInvariantMassInclusiveDeltaRSqrIncl2Charge_reject_bit"+std::to_string(static_cast<int>(i));
       // mass
       bookHist(m_histAcceptM, hname_accept, "INVM vs DR", 100, std::sqrt(m_InvMassMin[i]), std::sqrt(m_InvMassMax[i]), 100, std::sqrt(m_DeltaRMin[i]), std::sqrt(m_DeltaRMax[i]));
       bookHist(m_histRejectM, hname_reject, "INVM vs DR", 100, std::sqrt(m_InvMassMin[i]), std::sqrt(m_InvMassMax[i]), 100, std::sqrt(m_DeltaRMin[i]), std::sqrt(m_DeltaRMax[i]));
       // eta2 vs. eta1
       bookHist(m_histAcceptEta1Eta2, hname_accept, "ETA vs ETA", 100, -70, 70, 100, -70, 70);
       bookHist(m_histRejectEta1Eta2, hname_reject, "ETA vs ETA", 100, -70, 70, 100, -70, 70);
   }

   return StatusCode::SUCCESS;
}



TCS::StatusCode
TCS::InvariantMassInclusiveDeltaRSqrIncl2Charge::processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{

   if( input.size() == 2) {
      for( TOBArray::const_iterator tob1 = input[0]->begin();
           tob1 != input[0]->end() && distance(input[0]->begin(), tob1) < m_NumberLeading1;
           ++tob1)
         {


            for( TCS::TOBArray::const_iterator tob2 = input[1]->begin();
                 tob2 != input[1]->end() && distance(input[1]->begin(), tob2) < m_NumberLeading2;
                 ++tob2) {
                // Inv Mass calculation
                unsigned int invmass2 = calcInvMassBW( *tob1, *tob2 );
		// test DeltaR2Min, DeltaR2Max                                                                                                
		unsigned int deltaR2 = calcDeltaR2BW( *tob1, *tob2 );
		TRG_MSG_DEBUG("Jet1 = " << **tob1 << ", Jet2 = " << **tob2 << ", invmass2 = " << invmass2 << ", deltaR2 = " << deltaR2);
                const int eta1 = (*tob1)->eta();
                const int eta2 = (*tob2)->eta();
                const unsigned int aeta1 = std::abs(eta1);
                const unsigned int aeta2 = std::abs(eta2);
                // Charge cut ( 1 = positive, -1 = negative, 0 = undefined (RPC) )
                int charge1 = (*tob1)->charge();
                int charge2 = (*tob2)->charge();
                int totalCharge = charge1 + charge2;
                bool acceptCharge = true;
                if ( std::abs(totalCharge) == 2 ) { acceptCharge = false; }
		for(unsigned int i=0; i<numberOutputBits(); ++i) {
                   bool accept = false;
                   if( parType_t((*tob1)->Et()) <= m_MinET1[i]) continue; // ET cut
                   if( parType_t((*tob2)->Et()) <= m_MinET2[i]) continue; // ET cut
                   if(m_ApplyEtaCut &&
                      ((aeta1 < m_MinEta1 || aeta1 > m_MaxEta1 ) ||
                       (aeta2 < m_MinEta2 || aeta2 > m_MaxEta2 ) ))  continue;
                   accept = invmass2 >= m_InvMassMin[i] && invmass2 <= m_InvMassMax[i] && 
                            deltaR2 >= m_DeltaRMin[i] && deltaR2 <= m_DeltaRMax[i] && acceptCharge;
                   const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
                   const bool fillReject = fillHistos() and not fillAccept;
                   const bool alreadyFilled = decision.bit(i);
                   if( accept ) {
                       decision.setBit(i, true);
                       output[i]->push_back( TCS::CompositeTOB(*tob1, *tob2) );
                   }
                   if(fillAccept and not alreadyFilled) {
		     fillHist2D(m_histAcceptM[i],std::sqrt(static_cast<float>(invmass2)),std::sqrt(static_cast<float>(deltaR2)));
                     fillHist2D(m_histAcceptEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                   } else if(fillReject) {
		     fillHist2D(m_histRejectM[i],std::sqrt(static_cast<float>(invmass2)),std::sqrt(static_cast<float>(deltaR2)));
                     fillHist2D(m_histRejectEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                   }
                   TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail") << " invmass2 = " << invmass2 << " deltaR2 = " << deltaR2 );
               }
            }
         }
   } else {

      TCS_EXCEPTION("InvariantMassInclusiveDeltaRSqrIncl2Charge alg must have  2 inputs, but got " << input.size());

   }
   return TCS::StatusCode::SUCCESS;

}

TCS::StatusCode
TCS::InvariantMassInclusiveDeltaRSqrIncl2Charge::process( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{


   if( input.size() == 2) {
      for( TOBArray::const_iterator tob1 = input[0]->begin();
           tob1 != input[0]->end() && distance(input[0]->begin(), tob1) < m_NumberLeading1;
           ++tob1)
         {
            for( TCS::TOBArray::const_iterator tob2 = input[1]->begin();
                 tob2 != input[1]->end() && distance(input[1]->begin(), tob2) < m_NumberLeading2;
                 ++tob2) {

                 // Inv Mass calculation
                 unsigned int invmass2 = calcInvMass( *tob1, *tob2 );
                 // test DeltaR2Min, DeltaR2Max                                                                                                  
	         unsigned int deltaR2 = calcDeltaR2( *tob1, *tob2 );
                 TRG_MSG_DEBUG("Jet1 = " << **tob1 << ", Jet2 = " << **tob2 << ", invmass2 = " << invmass2 << ", deltaR2 = " << deltaR2);
                 const int eta1 = (*tob1)->eta();
                 const int eta2 = (*tob2)->eta();
                 const unsigned int aeta1 = std::abs(eta1);
                 const unsigned int aeta2 = std::abs(eta2);
                 // Charge cut ( 1 = positive, -1 = negative, 0 = undefined (RPC) )
                 int charge1 = (*tob1)->charge();
                 int charge2 = (*tob2)->charge();
                 int totalCharge = charge1 + charge2;
                 bool acceptCharge = true;
                 if ( std::abs(totalCharge) == 2 ) { acceptCharge = false; }
                 for(unsigned int i=0; i<numberOutputBits(); ++i) {
                     if( parType_t((*tob1)->Et()) <= m_MinET1[i]) continue; // ET cut
                     if( parType_t((*tob2)->Et()) <= m_MinET2[i]) continue; // ET cut
                     if(m_ApplyEtaCut &&
                        ((aeta1 < m_MinEta1 || aeta1 > m_MaxEta1 ) ||
                         (aeta2 < m_MinEta2 || aeta2 > m_MaxEta2 ) )) continue;
                     bool accept = invmass2 >= m_InvMassMin[i] && invmass2 <= m_InvMassMax[i] && 
                                   deltaR2 >= m_DeltaRMin[i] && deltaR2 <= m_DeltaRMax[i] && acceptCharge;
                     const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
                     const bool fillReject = fillHistos() and not fillAccept;
                     const bool alreadyFilled = decision.bit(i);
                     if( accept ) {
                         decision.setBit(i, true);
                         output[i]->push_back( TCS::CompositeTOB(*tob1, *tob2) );
                     }
                     if(fillAccept and not alreadyFilled) {
                       fillHist2D(m_histAcceptM[i],std::sqrt(static_cast<float>(invmass2)),std::sqrt(static_cast<float>(deltaR2)));
                       fillHist2D(m_histAcceptEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                     } else if(fillReject) {
                       fillHist2D(m_histRejectM[i],std::sqrt(static_cast<float>(invmass2)),std::sqrt(static_cast<float>(deltaR2)));
                       fillHist2D(m_histRejectEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                     }
                     TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail") << " invmass2 = " << invmass2 << " deltaR2 = " << deltaR2 );
               }
            }
         }
   } else {
      TCS_EXCEPTION("InvariantMassInclusiveDeltaRSqrIncl2Charge alg must have  2 inputs, but got " << input.size());
   }
   return TCS::StatusCode::SUCCESS;
}
