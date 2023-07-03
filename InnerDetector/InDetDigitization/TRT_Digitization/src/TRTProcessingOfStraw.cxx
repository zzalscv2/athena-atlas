/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TRTProcessingOfStraw.h"

#include "TRTDigit.h"
#include "TRTTimeCorrection.h"
#include "TRTElectronicsProcessing.h"
#include "TRTNoise.h"
#include "TRTDigCondBase.h"

#include "TRT_PAI_Process/ITRT_PAITool.h"
#include "ITRT_SimDriftTimeTool.h"
#include "TRTDigSettings.h"
#include "TRTDigiHelper.h"

//TRT detector information:
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "TRT_ReadoutGeometry/TRT_Numerology.h"

//helpers for identifiers and hitids (only for debug methods):
#include "InDetIdentifier/TRT_ID.h"
#include "InDetSimEvent/TRTHitIdHelper.h"

#include "GeoPrimitives/GeoPrimitives.h"

// Units & Constants
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Units/PhysicalConstants.h"

// Particle data table
#include "HepPDT/ParticleData.hh"

// For the Athena-based random numbers.
#include "CLHEP/Random/RandPoisson.h" //randpoissonq? (fixme)
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandBinomial.h"
#include "CLHEP/Random/RandExpZiggurat.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include <cmath>
#include <cstdlib> //Always include this when including cmath!


//________________________________________________________________________________
TRTProcessingOfStraw::TRTProcessingOfStraw(const TRTDigSettings* digset,
                                           const InDetDD::TRT_DetectorManager* detmgr,
                                           ITRT_PAITool* paitoolXe,
                                           ITRT_SimDriftTimeTool* simdrifttool,
                                           TRTElectronicsProcessing * ep,
                                           TRTNoise * noise,
                                           TRTDigCondBase* digcond,
                                           const HepPDT::ParticleDataTable* pdt,
                                           const TRT_ID* trt_id,
                                           ITRT_PAITool* paitoolAr,
                                           ITRT_PAITool* paitoolKr,
                                           const ITRT_CalDbTool* calDbTool)

: AthMessaging("TRTProcessingOfStraw"),
  m_settings(digset),
  m_detmgr(detmgr),
  m_pPAItoolXe(paitoolXe),
  m_pPAItoolAr(paitoolAr),
  m_pPAItoolKr(paitoolKr),
  m_pSimDriftTimeTool(simdrifttool),
// m_time_y_eq_zero(0.0),
// m_ComTime(NULL),
  m_pTimeCorrection(nullptr),
  m_pElectronicsProcessing(ep),
  m_pNoise(noise),
  m_pDigConditions(digcond),
  m_pParticleTable(pdt),
  m_alreadywarnedagainstpdg0(false),
  m_id_helper(trt_id)

{
  Initialize(calDbTool);
}

//________________________________________________________________________________
TRTProcessingOfStraw::~TRTProcessingOfStraw()
{
  delete m_pTimeCorrection;
}

//________________________________________________________________________________
void TRTProcessingOfStraw::Initialize(const ITRT_CalDbTool* calDbTool)
{

  m_useMagneticFieldMap    = m_settings->useMagneticFieldMap();
  m_signalPropagationSpeed = m_settings->signalPropagationSpeed() ;
  m_attenuationLength      = m_settings->attenuationLength();
  m_useAttenuation         = m_settings->useAttenuation();
  m_innerRadiusOfStraw     = m_settings->innerRadiusOfStraw();
  m_outerRadiusOfWire      = m_settings->outerRadiusOfWire();
  m_timeCorrection         = m_settings->timeCorrection();        // false for beamType='cosmics'
  m_solenoidFieldStrength  = m_settings->solenoidFieldStrength();

  m_maxelectrons = 100; // 100 gives good Gaussian approximation

  if (m_pPAItoolXe==nullptr) {
    ATH_MSG_FATAL ( "TRT_PAITool for Xenon not defined! no point in continuing!" );
  }
  if (m_pPAItoolKr==nullptr) {
    ATH_MSG_ERROR ( "TRT_PAITool for Krypton is not defined!!! Xenon TRT_PAITool will be used for Krypton straws!" );
    m_pPAItoolKr = m_pPAItoolXe;
  }
  if (m_pPAItoolAr==nullptr) {
    ATH_MSG_ERROR ( "TRT_PAITool for Argon is not defined!!! Xenon TRT_PAITool will be used for Argon straws!" );
    m_pPAItoolAr = m_pPAItoolXe;
  }

  ATH_MSG_INFO ( "Xe barrel drift-time at r = 2 mm is " << m_pSimDriftTimeTool->getAverageDriftTime(2.0, 0.002*0.002, 0) << " ns." );
  ATH_MSG_INFO ( "Kr barrel drift-time at r = 2 mm is " << m_pSimDriftTimeTool->getAverageDriftTime(2.0, 0.002*0.002, 1) << " ns." );
  ATH_MSG_INFO ( "Ar barrel drift-time at r = 2 mm is " << m_pSimDriftTimeTool->getAverageDriftTime(2.0, 0.002*0.002, 2) << " ns." );

  m_pTimeCorrection = new TRTTimeCorrection(m_settings, m_detmgr, m_id_helper, calDbTool);

  const double intervalBetweenCrossings(m_settings->timeInterval() / 3.);
  m_minCrossingTime = - (intervalBetweenCrossings * 2. + 1.*CLHEP::ns);
  m_maxCrossingTime =    intervalBetweenCrossings * 3. + 1.*CLHEP::ns;
  m_shiftOfZeroPoint = static_cast<double>( m_settings->numberOfCrossingsBeforeMain() ) * intervalBetweenCrossings;

  // Tabulate exp(-dist/m_attenuationLength) as a function of dist = time*m_signalPropagationSpeed [0.0 mm, 1500 mm)
  // otherwise we are doing an exp() for every cluster! > 99.9% of output digits are the same, saves 13% CPU time.
  m_expattenuation.reserve(150);
  for (unsigned int k=0; k<150; k++) {
    double dist = 10.0*(k+0.5); // [5mm, 1415mm] max 5 mm error (sigma = 3 mm)
    m_expattenuation.push_back(exp(-dist/m_attenuationLength));
  }

  //For the field effect on drifttimes in this class we will assume that the local x,y coordinates of endcap straws are such that the
  //local y-direction is parallel to the global z-direction. As a sanity check against future code developments we will test this
  //assumption on one straw from each endcap layer.

  if (m_solenoidFieldStrength!=0.0)
    {
      const InDetDD::TRT_Numerology * num = m_detmgr->getNumerology();
      for (unsigned int iwheel = 0; iwheel < num->getNEndcapWheels(); ++iwheel)
        {
          for (unsigned int iside = 0; iside < 2; ++iside)
            { //positive and negative endcap
              for (unsigned int ilayer = 0; ilayer < num->getNEndcapLayers(iwheel); ++ilayer)
                {
                  const InDetDD::TRT_EndcapElement * ec_element
                    = m_detmgr->getEndcapElement(iside,//positive or negative endcap
                                                 iwheel,//wheelIndex,
                                                 ilayer,//strawLayerIndex,
                                                 0);//phiIndex
                  //Local straw center and local straw point one unit along x:
                  if (!ec_element)
                    {
                      ATH_MSG_VERBOSE ( "Failed to retrieve endcap element for (iside,iwheel,ilayer)=("
                                        << iside<<", "<<iwheel<<", "<<ilayer<<")." );
                      continue;
                    }
                  Amg::Vector3D strawcenter(0.0,0.0,0.0);
                  Amg::Vector3D strawx(1.0,0.0,0.0);
                  //Transform to global coordinate (use first straw in element):
                  strawcenter = ec_element->strawTransform(0) * strawcenter;
                  strawx = ec_element->strawTransform(0) * strawx;
                  const Amg::Vector3D v(strawx-strawcenter);
                  const double zcoordfrac((v.z()>0?v.z():-v.z())/v.mag());
                  if (zcoordfrac<0.98 || zcoordfrac > 1.02)
                    {
                      ATH_MSG_WARNING ( "Found endcap straw where the assumption that local x-direction"
                                        <<" is parallel to global z-direction is NOT valid."
                                        <<" Drift times will be somewhat off." );
                    }
                }
            }
        }

      //check local barrel coordinates at four positions.
      for (unsigned int phi_it = 0; phi_it < 32; phi_it++)
        {
          const InDetDD::TRT_BarrelElement * bar_element = m_detmgr->getBarrelElement(1,1,phi_it,1);

          Amg::Vector3D strawcenter(0.0,0.0,0.0);
          Amg::Vector3D straw(cos((double)phi_it*2*M_PI/32.),sin((double)phi_it*2*M_PI/32.),0.0);
          strawcenter = bar_element->strawTransform(0) * strawcenter;
          straw = bar_element->strawTransform(0) * straw;
          const Amg::Vector3D v(straw-strawcenter);
          const double coordfrac(atan2(v.x(),v.y()));
          if (coordfrac>0.2 || coordfrac < -0.2)
            {
              ATH_MSG_WARNING ( "Found barrel straw where the assumption that local y-direction"
                                <<" is along the straw is NOT valid."
                                <<" Drift times will be somewhat off." );
            }
        }
    }

  m_randBinomialXe = std::make_unique<CLHEP::RandBinomialFixedP>(nullptr, 1, m_settings->smearingFactor(0), m_maxelectrons);
  m_randBinomialKr = std::make_unique<CLHEP::RandBinomialFixedP>(nullptr, 1, m_settings->smearingFactor(1), m_maxelectrons);
  m_randBinomialAr = std::make_unique<CLHEP::RandBinomialFixedP>(nullptr, 1, m_settings->smearingFactor(2), m_maxelectrons);

  ATH_MSG_VERBOSE ( "Initialization done" );
}

//________________________________________________________________________________
void TRTProcessingOfStraw::addClustersFromStep ( const double& scaledKineticEnergy, const double& particleCharge,
                                                 const double& timeOfHit,
                                                 const double& prex, const double& prey, const double& prez,
                                                 const double& postx, const double& posty, const double& postz,
                                                 std::vector<cluster>& clusterlist, int strawGasType,
                                                 CLHEP::HepRandomEngine* rndmEngine,
                                                 CLHEP::HepRandomEngine* paiRndmEngine)
{

  // Choose the appropriate ITRT_PAITool for this straw
  ITRT_PAITool* activePAITool;
  if      (strawGasType==0) { activePAITool = m_pPAItoolXe; }
  else if (strawGasType==1) { activePAITool = m_pPAItoolKr; }
  else if (strawGasType==2) { activePAITool = m_pPAItoolAr; }
  else { activePAITool = m_pPAItoolXe; } // should never happen

  //Differences and steplength:
  const double deltaX(postx - prex);
  const double deltaY(posty - prey);
  const double deltaZ(postz - prez);
  const double stepLength(sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ));

  const double meanFreePath(activePAITool->GetMeanFreePath( scaledKineticEnergy, particleCharge*particleCharge ));

  //How many clusters did we actually create:
  const unsigned int numberOfClusters(CLHEP::RandPoisson::shoot(rndmEngine,stepLength / meanFreePath));
  //fixme: use RandPoissionQ?

  //Position each of those randomly along the step, and use PAI to get their energies:
  for (unsigned int iclus(0); iclus<numberOfClusters; ++iclus)
    {
      //How far along the step did the cluster get produced:
      const double lambda(CLHEP::RandFlat::shoot(rndmEngine));

      //Append cluster (the energy is given by the PAI model):
      double clusE(activePAITool->GetEnergyTransfer(scaledKineticEnergy, paiRndmEngine));
      clusterlist.emplace_back(clusE, timeOfHit,
                                    prex + lambda * deltaX,
                                    prey + lambda * deltaY,
                                    prez + lambda * deltaZ);
    }

  
}

//________________________________________________________________________________
void TRTProcessingOfStraw::ProcessStraw ( MagField::AtlasFieldCache& fieldCache,
                                          hitCollConstIter i,
                                          hitCollConstIter e,
                                          TRTDigit& outdigit,
                                          bool & alreadyPrintedPDGcodeWarning,
                                          double cosmicEventPhase, // const ComTime* m_ComTime,
                                          int strawGasType,
                                          bool emulationArflag,
                                          bool emulationKrflag,
                                          CLHEP::HepRandomEngine* rndmEngine,
                                          CLHEP::HepRandomEngine* elecProcRndmEngine,
                                          CLHEP::HepRandomEngine* elecNoiseRndmEngine,
                                          CLHEP::HepRandomEngine* paiRndmEngine)
{

  //////////////////////////////////////////////////////////
  // We need the straw id several times in the following  //
  //////////////////////////////////////////////////////////
  const int hitID((*i)->GetHitID());
  unsigned int region(TRTDigiHelper::getRegion(hitID));
  const bool isBarrel(region<3 );
  //const bool isEC    (!isBarrel);
  //const bool isShort (region==1);
  //const bool isLong  (region==2);
  const bool isECA   (region==3);
  const bool isECB   (region==4);

  //////////////////////////////////////////////////////////
  //======================================================//
  //////////////////////////////////////////////////////////
  // First step is to loop over the simhits in this straw //
  // and produce a list of primary ionisation clusters.   //
  // Each cluster needs the following information:        //
  //                                                      //
  //  * The total energy of the cluster.                  //
  //  * Its location in local straw (x,y,z) coordinates.  //
  //  * The time the cluster was created.                 //
  //                                                      //
  //////////////////////////////////////////////////////////
  //======================================================//
  //////////////////////////////////////////////////////////
  // TimeShift is the same for all the simhits in the straw.
  const double timeShift(m_pTimeCorrection->TimeShift(hitID)); // rename hitID to strawID

  m_clusterlist.clear();

  //For magnetic field evaluation
  Amg::Vector3D TRThitGlobalPos(0.0,0.0,0.0);

  //Now we loop over all the simhits
  for (hitCollConstIter hit_iter= i;  hit_iter != e; ++hit_iter)
    {
      //Get the hit:
      const TimedHitPtr<TRTUncompressedHit> *theHit = &(*hit_iter);

      TRThitGlobalPos = getGlobalPosition(hitID, theHit);

      //Figure out the global time of the hit (all clusters from the hit
      //will get same timing).

      double timeOfHit(0.0); // remains zero if timeCorrection is false (beamType='cosmics')
      if (m_timeCorrection) {
        const double globalHitTime(hitTime(*theHit));
        const double globalTime = static_cast<double>((*theHit)->GetGlobalTime());
        const double bunchCrossingTime(globalHitTime - globalTime);
        if ( (bunchCrossingTime < m_minCrossingTime) || (bunchCrossingTime > m_maxCrossingTime) ) continue;
        timeOfHit = m_shiftOfZeroPoint + globalHitTime - timeShift; //is this shiftofzeropoint correct pileup-wise?? [fixme]
      }

      //if (timeOfHit>125.0) continue; // Will give 3% speed up (but changes seeds!). Needs careful thought though.

      //What kind of particle are we dealing with?
      const int particleEncoding((*theHit)->GetParticleEncoding());

      //Safeguard against pdgcode 0.
      if (particleEncoding == 0)
        {
          //According to Andrea this is usually nuclear fragments from
          //hadronic interactions. They therefore ought to be allowed to
          //contribute, but we ignore them for now - until it can be studied
          //that it is indeed safe to stop doing so
          if (!m_alreadywarnedagainstpdg0) {
            m_alreadywarnedagainstpdg0 = true;
            ATH_MSG_WARNING ( "Ignoring sim. particle with pdgcode 0. This warning is only shown once per job" );
          }
          continue;
        }

      // If it is a photon we assume it was absorbed entirely at the point of interaction.
      // we simply deposit the entire energy into the last point of the sim. step.
      if (particleEncoding == 22)
        {

          const double energyDeposit = (*theHit)->GetEnergyDeposit(); // keV (see comment below)
          // Apply radiator efficiency "fudge factor" to ignore some TR photons (assuming they are over produced in the sim. step.
          // The fraction removed is based on tuning pHT for electrons to data, after tuning pHT for muons (which do not produce TR).
          // The efficiency is different for Xe, Kr and Ar. Avoid fudging non-TR photons (TR is < 30 keV).
          // Also: for |eta|<0.5 apply parabolic scale; see "Parabolic Fudge" https://indico.cern.ch/event/304066/

          if ( energyDeposit<30.0 ) {

            // Improved Argon Emulation tuning October 2018 by Hassane Hamdaoui https://cds.cern.ch/record/2643784
            // Previously 0.15, 0.28, 0.28
            double ArEmulationScaling_BA  = 0.05;
            double ArEmulationScaling_ECA = 0.20;
            double ArEmulationScaling_ECB = 0.20;

            // ROUGH GUESSES RIGHT NOW
            double KrEmulationScaling_BA = 0.20;
            double KrEmulationScaling_ECA = 0.39;
            double KrEmulationScaling_ECB = 0.39;

            if (isBarrel) { // Barrel
              double trEfficiencyBarrel = m_settings->trEfficiencyBarrel(strawGasType);
              double hitx = TRThitGlobalPos[0];
              double hity = TRThitGlobalPos[1];
              double hitz = TRThitGlobalPos[2];
              double hitEta = std::abs(log(tan(0.5*atan2(sqrt(hitx*hitx+hity*hity),hitz))));
              if ( hitEta < 0.5 ) { trEfficiencyBarrel *= ( 0.833333+0.6666667*hitEta*hitEta ); }
              // scale down the TR efficiency if we are emulating
              if ( strawGasType == 0 && emulationArflag ) { trEfficiencyBarrel *= ArEmulationScaling_BA; }
              if ( strawGasType == 0 && emulationKrflag ) { trEfficiencyBarrel *= KrEmulationScaling_BA; }
              if ( CLHEP::RandFlat::shoot(rndmEngine) > trEfficiencyBarrel ) continue; // Skip this photon
            } // close if barrel
            else { // Endcap - no eta dependence here.
              if (isECA) {
                double trEfficiencyEndCapA = m_settings->trEfficiencyEndCapA(strawGasType);
                // scale down the TR efficiency if we are emulating
                if ( strawGasType == 0 && emulationArflag ) { trEfficiencyEndCapA *= ArEmulationScaling_ECA; }
                if ( strawGasType == 0 && emulationKrflag ) { trEfficiencyEndCapA *= KrEmulationScaling_ECA; }
                if ( CLHEP::RandFlat::shoot(rndmEngine) > trEfficiencyEndCapA ) continue; // Skip this photon
              }
              if (isECB) {
                double trEfficiencyEndCapB = m_settings->trEfficiencyEndCapB(strawGasType);
                // scale down the TR efficiency if we are emulating
                if ( strawGasType == 0 && emulationArflag ) { trEfficiencyEndCapB *= ArEmulationScaling_ECB; }
                if ( strawGasType == 0 && emulationKrflag ) { trEfficiencyEndCapB *= KrEmulationScaling_ECB; }
                if ( CLHEP::RandFlat::shoot(rndmEngine) > trEfficiencyEndCapB ) continue; // Skip this photon
              }
            } // close else (end caps)
          } // energyDeposit < 30.0

          // Append this (usually highly energetic) cluster to the list:
          m_clusterlist.emplace_back( energyDeposit*CLHEP::keV, timeOfHit, (*theHit)->GetPostStepX(), (*theHit)->GetPostStepY(), (*theHit)->GetPostStepZ() 
                                  );

          // Regarding the CLHEP::keV above: In TRT_G4_SD we converting the hits to keV,
          // so here we convert them back to CLHEP units by multiplying by CLHEP::keV.
        }
      //Special treatment of magnetic monopoles && highly charged Qballs (charge > 10)
      else if ( ((abs(particleEncoding)/100000==41) && (abs(particleEncoding)/10000000==0)) ||
                ((static_cast<int>(abs(particleEncoding)/10000000) == 1) &&
                 (static_cast<int>(abs(particleEncoding)/100000) == 100) &&
                 (static_cast<int>((abs(particleEncoding))-10000000)/100>10)) )
        {
          m_clusterlist.emplace_back((*theHit)->GetEnergyDeposit()*CLHEP::keV, timeOfHit, (*theHit)->GetPostStepX(), (*theHit)->GetPostStepY(), (*theHit)->GetPostStepZ() 
                                  );
        }
      else { // It's not a photon, monopole or Qball with charge > 10, so we proceed with regular ionization using the PAI model

        // Lookup mass and charge from the PDG info in CLHEP HepPDT:
        const HepPDT::ParticleData *particle(m_pParticleTable->particle(HepPDT::ParticleID(abs(particleEncoding))));
        double particleCharge(0.);
        double particleMass(0.);

        if (particle)
          {
            particleCharge = particle->charge();
            particleMass = particle->mass().value();
            if ((static_cast<int>(abs(particleEncoding)/10000000) == 1) && (static_cast<int>(abs(particleEncoding)/100000)==100))
              {
                particleCharge =  (particleEncoding>0 ? 1. : -1.) *(((abs(particleEncoding) / 100000.0) - 100.0) * 1000.0);
              }
            else if ((static_cast<int>(abs(particleEncoding)/10000000) == 2) && (static_cast<int>(abs(particleEncoding)/100000)==200))
              {
                particleCharge =  (particleEncoding>0 ? 1. : -1.) *((double)((abs(particleEncoding) / 1000) % 100) / (double)((abs(particleEncoding) / 10) % 100));
              }
          }
        else
          {
            const int number_of_digits(static_cast<int>(log10((double)abs(particleEncoding))+1.));
            if (number_of_digits != 10)
              {
                ATH_MSG_ERROR ( "Data for sim. particle with pdgcode "<<particleEncoding
                                <<" does not have 10 digits and could not be retrieved from PartPropSvc. Assuming mass and charge as pion." );
                particleCharge = 1.;
                particleMass = 139.57018*CLHEP::MeV;
              }
            else if (( number_of_digits == 10 ) && (static_cast<int>(abs(particleEncoding)/100000000)!=10) )
              {
                ATH_MSG_ERROR ( "Data for sim. particle with pdgcode "<<particleEncoding
                                <<" has 10 digits, could not be retrieved from PartPropSvc, and is inconsistent with ion pdg convention (+/-10LZZZAAAI)."
                                <<" Assuming mass and charge as pion." );
                particleCharge = 1.;
                particleMass = 139.57018*CLHEP::MeV;
              }
            else if ((number_of_digits==10) && (static_cast<int>(abs(particleEncoding)/100000000)==10))
              {
                const int A(static_cast<int>((((particleEncoding)%1000000000)%10000)/10.));
                const int Z(static_cast<int>((((particleEncoding)%10000000)-(((particleEncoding)%1000000000)%10000))/10000.));

                const double Mp(938.272*CLHEP::MeV);
                const double Mn(939.565*CLHEP::MeV);

                particleCharge = (particleEncoding>0 ? 1. : -1.) * static_cast<double>(Z);
                particleMass = std::abs( Z*Mp+(A-Z)*Mn );

                if (!alreadyPrintedPDGcodeWarning)
                  {
                    ATH_MSG_WARNING ( "Data for sim. particle with pdgcode "<<particleEncoding
                                      <<" could not be retrieved from PartPropSvc (unexpected ion)."
                                      <<" Calculating mass and charge from pdg code. "
                                      <<" The result is: Charge = "<<particleCharge<<" Mass = "<<particleMass<<"MeV" );
                    alreadyPrintedPDGcodeWarning = true;
                  }
              }
          }

        if (!particleCharge)//Abort if uncharged particle.
          {
            continue;
          }
        if (!particleMass)
          { //Abort if weird massless charged particle.
            ATH_MSG_WARNING ( "Ignoring ionization from sim. particle with pdgcode "<<particleEncoding
                              <<" since it appears to be a massless charged particle." );
            continue;
          }

        //We are now in the most likely case: A normal ionizing
        //particle. Using the PAI model we are going to distribute
        //ionization clusters along the step taken by the sim particle.

        const double scaledKineticEnergy( static_cast<double>((*theHit)->GetKineticEnergy()) * ( CLHEP::proton_mass_c2 / particleMass ));

        addClustersFromStep ( scaledKineticEnergy, particleCharge, timeOfHit,
                              (*theHit)->GetPreStepX(),(*theHit)->GetPreStepY(),(*theHit)->GetPreStepZ(),
                              (*theHit)->GetPostStepX(),(*theHit)->GetPostStepY(),(*theHit)->GetPostStepZ(),
                              m_clusterlist, strawGasType, rndmEngine, paiRndmEngine);

      }
    }//end of hit loop

  //////////////////////////////////////////////////////////
  //======================================================//
  //////////////////////////////////////////////////////////
  // Second step is, using the cluster list along with    //
  // gas and wire properties, to create a list of energy  //
  // deposits (i.e. potential fluctuations) reaching the  //
  // frontend electronics. Each deposit needs:            //
  //                                                      //
  //  * The energy of the deposit.                        //
  //  * The time it arrives at the FE electronics         //
  //                                                      //
  //////////////////////////////////////////////////////////
  //======================================================//
  //////////////////////////////////////////////////////////

  m_depositList.clear();

  ClustersToDeposits(fieldCache, hitID, m_clusterlist, m_depositList, TRThitGlobalPos, cosmicEventPhase, strawGasType, rndmEngine );


  //////////////////////////////////////////////////////////
  //======================================================//
  //////////////////////////////////////////////////////////
  // The third and final step is to simulate how the FE   //
  // turns the results into an output digit. This         //
  // includes the shaping/amplification and subsequent    //
  // discrimination as well as addition of noise.         //
  //                                                      //
  //////////////////////////////////////////////////////////
  //======================================================//
  //////////////////////////////////////////////////////////

  //If no deposits, and no electronics noise we might as well stop here:
  if ( m_depositList.empty() && !m_pNoise )
    {
      outdigit = TRTDigit(hitID, 0);
      return;
    }

  //Get straw conditions data:
  double lowthreshold, noiseamplitude;
  if (m_settings->noiseInSimhits()) {
    m_pDigConditions->getStrawData( hitID, lowthreshold, noiseamplitude );
  } else {
    lowthreshold = isBarrel ? m_settings->lowThresholdBar(strawGasType) : m_settings->lowThresholdEC(strawGasType);
    noiseamplitude = 0.0;
  }

  //Electronics processing:
  m_pElectronicsProcessing->ProcessDeposits( m_depositList, hitID, outdigit, lowthreshold, noiseamplitude, strawGasType, elecProcRndmEngine, elecNoiseRndmEngine );
}

//________________________________________________________________________________
void TRTProcessingOfStraw::ClustersToDeposits (MagField::AtlasFieldCache& fieldCache, const int& hitID,
					       const std::vector<cluster>& clusters,
					       std::vector<TRTElectronicsProcessing::Deposit>& deposits,
					       Amg::Vector3D TRThitGlobalPos,
					       double cosmicEventPhase, // was const ComTime* m_ComTime,
                                               int strawGasType,
                                               CLHEP::HepRandomEngine* rndmEngine)
{

  //
  // Some initial work before looping over the cluster
  //

  deposits.clear();

  unsigned int region(TRTDigiHelper::getRegion(hitID));
  const bool isBarrel(region<3 );
  const bool isEC    (!isBarrel);
  const bool isShort (region==1);
  const bool isLong  (region==2);
  //const bool isECA   (region==3);
  //const bool isECB   (region==4);

  CLHEP::RandBinomialFixedP *randBinomial{};
  if      (strawGasType == 0) { randBinomial = m_randBinomialXe.get(); }
  else if (strawGasType == 1) { randBinomial = m_randBinomialKr.get(); }
  else if (strawGasType == 2) { randBinomial = m_randBinomialAr.get(); }
  else { randBinomial = m_randBinomialXe.get(); } // should never happen

  double ionisationPotential = m_settings->ionisationPotential(strawGasType);
  double smearingFactor      = m_settings->smearingFactor(strawGasType);

  std::vector<cluster>::const_iterator currentClusterIter(clusters.begin());
  const std::vector<cluster>::const_iterator endOfClusterList(clusters.end());

  // if (m_settings->doCosmicTimingPit()) {
  //     if (m_ComTime) { m_time_y_eq_zero = m_ComTime->getTime(); }
  //     else           { ATH_MSG_WARNING("Configured to use ComTime tool, but did not find tool. All hits will get the same t0"); }
  // }

  // The average drift time is affected by the magnetic field which is not uniform so we need to use the
  // detailed magnetic field map to obtain an effective field for this straw (used in SimDriftTimeTool).
  // This systematically affects O(ns) drift times in the end cap, but has a very small effect in the barrel.
  Amg::Vector3D globalPosition;
  Amg::Vector3D mField;
  double map_x2(0.),map_y2(0.),map_z2(0.);
  double effectiveField2(0.); // effective field squared.

  // Magnetic field is the same for all clusters in the straw so this can be called before the cluster loop.
  if (m_useMagneticFieldMap)
    {
      globalPosition[0]=TRThitGlobalPos[0]*CLHEP::mm;
      globalPosition[1]=TRThitGlobalPos[1]*CLHEP::mm;
      globalPosition[2]=TRThitGlobalPos[2]*CLHEP::mm;

      // MT Field cache is stored in cache
      fieldCache.getField         (globalPosition.data(), mField.data());

      map_x2 = mField.x()*mField.x(); // would be zero for a uniform field
      map_y2 = mField.y()*mField.y(); // would be zero for a uniform field
      map_z2 = mField.z()*mField.z(); // would be m_solenoidfieldstrength^2 for uniform field
    }

  // Now we are ready to loop over clusters and form timed energy deposits at the wire:
  //
  //  1. First determine the number of surviving drift electrons and the energy of their deposits.
  //  2. Then determine the timing of these deposits.

  // Notes
  //
  // 1) It turns out that the total energy deposited (Ed) is, on average, equal to the cluster energy (Ec):
  // <Ed> = m_ionisationPotential/m_smearingFactor * Ec/m_ionisationPotential * m_smearingFactor = Ec.
  // Actually it exceeds this *slightly* due to the +1 electron in the calculation of nprimaryelectrons below.
  // The energy processing below therefore exists only to model the energy *fluctuations*.
  //
  // 2) Gaussian approximation (for photons, HIPs etc):
  // For large Ec (actually nprimaryelectrons > 99) a much faster Gaussian shoot replaces
  // the Binomial and many exponential shoots. The Gaussian pdf has a mean of Ec and variance
  // given by sigma^2 = Ec*ionisationPotential*(2-smearingFactor)/smearingFactor. That expression
  // comes from the sum of the variances from the Binomial and Exponential. In this scheme it is
  // sufficient (for diffusion) to divided the energy equally amongst 100 (maxelectrons) electrons.

  // straw radius
  const double  wire_r2 = m_outerRadiusOfWire*m_outerRadiusOfWire;
  const double straw_r2 = m_innerRadiusOfStraw*m_innerRadiusOfStraw;

  // Cluster loop
  for (;currentClusterIter!=endOfClusterList;++currentClusterIter)
    {
      // Get the cluster radius and energy.
      const double cluster_x(currentClusterIter->xpos);
      const double cluster_y(currentClusterIter->ypos);
      const double cluster_z(this->setClusterZ(currentClusterIter->zpos, isLong, isShort, isEC));
      const double cluster_x2(cluster_x*cluster_x);
      const double cluster_y2(cluster_y*cluster_y);
      double cluster_r2(cluster_x2+cluster_y2);

      // These may never occur, but could be very problematic for getAverageDriftTime(), so check and correct this now.
      if (cluster_r2<wire_r2)  cluster_r2=wire_r2;  // Compression may (v. rarely) cause r to be smaller than the wire radius. If r=0 then NaN's later!
      if (cluster_r2>straw_r2) cluster_r2=straw_r2; // Should never occur

      const double cluster_r(std::sqrt(cluster_r2));      // cluster radius
      const double cluster_E(currentClusterIter->energy); // cluster energy
      const unsigned int nprimaryelectrons( static_cast<unsigned int>( cluster_E / ionisationPotential + 1.0 ) );

      // Determine the number of surviving electrons and their energy sum.
      // If nprimaryelectrons > 100 then a Gaussian approximation is good (and much quicker)

      double depositEnergy(0.); // This will be the energy deposited at the wire.

      if (nprimaryelectrons<m_maxelectrons) // Use the detailed Binomial and Exponential treatment at this low energy.
        {
          unsigned int nsurvivingprimaryelectrons = static_cast<unsigned int>(randBinomial->fire(rndmEngine,nprimaryelectrons) + 0.5);
          if (nsurvivingprimaryelectrons==0) continue; // no electrons survived; move on to the next cluster.
          const double meanElectronEnergy(ionisationPotential / smearingFactor);
          for (unsigned int ielec(0); ielec<nsurvivingprimaryelectrons; ++ielec) {
            depositEnergy += CLHEP::RandExpZiggurat::shoot(rndmEngine, meanElectronEnergy);
          }
        }
      else // Use a Gaussian approximation
        {
          const double fluctSigma(sqrt(cluster_E * ionisationPotential * (2 - smearingFactor) / smearingFactor));
          do {
            depositEnergy = CLHEP::RandGaussZiggurat::shoot(rndmEngine, cluster_E, fluctSigma);
          } while(depositEnergy<0.0); // very rare.
        }

      // Now we have the depositEnergy, we need to work out the timing and attenuation

      // First calculate the "effective field" (it's never negative):
      // Electron drift time is prolonged by the magnetic field according to an "effective field".
      // For the endcap, the effective field is dependent on the relative (x,y) position of the cluster.
      // It is assumed here (checked in initialize) that the local straw x-direction is parallel with
      // the solenoid z-direction. After Garfield simulations and long thought it was found that only
      // field perpendicular to the electron drift is of importance.

      if (!isBarrel) // Endcap
        {
          if (m_useMagneticFieldMap) { // Using magnetic field map
            effectiveField2 = map_z2*cluster_y2/cluster_r2 + map_x2 + map_y2;
          }
          else { // Not using magnetic field map (you really should not do this!):
            effectiveField2 = m_solenoidFieldStrength*m_solenoidFieldStrength * cluster_y2 / cluster_r2;
          }
        }
      else // Barrel
        {
          if (m_useMagneticFieldMap) { // Using magnetic field map (here bug #91830 is corrected)
            effectiveField2 = map_z2 + (map_x2+map_y2)*cluster_y2/cluster_r2;
          }
          else { // Without the mag field map (very small change in digi output)
            effectiveField2 = m_solenoidFieldStrength*m_solenoidFieldStrength;
          }
        }

      // If there is no field we might need to reset effectiveField2 to zero.
      if (m_solenoidFieldStrength == 0. ) effectiveField2=0.;

      // Now we need the deposit time which is the sum of three components:
      // 1. Time of the hit(cluster): clusterTime.
      // 2. Drift times: either commondrifttime, or diffused m_drifttimes
      // 3. Wire propagation times: timedirect and timereflect.

      // get the time of the hit(cluster)
      double clusterTime(currentClusterIter->time);

      if ( m_settings->doCosmicTimingPit() )
        { // make (x,y) dependent? i.e: + f(x,y).
          // clusterTime = clusterTime - m_time_y_eq_zero + m_settings->jitterTimeOffset()*( CLHEP::RandFlat::shoot(rndmEngine) );
          clusterTime = clusterTime + cosmicEventPhase + m_settings->jitterTimeOffset()*( CLHEP::RandFlat::shoot(rndmEngine) );
          // yes it is a '+' now. Ask Alex Alonso.
        }

      // get the wire propagation times (for direct and reflected signals)
      double timedirect(0.), timereflect(0.);
      m_pTimeCorrection->PropagationTime( hitID, cluster_z, timedirect, timereflect );

      // While we have the propagation times, we can calculate the exponential attenuation factor
      double expdirect(1.0), expreflect(1.0); // Initially set to "no attenuation".
      if (m_useAttenuation)
        {
          //expdirect  = exp( -timedirect *m_signalPropagationSpeed / m_attenuationLength);
          //expreflect = exp( -timereflect*m_signalPropagationSpeed / m_attenuationLength);
          // Tabulating exp(-dist/m_attenuationLength) with only 150 elements: index [0,149].
          //   > 99.9% of output digits are the same, saves 13% CPU time.
          // Distances the signal propagate along the wire:
          // * distdirect is rarely negative (<0.2%) by ~ mm. In such cases there is
          //   no attenuation, which is equivalent to distdirect=0 and so is good.
          // * distreflect is always +ve and less than 1500, and so is good.
          // The code is protected against out of bounds in anycase.
          const double distdirect  = timedirect *m_signalPropagationSpeed;
          const double distreflect = timereflect*m_signalPropagationSpeed;
          const unsigned int kdirect  = static_cast<unsigned int>(distdirect/10);
          const unsigned int kreflect = static_cast<unsigned int>(distreflect/10);
          if (kdirect<150) expdirect  = m_expattenuation[kdirect];    // otherwise there
          if (kreflect<150) expreflect = m_expattenuation[kreflect];  // is no attenuation.
        }

      // Finally, deposit the energy on the wire using the drift-time tool (diffusion is no longer available).
      double commondrifttime = m_pSimDriftTimeTool->getAverageDriftTime(cluster_r,effectiveField2,strawGasType);
      double dt = clusterTime + commondrifttime;
      deposits.emplace_back(0.5*depositEnergy*expdirect,  timedirect+dt);
      deposits.emplace_back(0.5*depositEnergy*expreflect, timereflect+dt);

    } // end of cluster loop

  }

//________________________________________________________________________________
Amg::Vector3D TRTProcessingOfStraw::getGlobalPosition (  int hitID, const TimedHitPtr<TRTUncompressedHit> *theHit ) {

  const int mask(0x0000001F);
  int word_shift(5);
  int trtID, ringID, moduleID, layerID, strawID;
  int wheelID, planeID, sectorID;

  const InDetDD::TRT_BarrelElement *barrelElement;
  const InDetDD::TRT_EndcapElement *endcapElement;

  if ( !(hitID & 0x00200000) ) {      // barrel

    strawID   = hitID & mask;
    hitID   >>= word_shift;
    layerID   = hitID & mask;
    hitID   >>= word_shift;
    moduleID  = hitID & mask;
    hitID   >>= word_shift;
    ringID    = hitID & mask;
    trtID     = hitID >> word_shift;

    barrelElement = m_detmgr->getBarrelElement(trtID, ringID, moduleID, layerID);

    if (barrelElement) {
      const Amg::Vector3D v( (*theHit)->GetPreStepX(),(*theHit)->GetPreStepY(),(*theHit)->GetPreStepZ());
      return barrelElement->strawTransform(strawID)*v;
    }

  } else {                           // endcap

    strawID   = hitID & mask;
    hitID   >>= word_shift;
    planeID   = hitID & mask;
    hitID   >>= word_shift;
    sectorID  = hitID & mask;
    hitID   >>= word_shift;
    wheelID   = hitID & mask;
    trtID     = hitID >> word_shift;

    // change trtID (which is 2/3 for endcaps) to use 0/1 in getEndcapElement
    if (trtID == 3) trtID = 0;
    else            trtID = 1;

    endcapElement = m_detmgr->getEndcapElement(trtID, wheelID, planeID, sectorID);

    if ( endcapElement ) {
      const Amg::Vector3D v( (*theHit)->GetPreStepX(),(*theHit)->GetPreStepY(),(*theHit)->GetPreStepZ());
      return endcapElement->strawTransform(strawID)*v;
    }

  }

  ATH_MSG_WARNING ( "Could not find global coordinate of a straw - drifttime calculation will be inaccurate" );
  return {0.0,0.0,0.0};

}


double TRTProcessingOfStraw::setClusterZ(double cluster_z_in, bool isLong, bool isShort, bool isEC) const {
  double cluster_z(cluster_z_in);

  // The active gas volume along the straw z-axis is: Barrel long +-349.315 mm; Barrel short +-153.375 mm; End caps +-177.150 mm.
  // Here we give a warning for clusters that are outside of the straw gas volume in in z. Since T/P version 3 cluster z values
  // can go several mm outside these ranges; 30 mm is plenty allowance in the checks below.
  const double  longBarrelStrawHalfLength(349.315*CLHEP::mm);
  const double shortBarrelStrawHalfLength(153.375*CLHEP::mm);
  const double      EndcapStrawHalfLength(177.150*CLHEP::mm);
  if ( isLong  && std::abs(cluster_z)>longBarrelStrawHalfLength+30 ) {
    double d = cluster_z<0 ? cluster_z+longBarrelStrawHalfLength : cluster_z-longBarrelStrawHalfLength;
    ATH_MSG_WARNING ("Long barrel straw cluster is outside the active gas volume z = +- 349.315 mm by " << d << " mm.");
    ATH_MSG_WARNING ("Setting cluster_z = 0.0");
    cluster_z = 0.0;
  }
  if ( isShort && std::abs(cluster_z)>shortBarrelStrawHalfLength+30 ) {
    double d = cluster_z<0 ? cluster_z+shortBarrelStrawHalfLength : cluster_z-shortBarrelStrawHalfLength;
    ATH_MSG_WARNING ("Short barrel straw cluster is outside the active gas volume z = +- 153.375 mm by " << d << " mm.");
    ATH_MSG_WARNING ("Setting cluster_z = 0.0");
    cluster_z = 0.0;
  }
  if ( isEC    && std::abs(cluster_z)>EndcapStrawHalfLength+30 ) {
    double d = cluster_z<0 ? cluster_z+EndcapStrawHalfLength : cluster_z-EndcapStrawHalfLength;
    ATH_MSG_WARNING ("End cap straw cluster is outside the active gas volume z = +- 177.150 mm by " << d << " mm.");
    ATH_MSG_WARNING ("Setting cluster_z = 0.0");
    cluster_z = 0.0;
  }
  return cluster_z;
}
