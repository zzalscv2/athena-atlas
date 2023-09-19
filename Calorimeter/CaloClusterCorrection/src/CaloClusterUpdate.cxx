/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************

NAME:     CaloClusterUpdate.cxx
PACKAGE:  offline/LArCalorimeter/LArClusterRec

AUTHORS:  H. Ma, S. Rajagopalan
CREATED:  Nov, 2000

PURPOSE:  Recalculate the total energy, eta,phi of a cluster.
	  This should be called after all corrections to individual 
	  samplings are done. 

	  energy = Sum of energy in all sampling
	  eta = average of eta1 and eta2, weighted by energy and
                relative resolution.  
		This needs to be tuned. 
	  phi    = phi of second sampling. 

Base class: LArClusterCorrection (Algorithm)

Atrecon Orig: emreco/qetamod.F

Updated:  May 10, 2000    (SR, HM)
          Migrated to Athena Framework from PASO

Updated:  Jan 5, 2001    (HM)
          QA. 

Updated:  Feb 28, 2003, Mar 4, 2003    (MW)
          protect against unphysical clusters

Updated:  May 5, 2004    (Sven Menke)
	  base class changed from algo to tool

********************************************************************/
// include header files
#include "CaloClusterUpdate.h"

#include "GaudiKernel/MsgStream.h"
#include "CaloGeoHelpers/proxim.h" 
#include "CaloGeoHelpers/CaloPhiRange.h"

using xAOD::CaloCluster;
void CaloClusterUpdate::makeCorrection (const Context& myctx,
                                        CaloCluster* cluster) const
{
  float energy=0; 
  float eta=0; 
  float weta=0;

  // set eta to be weighted average of eta1 and eta2
  ATH_MSG_DEBUG(" inBarrel "<<cluster->inBarrel()
		<< " inEndcap "<<cluster->inEndcap()) ;

  for(int i=0; i<5; i=i+4 )
  { 
  
   if (i==0 && !cluster->inBarrel()) continue;
   if (i==4 && !cluster->inEndcap()) continue;

   CaloSampling::CaloSample sam0 = (CaloSampling::CaloSample)(CaloSampling::PreSamplerB+i);   
   CaloSampling::CaloSample sam1 = (CaloSampling::CaloSample)(CaloSampling::PreSamplerB+i+1);   
   CaloSampling::CaloSample sam2 = (CaloSampling::CaloSample)(CaloSampling::PreSamplerB+i+2);   
   CaloSampling::CaloSample sam3 = (CaloSampling::CaloSample)(CaloSampling::PreSamplerB+i+3);
   
   float e0 = cluster->eSample(sam0);
   float e1 = cluster->eSample(sam1);
   float e2 = cluster->eSample(sam2);
   float e3 = cluster->eSample(sam3);

   // total energy is the sum of each sampling, which had all corrections
   energy += (e0 + e1 + e2 + e3);

   // do not consider including in the average if both energies are negative
   if (e1 <= 0 && e2 <= 0) continue;

   // reject weird clusters. Not even sure it can ever happen
   float eta2 = cluster->etaSample(sam2);
   if (cluster->hasSampling(sam2) && eta2 == -999.) continue;

   float eta1 = cluster->etaSample(sam1);
   if (cluster->hasSampling(sam1) && eta1 == -999.) continue;

   // eta1 has better resolution, so weight it differently
   float w1 = e1*m_w1;
   float w2 = e2;

   // do not include layer if energy is negative
   if (e1 <= 0) {
     w1 = 0;
   }
   else if (e2 <= 0) {
     w2 = 0;
   }

   eta  += (eta1*w1+eta2*w2) ;
   weta += w1+w2; 
  }

  //
  // set them in the cluster. 
  //
  if (eta  != -999. && weta != 0)
    eta  = eta/weta; 
  else {
    ATH_MSG_DEBUG("Weird cluster "
		  " EB1 = " << cluster->eSample(CaloSampling::EMB1)
		  << " etaB1 = " << cluster->etaSample(CaloSampling::EMB1)
		  << " EE1 = " << cluster->eSample(CaloSampling::EME1)
		  << " etaE1 = " << cluster->etaSample(CaloSampling::EME1)
		  << " EB2 = " << cluster->eSample(CaloSampling::EMB2)
		  << " etaB2 = " << cluster->etaSample(CaloSampling::EMB2)
		  << " EE2 = " << cluster->eSample(CaloSampling::EME2)
		  << " etaE2 = " << cluster->etaSample(CaloSampling::EME2));
    if (cluster->inBarrel() && !cluster->inEndcap())
      eta = cluster->etaSample(CaloSampling::EMB2);
    else if (cluster->inEndcap() && !cluster->inBarrel())
      eta = cluster->etaSample(CaloSampling::EME2);
    else {
      if (cluster->eSample(CaloSampling::EMB2) >
	  cluster->eSample(CaloSampling::EME2))
	eta = cluster->etaSample(CaloSampling::EMB2);
      else
	eta = cluster->etaSample(CaloSampling::EME2);
    }
  }

  cluster->setEta(eta);
  cluster->setPhi(cluster->phiBE(2));

  if (m_update_energy (myctx))
    cluster->setE(energy);
}


