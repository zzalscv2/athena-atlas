/*********************************
 * DisambiguationDRIncl3.cpp
 * Created by Joerg Stelzer / V Sorin on 9/16/14.
 * Copyright (c) 2012 Joerg Stelzer. All rights reserved.
 * 
 * @brief algorithm calculates the dR distance between objects in three lists, check EM vs TAU , if those not match
 * check agains 3rd object, passed if no match with 3rd
 *
 * @param NumberLeading
**********************************/

#include <cmath>

#include "L1TopoAlgorithms/DisambiguationDRIncl3.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"

REGISTER_ALG_TCS(DisambiguationDRIncl3)

using namespace std;

// not the best solution but we will move to athena where this comes for free
#define LOG cout << "TCS::DisambiguationDRIncl3:     "




namespace {
   unsigned int
   calcDeltaR2(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
      double deta = ( tob1->etaDouble() - tob2->etaDouble() );
      double dphi = fabs( tob1->phiDouble() - tob2->phiDouble() );
      if(dphi>M_PI)
         dphi = 2*M_PI - dphi;

      return round ( 100 * ((dphi)*(dphi) + (deta)*(deta) )) ;

   }
}


TCS::DisambiguationDRIncl3::DisambiguationDRIncl3(const std::string & name) : DecisionAlg(name)
{
   defineParameter("InputWidth1", 9);
   defineParameter("InputWidth2", 9);
   defineParameter("InputWidth3", 9);
   defineParameter("MaxTob1", 0); 
   defineParameter("MaxTob2", 0); 
   defineParameter("MaxTob3", 0);
   defineParameter("NumResultBits", 2);
   defineParameter("MinET1",1);
   defineParameter("MinET2",1);
   defineParameter("MinET3",1);
   defineParameter("EtaMin1",0);
   defineParameter("EtaMax1",49);
   defineParameter("EtaMin2",0);
   defineParameter("EtaMax2",49);
   defineParameter("EtaMin3",0);
   defineParameter("EtaMax3",49);
   defineParameter("DRCutMin",0);
   defineParameter("DRCutMax",0);
   defineParameter("DisambDR",0,0);
   defineParameter("DisambDR",0,1); 
   setNumberOutputBits(2);
}

TCS::DisambiguationDRIncl3::~DisambiguationDRIncl3(){}


TCS::StatusCode
TCS::DisambiguationDRIncl3::initialize() {
   p_NumberLeading1 = parameter("InputWidth1").value();
   p_NumberLeading2 = parameter("InputWidth2").value();
   p_NumberLeading3 = parameter("InputWidth3").value();

   if(parameter("MaxTob1").value() > 0) p_NumberLeading1 = parameter("MaxTob1").value();
   if(parameter("MaxTob2").value() > 0) p_NumberLeading2 = parameter("MaxTob2").value();
   if(parameter("MaxTob3").value() > 0) p_NumberLeading3 = parameter("MaxTob3").value();


   p_MinET1 = parameter("MinET1").value();
   p_MinET2 = parameter("MinET2").value();
   p_MinET3 = parameter("MinET3").value();
   p_EtaMin1 = parameter("EtaMin1").value();
   p_EtaMax1 = parameter("EtaMax1").value();
   p_EtaMin2 = parameter("EtaMin2").value();
   p_EtaMax2 = parameter("EtaMax2").value();
   p_EtaMin3 = parameter("EtaMin3").value();
   p_EtaMax3 = parameter("EtaMax3").value();
   p_DRCutMin = parameter("DRCutMin").value();
   p_DRCutMax = parameter("DRCutMax").value();

   for(int i=0; i<2; ++i) {
      p_DisambDR[i] = parameter("DisambDR", i).value();
   }



   TRG_MSG_INFO("MinET1          : " << p_MinET1);
   TRG_MSG_INFO("EtaMin1         : " << p_EtaMin1);
   TRG_MSG_INFO("EtaMax1         : " << p_EtaMax1);
   TRG_MSG_INFO("MinET2          : " << p_MinET2);
   TRG_MSG_INFO("EtaMin2         : " << p_EtaMin2);
   TRG_MSG_INFO("EtaMax2         : " << p_EtaMax2);
   TRG_MSG_INFO("MinET3          : " << p_MinET3);
   TRG_MSG_INFO("EtaMin3         : " << p_EtaMin3);
   TRG_MSG_INFO("EtaMax3         : " << p_EtaMax3);
   TRG_MSG_INFO("DisambDR0         : " << p_DisambDR[0]);
   TRG_MSG_INFO("DisambDR1         : " << p_DisambDR[1]);
   TRG_MSG_INFO("DRCutMin           : " << p_DRCutMin);
   TRG_MSG_INFO("DRCutMax           : " << p_DRCutMax);
   TRG_MSG_INFO("number output : " << numberOutputBits());


   return StatusCode::SUCCESS;
}



TCS::StatusCode
TCS::DisambiguationDRIncl3::process( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{

      
   if( input.size() == 3) {

      
      for( TOBArray::const_iterator tob1 = input[0]->begin(); 
           tob1 != input[0]->end() && distance(input[0]->begin(), tob1) < p_NumberLeading1;
           ++tob1)
         {

            if( parType_t((*tob1)->Et()) <= p_MinET1) continue; // ET cut
            if( parType_t(fabs((*tob1)->eta())) > p_EtaMax1 ) continue; // Eta cut
            if( parType_t(fabs((*tob1)->eta())) < p_EtaMin1 ) continue; // Eta cut

            for( TCS::TOBArray::const_iterator tob2 = input[1]->begin(); 
                 tob2 != input[1]->end() && distance(input[1]->begin(), tob2) < p_NumberLeading2;
                 ++tob2) {

               if( parType_t((*tob2)->Et()) <= p_MinET2) continue; // ET cut
               if( parType_t(fabs((*tob2)->eta())) > p_EtaMax2 ) continue; // Eta cut
               if( parType_t(fabs((*tob2)->eta())) < p_EtaMin2 ) continue; // Eta cut

               // test DeltaR2Min, DeltaR2Max
               unsigned int deltaR2Cut = calcDeltaR2( *tob1, *tob2 );
               if (deltaR2Cut > p_DRCutMax) continue;
               if (deltaR2Cut <= p_DRCutMin) continue;
               
	       for( TCS::TOBArray::const_iterator tob3 = input[2]->begin();
                 tob3 != input[2]->end() ;
                 ++tob3) {

                    if( parType_t((*tob3)->Et()) <= p_MinET3) continue; // ET cut
                    if( parType_t(fabs((*tob3)->eta())) > p_EtaMax3 ) continue; // Eta cut
                    if( parType_t(fabs((*tob3)->eta())) < p_EtaMin3 ) continue; // Eta cut
               
		    unsigned int deltaR13 = calcDeltaR2( *tob1, *tob3 );
		    unsigned int deltaR23 = calcDeltaR2( *tob2, *tob3 );

                    bool accept[3];
                    for(unsigned int i=0; i<numberOutputBits(); ++i) {
                       accept[i] = deltaR13 > p_DisambDR[i] && deltaR23 > p_DisambDR[i] ;
                       if( accept[i] ) {
                         decision.setBit(i, true);
                         output[i]->push_back(TCS::CompositeTOB(*tob1, *tob2));
                       }
	               TRG_MSG_DEBUG("Decision " << i << ": " << (accept[i]?"pass":"fail") << " deltaR13 = " << deltaR13 << " deltaR23 = " << deltaR23);

                    }

               }
 



            }
      }

   } else {

      TCS_EXCEPTION("DisambiguationDRIncl3 alg must have  3 inputs, but got " << input.size());

   }
   return TCS::StatusCode::SUCCESS;
}
