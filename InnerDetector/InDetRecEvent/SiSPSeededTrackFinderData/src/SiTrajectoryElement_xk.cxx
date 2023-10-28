/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SiSPSeededTrackFinderData/SiTrajectoryElement_xk.h"

#include "TrkSurfaces/PerigeeSurface.h" 
#include "TrkSurfaces/AnnulusBounds.h"
#include "TrkExUtils/JacobianHelper.h"
#include "TrkExUtils/RungeKuttaUtils.h" 
#include "TrkMaterialOnTrack/ScatteringAngles.h"
#include "TrkMaterialOnTrack/MaterialEffectsOnTrack.h"
#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"

#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"

#include "StoreGate/ReadCondHandle.h"

#include <cmath>
#include <memory>
#include <stdexcept>

///////////////////////////////////////////////////////////////////
// Set work dead trajectory element
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::setDead(const Trk::Surface* SU)
{
  m_fieldMode    = false;
  if(m_tools->fieldTool().magneticFieldMode()!=0) m_fieldMode = true;
  m_status       = 0                       ;
  m_detstatus    = 0                       ;
  m_nMissing        = 0                       ;
  m_nlinksForward      = 0                       ;
  m_nlinksBackward     = 0                       ;
  m_nholesForward      = 0                       ;
  m_nholesBackward     = 0                       ;
  m_dholesForward      = 0                       ;
  m_dholesBackward     = 0                       ;
  m_nclustersForward   = 0                       ;
  m_nclustersBackward  = 0                       ;
  m_npixelsBackward    = 0                       ;
  m_ndfForward         = 0                       ;
  m_ndfBackward        = 0                       ;
  m_ntsos              = 0                       ;
  m_detelement   = nullptr                       ;
  m_detlink      = nullptr                       ;
  m_surface      = SU                      ;
  m_cluster      = nullptr                       ;
  m_clusterOld   = nullptr                       ;
  m_clusterNoAdd = nullptr                       ;
  noiseInitiate()                          ;
  m_radlength    = -1.                     ;
  m_inside       = -1                      ;
  m_localDir[0] = 1.;
  m_localDir[1] = 0.;
  m_localDir[2] = 0.;
  return true;
}

///////////////////////////////////////////////////////////////////
// Set passive material contribution in radiation lengths
// Contribution increases as a function of z to mimic the
// routing of additional services along z
///////////////////////////////////////////////////////////////////
void InDet::SiTrajectoryElement_xk::setDeadRadLength(Trk::PatternTrackParameters& Tp)
{
  if(m_radlength >= 0.) return;
  double z = std::abs(Tp.parameters()[1]);
  if     (z <  50.)   m_radlength = .0075;
  else if(z <  205.)  m_radlength = .02;
  else if(z <  360.)  m_radlength = .03+(z-205.)*0.000129032;
  else if(z <  570.)  m_radlength = .035+(z-360.)*9.04762e-05;
  else if(z <  1500.) m_radlength = .054+(z-570.)*1.82796e-05;
  else if(z <  2400.) m_radlength = .071+(z-1500.)*7.14286e-06;
  else                m_radlength = .078;

}


///////////////////////////////////////////////////////////////////
// Set work information to trajectory
///////////////////////////////////////////////////////////////////

void InDet::SiTrajectoryElement_xk::setTools(const InDet::SiTools_xk* t)
{
  m_tools        = t                      ;
  m_prdToTrackMap= m_tools->PRDtoTrackMap();
  m_useassoTool  = m_tools->usePRDtoTrackAssociation() ;
  m_updatorTool  = m_tools->updatorTool ();
  m_proptool     = m_tools->propTool    ();
  m_riotool      = m_tools->rioTool     ();
  m_tools->fieldCondObj()->getInitializedCache (m_fieldCache);
}

void InDet::SiTrajectoryElement_xk::setParameters()
{
  m_maxholes     = m_tools->maxholes   ()  ; 
  m_maxdholes    = m_tools->maxdholes  ()  ; 
  m_xi2max       = m_tools->xi2max     ()  ;
  m_xi2maxNoAdd  = m_tools->xi2maxNoAdd()  ;  
  m_xi2maxlink   = m_tools->xi2maxlink ()  ;
  m_xi2multi     = m_tools->xi2multi   ()  ;
} 

///////////////////////////////////////////////////////////////////
// Initiate first element of trajectory using external 
// track parameters
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::firstTrajectorElement
(const Trk::TrackParameters& startingParameters, const EventContext& ctx)
{
  /// if we don't have a cluster, something went wrong! 
  if(!m_cluster) return false;     

  /// generate pattern track parameters from the starting parameters
  Trk::PatternTrackParameters startingPatternPars; 
  if(!startingPatternPars.production(&startingParameters)) return false;

  /// get the surface belonging to our staring pars
  const Trk::Surface* pl = &startingPatternPars.associatedSurface();

  /// Track propagation if needed
  /// if we are already on the correct surface, we can assign the params as they ar
  if(m_surface==pl) m_parametersPredForward = startingPatternPars;
  /// otherwise, we need to propagate to "our" surface
  else  if(!propagate(startingPatternPars,m_parametersPredForward,m_step,ctx)) return false;

  // Initiate track parameters without initial covariance
  //
  double cv[15]={ 
      1. ,
		  0. , 1.,
		  0. , 0.,.001,
		  0. , 0.,  0.,.001,
		  0. , 0.,  0.,  0.,.00001};

  if (m_tools->isITkGeometry()) {
    cv[5] = m_ndf==2 ? .1 : .01;
    cv[9] = cv[5];
  }

  /// write our covariance into the parameters
  m_parametersPredForward.setCovariance(cv);
  /// and initiate our state. parametersUF will be constrained 
  /// to the local cluster coordinates (with its covariance), 
  /// while taking the momentum and direction from the input 
  /// parameters 
  initiateState(m_parametersPredForward,m_parametersUpdatedForward);

  /// add noise to the parameter estimate (scattering, eloss) 
  if(!m_tools->isITkGeometry()) noiseProduction(1,m_parametersUpdatedForward);
  else noiseProduction(1,m_parametersUpdatedForward,1.);

  /// and init other values
  m_dist               = -10. ;
  m_step               =  0.  ;
  m_xi2Forward         =  0.  ;
  m_xi2totalForward    =  0.  ;
  m_status             =  1   ;
  m_inside             = -1   ;
  m_nMissing           =  0   ;
  m_nlinksForward      =  0   ;
  m_nholesForward      =  0   ;
  m_dholesForward      =  0   ;
  m_clusterNoAdd       =  nullptr   ;
  m_nclustersForward   =  1   ;
  m_ndfForward         = m_ndf;
  return true;
}

///////////////////////////////////////////////////////////////////
// Initiate first element of trajectory using smoother result
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::firstTrajectorElement(bool correction)
{

  if(!m_cluster || !m_status) return false;
  if(m_status > 1 ) m_parametersPredForward = m_parametersUpdatedBackward;

  if(correction){
    m_parametersPredForward.diagonalization(10.);
    if(!initiateStateWithCorrection(m_parametersPredBackward,m_parametersPredForward,m_parametersUpdatedForward)) return false;
  }
  else{
    /// reset the cov (off-diagonal to zero, diagonal multiplied by 100)
    m_parametersPredForward.diagonalization(100.);
    if(!initiateState(m_parametersPredForward,m_parametersUpdatedForward)) return false;
  }

  m_invMoment = std::abs(m_parametersUpdatedBackward.parameters()[4]);
  noiseProduction(1,m_parametersUpdatedForward);

  m_dist               = -10. ;
  m_xi2Forward         =  0.  ;
  m_xi2totalForward    =  0.  ;
  m_status             =  1   ;
  m_inside             = -1   ;
  m_nMissing           =  0   ;
  m_nlinksForward      =  0   ;
  m_nholesForward      =  0   ;
  m_dholesForward      =  0   ;
  m_clusterNoAdd       =  nullptr   ;
  m_nclustersForward   =  1   ;
  m_ndfForward         = m_ndf;
  return true;
}

///////////////////////////////////////////////////////////////////
// Initiate last element of trajectory
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::lastTrajectorElement()
{
  if(m_status==0 || !m_cluster) return false; 
  noiseProduction(1,m_parametersUpdatedForward);

  m_parametersSM = m_parametersPredForward;
  m_parametersPredBackward = m_parametersUpdatedForward;
  m_parametersPredBackward.diagonalization(100.);
  if(!initiateState(m_parametersPredBackward,m_parametersUpdatedBackward)) return false;

  m_status              =      3 ;
  m_inside              =     -1 ;
  m_nMissing            =      0 ;
  m_nlinksBackward      =      0 ;
  m_nholesBackward      =      0 ;
  m_dholesBackward      =      0 ;
  m_clusterNoAdd        =      nullptr ;
  m_nclustersBackward   =      1 ;
  m_ndf == 2 ? m_npixelsBackward = 1 : m_npixelsBackward = 0;
  m_ndfBackward         =  m_ndf ;
  m_xi2Backward         =  m_xi2Forward;
  m_xi2totalBackward    =  m_xi2Forward;
  m_dist                =    -10.;
  return true;
}

///////////////////////////////////////////////////////////////////
// Initiate last element of trajectory 
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::lastTrajectorElementPrecise()
{
  if(m_status==0 || !m_cluster) return false;
  m_radlength = .04;
  noiseProduction(1,m_parametersUpdatedForward);

  m_parametersSM = m_parametersPredForward;
  m_parametersPredBackward = m_parametersUpdatedForward;
  m_parametersPredBackward.diagonalization(10.);
  if(!initiateStatePrecise(m_parametersPredBackward,m_parametersUpdatedBackward)) return false;
  m_status             =  3;
  m_inside             = -1;
  m_nMissing           =  0;
  m_nlinksBackward     =  0;
  m_nholesBackward     =  0;
  m_dholesBackward     =  0;
  m_clusterNoAdd       =  nullptr;
  m_nclustersBackward  =  1;
  m_clusterNoAdd       =  nullptr;
  m_npixelsBackward = m_ndf==2 ? 1 : 0;
  m_ndfBackward        =  m_ndf ;
  m_xi2Backward        =  m_xi2Forward;
  m_xi2totalBackward   =  m_xi2Forward; // As in 21.9, should this be m_xi2totalForward instead?
  m_dist         =    -10.;
  return true;
}

///////////////////////////////////////////////////////////////////
// Propagate information in forward direction without closest 
// clusters search
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::ForwardPropagationWithoutSearch
(InDet::SiTrajectoryElement_xk& TE, const EventContext& ctx)
{
  /// Track propagation
  /// If the starting trajectory element has a cluster: 
  if(TE.m_cluster) {
    /// add noise to the track parameters of the starting TE 
    TE.m_parametersUpdatedForward.addNoise   (TE.m_noise,Trk::alongMomentum);
    /// propagate to the current element, plug into m_parametersPredForward
    if(!propagate(TE.m_parametersUpdatedForward,m_parametersPredForward,m_step, ctx)) return false;
    /// and restore the starting TE again 
    TE.m_parametersUpdatedForward.removeNoise(TE.m_noise,Trk::alongMomentum);
    /// reset the double hole count to zero - we had a cluster
    m_dholesForward = 0;
  }
  else             {
    /// add noise to the starting pars 
    TE.m_parametersPredForward.addNoise   (TE.m_noise,Trk::alongMomentum);
    /// propagate, plug into m_parametersPredForward
    if(!propagate(TE.m_parametersPredForward,m_parametersPredForward,m_step, ctx)) return false;
    /// restore starting TE
    TE.m_parametersPredForward.removeNoise(TE.m_noise,Trk::alongMomentum);
    /// double hole count is copied from the previous one
    m_dholesForward = TE.m_dholesForward;
  }

  /// copy information from starting TE 
  m_nclustersForward = TE.m_nclustersForward;
  m_nholesForward    = TE.m_nholesForward   ; 
  m_nMissing      = TE.m_nMissing     ;
  m_ndfForward       = TE.m_ndfForward      ;
  m_xi2totalForward  = TE.m_xi2totalForward ;
  m_step      += TE.m_step      ;  

  /// Track update
  /// If we have a cluster on this element 
  if(!m_tools->isITkGeometry() || m_detelement){
    if( m_cluster) {
      /// add it to refine the parameter estimate
      if(!addCluster(m_parametersPredForward,m_parametersUpdatedForward,m_xi2Forward)) return false;
      /// set m_inside to signify this is a hit on track
      m_inside  = -1;
      /// increment cluster count
      ++m_nclustersForward;
      /// increment chi² and degrees of freedom
      m_xi2totalForward+=m_xi2Forward;
      m_ndfForward+=m_ndf;
    }
    /// if we have no cluster on this element
    else           {
      /// check the intersection of the propagated track with this surface
      checkBoundaries(m_parametersPredForward);
      /// if we have hits on this element
      if( m_detstatus >=0) {
	/// and expect to be inside the active area
	if(m_inside <  0 ) {
	  /// then increment the hole counters
	  ++m_nholesForward;
	  ++m_dholesForward;
	}
	/// for bond gaps, increment ndist
	if(m_dist   < -2.) ++m_nMissing;
      }
    }
  } else setDeadRadLength(m_parametersPredForward);


  /// Noise production
  /// If the track passes through our element
  if(m_inside<=0) {
    /// if we have a cluster, update noise based on the updated parameters
    if(m_cluster) noiseProduction(1,m_parametersUpdatedForward);
    /// otherwise, update noise based on the predicted parameters
    else          noiseProduction(1,m_parametersPredForward);
  }
  /// otherwise, reset noise 
  else            {
    noiseInitiate();
  }
  m_status       = 1;
  m_nlinksForward      = 0;
  m_clusterNoAdd = nullptr;
  return true;
}

///////////////////////////////////////////////////////////////////
// Propagate information in forward direction without closest
// clusters search
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::ForwardPropagationWithoutSearchPreciseWithCorrection
(InDet::SiTrajectoryElement_xk& TE, const EventContext& ctx)
{
  Trk::PatternTrackParameters P;

  if(TE.m_cluster) {
    P = TE.m_parametersUpdatedForward;
    m_dholesForward = 0;
  }
  else             {
    P = TE.m_parametersPredForward;
    m_dholesForward = TE.m_dholesForward;
  }
  m_invMoment = TE.m_invMoment;

  // Track propagation
  //
  P.addNoise(TE.m_noise,Trk::alongMomentum);
  if(!propagate(P,m_parametersPredForward,m_step,ctx)) return false;

  m_nclustersForward = TE.m_nclustersForward;
  m_nholesForward    = TE.m_nholesForward   ;
  m_nMissing         = TE.m_nMissing        ;
  m_ndfForward       = TE.m_ndfForward      ;
  m_xi2totalForward  = TE.m_xi2totalForward ;
  m_step      += TE.m_step      ;
  m_inside     = -1             ;

  // Track update
  //
  if(!m_tools->isITkGeometry() || m_detelement){
    if( m_cluster) {
      if(!addClusterPreciseWithCorrection(m_parametersPredForward,m_parametersPredForward,m_parametersUpdatedForward,m_xi2Forward)) return false;
      ++m_nclustersForward; m_xi2totalForward+=m_xi2Forward; m_ndfForward+=m_ndf;
    }
    else           {
      if( m_detstatus >=0) {
        ++m_nholesForward; ++m_dholesForward; ++m_nMissing;
      }
    }
  }
  else setDeadRadLength(m_parametersPredForward);

  // Noise production
  //
  m_radlength = .04;
  noiseProduction(1,m_parametersUpdatedBackward,-1, true);
  m_status       = 1;
  m_nlinksForward = 0;
  m_clusterNoAdd = nullptr;
  return true;
}

///////////////////////////////////////////////////////////////////
// Propagate information in forward direction with closest 
// clusters search
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::ForwardPropagationWithSearch
(InDet::SiTrajectoryElement_xk& TE, const EventContext& ctx)
{
  /// Track propagation
  /// as usual, if the previous element has a cluster, propagate the updated parameters.
  /// otherwise the predicted ones. 
  if(TE.m_cluster) {

    TE.m_parametersUpdatedForward.addNoise   (TE.m_noise,Trk::alongMomentum);
    if(!propagate(TE.m_parametersUpdatedForward,m_parametersPredForward,m_step,ctx)) return false; 
    TE.m_parametersUpdatedForward.removeNoise(TE.m_noise,Trk::alongMomentum);
    /// double hole running counter is reset if the prev element has a cluster
    m_dholesForward = 0;
  }
  else             {

    TE.m_parametersPredForward.addNoise   (TE.m_noise,Trk::alongMomentum);
    if(!propagate(TE.m_parametersPredForward,m_parametersPredForward,m_step,ctx)) return false; 
    TE.m_parametersPredForward.removeNoise(TE.m_noise,Trk::alongMomentum);
    m_dholesForward = TE.m_dholesForward;
  }

  /// reset old fwd propagation info
  m_status             =         1                           ;  /// note the socially distant semicolons! 
  m_nlinksForward      =         0                           ;
  /// current cluster --> old cluster, reset current 
  m_clusterOld   = m_cluster                                 ;
  m_cluster      =         nullptr                           ;
  m_clusterNoAdd =         nullptr                           ;  
  m_xi2Forward         =    10000.                           ;

  /// copy running counts from previous element
  m_nholesForward      = TE.m_nholesForward                        ;
  m_nMissing           = TE.m_nMissing                             ;
  m_nclustersForward   = TE.m_nclustersForward                     ; 
  m_ndfForward         = TE.m_ndfForward                           ;
  m_xi2totalForward    = TE.m_xi2totalForward                      ;
  m_step              += TE.m_step                                 ;

  if(m_tools->isITkGeometry() && !m_detelement) {
    setDeadRadLength(m_parametersPredForward);
    noiseProduction(1,m_parametersPredForward);
    return true;
  }

  /// re-evaluate if we are expected to intersect this element based on the updated prediction
  checkBoundaries(m_parametersPredForward);
  
  /// if we are not passing through this element, reset noise production and return 
  if(m_inside > 0) {
    noiseInitiate(); return true;
  }
  
  /// now perform cluster search.
  m_nlinksForward=searchClusters(m_parametersPredForward,m_linkForward);  
  /// if we found any: 
  if(m_nlinksForward!=0) {
    /// set chi2 to the one for the first candidate
    m_xi2Forward =  m_linkForward[0].xi2();

    /// if the chi2 is acceptable: 
    if     (m_xi2Forward <= m_xi2max    ) {
      /// use first cluster as cluster on this element 
      m_cluster = m_linkForward[0].cluster();
      /// try to add it to the trajectory, update our parameters
      if(!addCluster(m_parametersPredForward,m_parametersUpdatedForward)) return false;
      /// update noise based on this cluster
      noiseProduction(1,m_parametersUpdatedForward);
      /// increment running cluster count for forward trajectory
      ++m_nclustersForward; 
      /// increment total chi2 for forward trajectory
      m_xi2totalForward+=m_xi2Forward; 
      /// and the degrees of freedom as well 
      m_ndfForward+=m_ndf;
    }
    /// if we have an outlier: 
    else if(m_xi2Forward <= m_xi2maxNoAdd) { 
      /// update outlier cluster member
      m_clusterNoAdd =  m_linkForward[0].cluster();
      /// and reset noise 
      noiseProduction(1,m_parametersPredForward);
    }
  } /// end of branch for found clusters
  else {
    /// reset noise if nothing was found
    noiseProduction(1,m_parametersPredForward);
  }
  /// if we are in an active module (with nonzero clusters) and didn't find anything: 
  if(m_detstatus >=0 && !m_cluster) {
    /// if we expect a hit, and didn't see an outlier: increment hole counts
    if(m_inside < 0 && !m_clusterNoAdd) {
      ++m_nholesForward; 
      ++m_dholesForward;
    } 
    /// if we are well within bounds, increment nDist (may be bond-gap case)
    if(m_dist < -2. ) ++m_nMissing;
  } /// end of case covering no found cluster
  return true;
}

///////////////////////////////////////////////////////////////////
// Backward propagation for filter
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::BackwardPropagationFilter
(InDet::SiTrajectoryElement_xk& TE, const EventContext& ctx)
{
  // Track propagation
  // 
  if(TE.m_noise.correctionIMom() < 1.) {

    if(TE.m_cluster) {

      TE.m_parametersUpdatedBackward.addNoise   (TE.m_noise,Trk::oppositeMomentum);
      if(!propagate(TE.m_parametersUpdatedBackward,m_parametersPredBackward,m_step,ctx)) return false; 
      TE.m_parametersUpdatedBackward.removeNoise(TE.m_noise,Trk::oppositeMomentum);
      m_dholesBackward = 0;
    }
    else             {

      TE.m_parametersPredBackward.addNoise   (TE.m_noise,Trk::oppositeMomentum);
      if(!propagate(TE.m_parametersPredBackward,m_parametersPredBackward,m_step,ctx)) return false; 
      TE.m_parametersPredBackward.removeNoise(TE.m_noise,Trk::oppositeMomentum);
      m_dholesBackward = TE.m_dholesBackward;
    }
  }
  else                                  {

    if(TE.m_cluster) {

      if(!propagate(TE.m_parametersUpdatedBackward,m_parametersPredBackward,m_step,ctx)) return false; 
      m_dholesBackward = 0;
    }
    else             {

      if(!propagate(TE.m_parametersPredBackward,m_parametersPredBackward,m_step,ctx)) return false; 
      m_dholesBackward = TE.m_dholesBackward;
    }
  }
  m_status              = 2;
  m_nlinksBackward      = 0;
  m_clusterOld          = m_cluster;
  m_cluster             = nullptr;
  m_clusterNoAdd        = nullptr;
  m_xi2Backward         = 10000.;
  m_nholesBackward      = TE.m_nholesBackward;
  m_nMissing            = TE.m_nMissing;
  m_nclustersBackward   = TE.m_nclustersBackward;
  m_npixelsBackward     = TE.m_npixelsBackward;
  m_ndfBackward         = TE.m_ndfBackward;
  m_xi2totalBackward    = TE.m_xi2totalBackward;
  m_step               += TE.m_step;

  if(m_tools->isITkGeometry() && !m_detelement) {
    setDeadRadLength(m_parametersPredBackward);
    noiseProduction(-1,m_parametersPredBackward);
    return true;
  }
  checkBoundaries(m_parametersPredBackward);

  if(m_inside >0 ) {noiseInitiate(); return true;}
  
  if((m_nlinksBackward = searchClusters(m_parametersPredBackward,m_linkBackward))) {

    m_xi2Backward =  m_linkBackward[0].xi2();
    
    if     (m_xi2Backward <= m_xi2max     ) {
      
      m_cluster = m_linkBackward[0].cluster();
      if(!addCluster(m_parametersPredBackward,m_parametersUpdatedBackward)) return false;
      noiseProduction(-1,m_parametersUpdatedBackward);
      ++m_nclustersBackward; m_xi2totalBackward+=m_xi2Backward; m_ndfBackward+=m_ndf; if(m_ndf==2) ++m_npixelsBackward;
    }
    else if(m_xi2Backward <= m_xi2maxNoAdd) { 
      
      m_clusterNoAdd =  m_linkBackward[0].cluster();
      noiseProduction(-1,m_parametersPredBackward);
    }
  }
  else {
    noiseProduction(-1,m_parametersPredBackward);
  }

  if(m_detstatus >=0 && !m_cluster){
    if(m_inside < 0 && !m_clusterNoAdd) {++m_nholesBackward; ++m_dholesBackward;}
    if(m_dist < -2.) ++m_nMissing;
  }
  return true;
}

///////////////////////////////////////////////////////////////////
// Backward propagation for smoother  
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::BackwardPropagationSmoother
(InDet::SiTrajectoryElement_xk& TE,bool isTwoSpacePointsSeed, const EventContext& ctx)
{

  // Track propagation
  //
  double step;
  if(TE.m_cluster) {
    if(!propagate(TE.m_parametersUpdatedBackward,m_parametersPredBackward,step,ctx)) return false;
    m_dholesBackward = 0;
  }
  else             {
    if(!propagate(TE.m_parametersPredBackward,m_parametersPredBackward,step,ctx)) return false;
    m_dholesBackward = TE.m_dholesBackward;
  }

  m_parametersPredBackward.addNoise(m_noise,Trk::oppositeMomentum);
  m_nholesBackward      = TE.m_nholesBackward;
  m_nMissing            = TE.m_nMissing;
  m_nclustersBackward   = TE.m_nclustersBackward;
  m_npixelsBackward     = TE.m_npixelsBackward;
  m_ndfBackward         = TE.m_ndfBackward;
  m_xi2totalBackward    = TE.m_xi2totalBackward;

  // remove case if you have trajectory element without actual detector element
  // this happens if you have added a dead cylinder
  if(!m_detelement) {
    m_status = 2;
    return true;
  }

  // Forward-backward predict parameters
  //
  if(!combineStates(m_parametersPredBackward,m_parametersPredForward,m_parametersSM)) return false;

  m_cluster ? m_status = 3 : m_status = 2;

  double Xi2max = m_xi2max; if( isTwoSpacePointsSeed) Xi2max*=2.;
  checkBoundaries(m_parametersSM); 
  m_nlinksBackward      = 0;
  m_clusterOld          = m_cluster;
  m_cluster             = nullptr;
  m_clusterNoAdd        = nullptr;
  m_xi2Backward         = 10000.;

  //m_step        += TE.m_step               ;
  if(m_inside> 0 ) return true;


  // For not first cluster on trajectory
  //
  if(!m_clusterOld || m_ndfForward != m_ndf || m_ndfForward+m_ndfBackward >6) {

    if((m_nlinksBackward = searchClusters(m_parametersSM,m_linkBackward))) {
    
      m_xi2Backward = m_linkBackward[0].xi2();
      
      
      if     (m_xi2Backward <= Xi2max) {

	m_cluster = m_linkBackward[0].cluster();

	if(!addCluster(m_parametersPredBackward,m_parametersUpdatedBackward)) return false;
	++m_nclustersBackward; m_xi2totalBackward+=m_xi2Backward; m_ndfBackward+=m_ndf; if(m_ndf==2) ++m_npixelsBackward;
      }
      else if(m_xi2Backward <= m_xi2maxNoAdd) {
	
	m_clusterNoAdd = m_linkBackward[0].cluster();
      }
    }
    if(m_detstatus >=0 && !m_cluster) {
      if(m_inside < 0 && !m_clusterNoAdd) {++m_nholesBackward; ++m_dholesBackward;}
      if(m_dist < -2.) ++m_nMissing;
    } 
    return true;
  }

  // For first cluster of short trajectory
  //
  m_cluster =  m_clusterOld;
  if(!addCluster(m_parametersPredBackward,m_parametersUpdatedBackward,m_xi2Backward)) return false;

  if     (m_xi2Backward <= Xi2max       ) {++m_nclustersBackward; m_xi2totalBackward+=m_xi2Backward; m_ndfBackward+=m_ndf; if(m_ndf==2) ++m_npixelsBackward;}
  else if(m_xi2Backward <= m_xi2maxNoAdd) {m_cluster      = nullptr; m_clusterNoAdd = m_clusterOld; }
  else                                      {m_cluster      = nullptr;                       }

  if(!m_cluster) {
    if(m_inside < 0 && !m_clusterNoAdd) {++m_nholesBackward; ++m_dholesBackward;}
    if(m_dist < -2.) ++m_nMissing;

  }
  return true;
}

///////////////////////////////////////////////////////////////////
// Backward propagation with precise information
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::BackwardPropagationPrecise
(InDet::SiTrajectoryElement_xk& TE, const EventContext& ctx)
{

  // Track propagation
  //
  double step;
  if(TE.m_cluster) {

    if(!propagate(TE.m_parametersUpdatedBackward,m_parametersPredBackward,step,ctx)) return false;
  }
  else             {

    if(!propagate(TE.m_parametersPredBackward,m_parametersPredBackward,step,ctx)) return false;
  }
  
  m_parametersPredBackward.addNoise(m_noise,Trk::oppositeMomentum);

  // Forward-backward predict parameters
  //
  if(m_cluster) {
    m_status = 3 ;
     if(!addClusterPrecise(m_parametersPredBackward,m_parametersUpdatedBackward,m_xi2Backward)) return false;
  }
  else          {
    m_status = 2 ;
  }
  return true;
}

///////////////////////////////////////////////////////////////////
// Add next cluster for backward propagation
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::addNextClusterB()
{
  if(m_nlinksBackward <= 0) return false; 

  if(m_cluster) m_xi2totalBackward-=m_xi2Backward;

  if(m_nlinksBackward > 1 && m_linkBackward[1].xi2() <= m_xi2max) {
  
    int n = 0; 
    for(; n!=m_nlinksBackward-1; ++n) m_linkBackward[n]=m_linkBackward[n+1];
    m_nlinksBackward=n;

    m_cluster   = m_linkBackward[0].cluster();
    m_xi2Backward      = m_linkBackward[0].xi2()    ;
    m_xi2totalBackward+= m_xi2Backward              ;
    if(!addCluster(m_parametersPredBackward,m_parametersUpdatedBackward)) return false;
  }
  else {
    m_nlinksBackward = 0;
    m_cluster = nullptr; --m_nclustersBackward; m_ndfBackward-=m_ndf; if(m_ndf==2) --m_npixelsBackward;
    if(m_inside < 0 ) {++m_nholesBackward; ++m_dholesBackward;} m_xi2Backward = 0.;
    if(m_dist   < -2.) ++m_nMissing;
  }
  return true;
} 

///////////////////////////////////////////////////////////////////
// Add next cluster for forward propagation
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::addNextClusterF()
{
  if(m_nlinksForward <= 0) return false;
 
  if(m_cluster) m_xi2totalForward-=m_xi2Forward;

  if(m_nlinksForward > 1 && m_linkForward[1].xi2() <= m_xi2max) {
  
    int n = 0; 
    for(; n!=m_nlinksForward-1; ++n) m_linkForward[n]=m_linkForward[n+1];
    m_nlinksForward=n;
    
    m_cluster   = m_linkForward[0].cluster();
    m_xi2Forward      = m_linkForward[0].xi2()    ;
    m_xi2totalForward+= m_xi2Forward              ;
    if(!addCluster(m_parametersPredForward,m_parametersUpdatedForward)) return false;
  }
  else            {
    m_nlinksForward = 0;
    m_cluster = nullptr; --m_nclustersForward; m_ndfForward-=m_ndf;
    if(m_inside < 0) {++m_nholesForward; ++m_dholesForward;} m_xi2Forward = 0.;
    if(m_dist   < -2.) ++m_nMissing;
  }
  return true;
} 

///////////////////////////////////////////////////////////////////
// Add next cluster for backward propagation
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::addNextClusterB
(InDet::SiTrajectoryElement_xk& TE,const InDet::SiCluster* Cl)
{

  m_clusterOld = m_cluster      ;
  m_cluster    = Cl             ;
  m_nclustersBackward = TE.m_nclustersBackward;
  m_npixelsBackward   = TE.m_npixelsBackward  ;
  m_ndfBackward       = TE.m_ndfBackward      ;
  m_nholesBackward    = TE.m_nholesBackward   ;
  m_nMissing      = TE.m_nMissing     ;
  m_xi2totalBackward  = TE.m_xi2totalBackward ; 

  TE.m_cluster ? m_dholesBackward = 0 : m_dholesBackward = TE.m_dholesBackward;

  if(Cl) {

    if(!addCluster(m_parametersPredBackward,m_parametersUpdatedBackward,m_xi2Backward)) return false;
    m_inside    = -1; 
    noiseProduction(-1,m_parametersUpdatedBackward);
    ++m_nclustersBackward; m_xi2totalBackward+=m_xi2Backward; m_ndfBackward+=m_ndf; if(m_ndf==2) ++m_npixelsBackward;
  }
  else  {

    if(m_inside < 0) {++m_nholesBackward; ++m_dholesBackward;} m_xi2Backward = 0.;
    if(m_dist < -2.) ++m_nMissing;
  }
  return true;
} 

///////////////////////////////////////////////////////////////////
// Add next cluster for forward propagation
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::addNextClusterF
(InDet::SiTrajectoryElement_xk& TE,const InDet::SiCluster* Cl)
{
  /// write the current cluster (if any) into the old cluster slot
  m_clusterOld = m_cluster      ;
  /// and set the current cluster to the one assigned
  m_cluster    = Cl             ;   /// can be null! 
  /// copy running counts from the previous element
  m_nclustersForward = TE.m_nclustersForward;
  m_ndfForward       = TE.m_ndfForward      ;
  m_nholesForward    = TE.m_nholesForward   ;
  m_nMissing      = TE.m_nMissing     ;
  m_xi2totalForward  = TE.m_xi2totalForward ; 
  /// if the previous element has a hit, reset incremental 
  /// hole counter 
  if (TE.m_cluster){
    m_dholesForward = 0;
  }
  /// otherwise start from the running count
  else{
    m_dholesForward = TE.m_dholesForward;
  }
  /// if we have a non-null cluster to add 
  if(Cl) {
    /// add it to the track, and update our parameters using it 
    if(!addCluster(m_parametersPredForward,m_parametersUpdatedForward,m_xi2Forward)) return false;
    /// update m_inside to signify we are on the module
    m_inside    = -1; 
    /// update noise to current parameters
    noiseProduction(1,m_parametersUpdatedForward);
    /// update hit counts, chi2, ndf
    ++m_nclustersForward; 
    m_xi2totalForward+=m_xi2Forward; 
    m_ndfForward+=m_ndf; 
  }
  /// no hit (nullptr cluster)
  else  {
    /// if we are within boundaries, add to hole counts 
    if(m_inside < 0) {
      ++m_nholesForward; 
      ++m_dholesForward;
      } 
      /// no chi2 contribution here
      m_xi2Forward = 0.;
      /// if crossed
    if(m_dist < -2.) ++m_nMissing;
  }
  return true;
} 

///////////////////////////////////////////////////////////////////
// TrackStateOnSurface production 
///////////////////////////////////////////////////////////////////

Trk::TrackStateOnSurface*  
InDet::SiTrajectoryElement_xk::trackStateOnSurface (bool change,bool cov,bool multi,int Q)
{
  std::unique_ptr<Trk::TrackParameters> tp = nullptr;
  if (!change) {
    tp = trackParameters(cov, Q);
  } else {
    tp = trackParametersWithNewDirection(cov, Q);
  }
  if (!tp) {
    return nullptr;
  }
  if (&tp->associatedSurface() != m_surface) {
    return nullptr;
  }

  Trk::FitQualityOnSurface fq{};
  std::unique_ptr<Trk::MeasurementBase> ro{};

  if (m_status == 1) {
    fq = Trk::FitQualityOnSurface(m_xi2Forward, m_ndf);
  } else {
    fq = Trk::FitQualityOnSurface(m_xi2Backward, m_ndf);
  }

  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> pat(
    0);

  if (m_cluster) {
    ro.reset(m_riotool->correct(*m_cluster, *tp));
    pat.set(Trk::TrackStateOnSurface::Measurement);
  } else {
    ro.reset(m_riotool->correct(*m_clusterNoAdd, *tp));
    pat.set(Trk::TrackStateOnSurface::Outlier);
  }
  auto sa = Trk::ScatteringAngles(
    0., 0., std::sqrt(m_noise.covarianceAzim()), std::sqrt(m_noise.covariancePola()));

  auto meTemplate = std::make_unique<Trk::MaterialEffectsOnTrack>(
    m_radlengthN, sa, tp->associatedSurface());

  pat.set(Trk::TrackStateOnSurface::Scatterer);
  Trk::TrackStateOnSurface* sos =
    new Trk::TrackStateOnSurface(fq, std::move(ro), std::move(tp), meTemplate->uniqueClone(), pat);

  m_tsos[0] = sos;
  m_utsos[0] = true;
  m_ntsos = 1;

  if (multi && m_cluster && m_ndf == 2 && m_nlinksBackward > 1) {
    for(int i=1; i!= m_nlinksBackward; ++i) {
      if(m_linkBackward[i].xi2() > m_xi2multi) break;
      std::unique_ptr<Trk::TrackParameters> tpn{};
      if (!change) {
        tpn = trackParameters(cov, Q);
      } else {
        tpn = trackParametersWithNewDirection(cov, Q);
      }
      if (!tpn){
        break;
      }
      auto fqn = Trk::FitQualityOnSurface(m_linkBackward[i].xi2(),m_ndf);
      std::unique_ptr<Trk::MeasurementBase> ron(m_riotool->correct(
        *m_linkBackward[i].cluster(), *(sos->trackParameters())) );
      m_tsos[m_ntsos] = new Trk::TrackStateOnSurface(
        fqn, std::move(ron), std::move(tpn), meTemplate->uniqueClone(), pat);
      m_utsos[m_ntsos] = false;
      if(++m_ntsos == 3) break;
    }
  }
  return sos;
}

///////////////////////////////////////////////////////////////////
// TrackStateOnSurface production for simple track
///////////////////////////////////////////////////////////////////

Trk::TrackStateOnSurface*  
InDet::SiTrajectoryElement_xk::trackSimpleStateOnSurface 
(bool change,bool cov,int Q)
{
  if(!m_detelement) {
    return nullptr;
  }

  std::unique_ptr<Trk::TrackParameters> tp = nullptr;

  if (Q) {
    if (!change) {
      tp = trackParameters(cov, Q);
    } else {
      tp = trackParametersWithNewDirection(cov, Q);
    }
    if (!tp) {
      return nullptr;
    }
    if (&tp->associatedSurface() != m_surface) {
       return nullptr;
    }
  }

  IdentifierHash iH = m_detelement->identifyHash();
  std::unique_ptr<Trk::MeasurementBase> ro{};
  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes> pat(
    0);

  const InDet::SiCluster* cl = nullptr;
  if (m_cluster) {
    cl = m_cluster;
    pat.set(Trk::TrackStateOnSurface::Measurement);
  } else {
    cl = m_clusterNoAdd;
    pat.set(Trk::TrackStateOnSurface::Outlier);
  }
  pat.set(Trk::TrackStateOnSurface::Scatterer);

  Trk::LocalParameters locp = Trk::LocalParameters(cl->localPosition());
  Amg::MatrixX cv = cl->localCovariance();

  Trk::FitQualityOnSurface fq{};
  if     (m_status == 1) fq = Trk::FitQualityOnSurface(m_xi2Forward,m_ndf);
  else                   fq = Trk::FitQualityOnSurface(m_xi2Backward,m_ndf);

  if (m_ndf == 1) {
    const InDet::SCT_Cluster* sc = static_cast<const InDet::SCT_Cluster*>(cl);
    if (sc)
      ro = std::make_unique<InDet::SCT_ClusterOnTrack>(sc, locp, cv, iH, sc->globalPosition());
  } else {

    const InDet::PixelCluster* pc = static_cast<const InDet::PixelCluster*>(cl);
    if (pc)
      ro = std::make_unique<InDet::PixelClusterOnTrack>(
        pc, locp, cv, iH, pc->globalPosition(), pc->gangedPixel());
  }
  return new Trk::TrackStateOnSurface(fq, std::move(ro), std::move(tp), nullptr, pat);
}

///////////////////////////////////////////////////////////////////
// TrackStateOnSurface production for perigee
///////////////////////////////////////////////////////////////////

Trk::TrackStateOnSurface*  
InDet::SiTrajectoryElement_xk::trackPerigeeStateOnSurface (const EventContext& ctx)
{
  if(&m_parametersUpdatedBackward.associatedSurface()!=m_surface) return nullptr;
  
  double step                   ;
  Trk::PatternTrackParameters Tp; 

  Trk::PerigeeSurface per;

  bool Q = m_proptool->propagate
	(ctx,
   m_parametersUpdatedBackward, per,Tp,Trk::anyDirection,m_tools->fieldTool(),step,Trk::pion);

  if(Q) {
    std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>  typePattern;
    typePattern.set(Trk::TrackStateOnSurface::Perigee);
    return new Trk::TrackStateOnSurface(nullptr,Tp.convert(true),nullptr,typePattern);
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////
// TrackParameters production
// Q = 0 no first or last  element of the trajectory
// Q = 1             first element of the trajectory
// Q = 2             last  element of the trajectory
///////////////////////////////////////////////////////////////////

std::unique_ptr<Trk::TrackParameters>
InDet::SiTrajectoryElement_xk::trackParameters(bool cov, int Q)
{
  if (m_status == 1) {
    if (m_cluster) {
      return m_parametersUpdatedForward.convert(cov);
    } else {
      return m_parametersPredForward.convert(cov);
    }
  } else if (m_status == 2) {
    if (m_cluster) {
      return m_parametersUpdatedBackward.convert(cov);
    } else {
      return m_parametersPredBackward.convert(cov);
    }
  } else if (m_status == 3) {
    if (Q == 0) {
      if (m_cluster) {
        if (addCluster(m_parametersSM, m_parametersSM)) {
          return m_parametersSM.convert(cov);
        } else if ((*m_parametersUpdatedBackward.covariance())(4, 4) <
                   (*m_parametersPredForward.covariance())(4, 4)) {
          return m_parametersUpdatedBackward.convert(cov);
        } else {
          return m_parametersPredForward.convert(cov);
        }
      } else
        return m_parametersSM.convert(cov);
    }
    if (Q == 1) {
      if (m_cluster) {
        return m_parametersUpdatedBackward.convert(cov);
      }
    }
    if (Q == 2) {
      if (m_cluster) {
        return m_parametersUpdatedForward.convert(cov);
      }
    }
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////
// Noise production
// Dir  = +1 along momentum , -1 opposite momentum 
// Model = 1 - muon, 2 - electron 
// useMomentum = true - use m_invMoment instead of Tp.par()[4]
///////////////////////////////////////////////////////////////////

void  InDet::SiTrajectoryElement_xk::noiseProduction
(int Dir,const Trk::PatternTrackParameters& Tp,double rad_length, bool useMomentum)
{

  int Model = m_noisemodel; 
  if(Model < 1 || Model > 2) return; 
  if (rad_length<0.) rad_length=m_radlength;

  double q = useMomentum ? m_invMoment : std::abs(Tp.parameters()[4]);   /// qoverp

  /// projection of direction normal to surface 
  double s = std::abs(m_localDir[0]*m_localTransform[6]+
		      m_localDir[1]*m_localTransform[7]+
		      m_localDir[2]*m_localTransform[8]);
  if(m_tools->isITkGeometry() && !m_detelement) s = sqrt(m_localDir[0]*m_localDir[0]+m_localDir[1]*m_localDir[1]);

  if(m_tools->isITkGeometry()){
    if (s < .01) s = 100.;
    else s = 1./s;
  }
  else{
    if (s  < .05) s = 20.;
    else s = 1./s;
  }

  /// number of radiation lengths when crossing this surface
  m_radlengthN = s*m_radlength; 
  /// 134 is a very magic number related to multiple scattering, 
  /// used frequently throught the track finder chain. 
  /// Here we use it to estimate the scattering impact on the errors
  double covariancePola = (134.*m_radlengthN)*(q*q);
  if(m_tools->isITkGeometry()){
    double qc      = (1.+.038*log(m_radlengthN))*q;
    covariancePola = (185.*m_radlengthN)*qc*qc;
  }

  /// 1 - Az² --> squared transverse direction component 
  double d = (1.-m_localDir[2])*(1.+m_localDir[2]);   
  /// if too small, set to a reasonable minimum 
  if(d < 1.e-5) d = 1.e-5;
  /// azimuthal component: scale with 1/d 
  double covarianceAzim = covariancePola/d;

  double covarianceIMom;
  double correctionIMom;

  /// muon model 
  if(Model==1) {
    /// estimate e-loss
    double        dp = m_energylose*q*s;
    /// impact on covariance
    covarianceIMom = (.2*dp*dp)*(q*q);
    /// impact on momentum
    correctionIMom = 1.-dp;
  }
  /// electron model - much more generous 
  else {
    correctionIMom = .70;
    covarianceIMom = (correctionIMom-1.)*(correctionIMom-1.)*(q*q);
  }
  /// for forward direction, invert correction
  if(Dir>0) correctionIMom = 1./correctionIMom;
  /// store in noise object
  m_noise.set(covarianceAzim,covariancePola,covarianceIMom,correctionIMom);
}


///////////////////////////////////////////////////////////////////
// TrackParameters production with new direction
// Q = 0 no first or last  element of the trajectory
// Q = 1             first element of the trajectory
// Q = 2             last  element of the trajectory
///////////////////////////////////////////////////////////////////

std::unique_ptr<Trk::TrackParameters>
InDet::SiTrajectoryElement_xk::trackParametersWithNewDirection(bool cov, int Q)
{
  if (m_status == 1) {
    if (m_cluster) {
      return trackParameters(m_parametersUpdatedForward, cov);
    } else {
      return trackParameters(m_parametersPredForward, cov);
    }

  } else if (m_status == 2) {
    if (m_cluster) {
      return trackParameters(m_parametersUpdatedBackward, cov);
    } else {
      return trackParameters(m_parametersPredBackward, cov);
    }
  } else if (m_status == 3) {

    if (Q == 0) {
      if (m_cluster) {
        if (addCluster(m_parametersSM, m_parametersSM))
          return trackParameters(m_parametersSM, cov);
        else if ((*m_parametersUpdatedBackward.covariance())(4, 4) <
                 (*m_parametersPredForward.covariance())(4, 4)) {
          return trackParameters(m_parametersUpdatedBackward, cov);
        } else {
          return trackParameters(m_parametersPredForward, cov);
        }
      } else {
        return trackParameters(m_parametersSM, cov);
      }
    }
    if (Q == 1) {
      if (m_cluster){
        return trackParameters(m_parametersUpdatedBackward, cov);
      }
    }
    if (Q == 2) {
      if (m_cluster){
        return trackParameters(m_parametersUpdatedForward, cov);
      }
    }
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////
// TrackParameters production with new direction
///////////////////////////////////////////////////////////////////
std::unique_ptr<Trk::TrackParameters>
InDet::SiTrajectoryElement_xk::trackParameters
(Trk::PatternTrackParameters& Tp,bool cov)
{
  Tp.changeDirection();
  return Tp.convert(cov);
}


///////////////////////////////////////////////////////////////////
// Step calculation 
///////////////////////////////////////////////////////////////////

double InDet::SiTrajectoryElement_xk::step
(InDet::SiTrajectoryElement_xk& TE)
{
  Trk::PatternTrackParameters Ta,Tb; 

  if    (TE.m_status == 1) {
    if(TE.m_cluster) Ta = TE.m_parametersUpdatedForward;
    else             Ta = TE.m_parametersPredForward;
  }
  else if(TE.m_status == 2) {
    if(TE.m_cluster) Ta = TE.m_parametersUpdatedBackward;
    else             Ta = TE.m_parametersPredBackward;
  }
  else if(TE.m_status == 3) {
    Ta = TE.m_parametersSM;
  }
  double step = 0.;
  bool   Q    = propagateParameters(Ta,Tb,step);
  if(Q) return step;
  return 0.;
}

///////////////////////////////////////////////////////////////////
// Global position of track parameters
///////////////////////////////////////////////////////////////////

Amg::Vector3D InDet::SiTrajectoryElement_xk::globalPosition()
{
  if    (m_status == 1) {
    if(m_cluster)  return m_parametersUpdatedForward.position();
    else           return m_parametersPredForward.position();
  }
  else if(m_status == 2) {
    if(m_cluster)  return m_parametersUpdatedBackward.position();
    else           return m_parametersPredBackward.position();
  }
  else if(m_status == 3) {

    Amg::Vector3D gp(0.,0.,0.);
    bool QA; Trk::PatternTrackParameters S1,SM,S2(parametersPF()); 
    
    if(m_cluster) S1 = parametersUB();
    else          S1 = parametersPB();
    
    QA = m_updatorTool->combineStates(S1,S2,SM);
    
    if(QA) {
      gp = SM.position();
    }
    return gp;
  }
  Amg::Vector3D gp(0.,0.,0.);
  return gp;
}

///////////////////////////////////////////////////////////////////
// Step estimation to 0,0,0
///////////////////////////////////////////////////////////////////

double InDet::SiTrajectoryElement_xk::stepToPerigee()
{
  Amg::Vector3D M;
  Amg::Vector3D P; 
   
  if(m_cluster) {
    M = m_parametersUpdatedBackward.momentum();
    P = m_parametersUpdatedBackward.position();
  }  
  else          {
    M = m_parametersPredBackward.momentum(); 
    P = m_parametersPredBackward.position();
  }
  
  return -(P[0]*M[0]+P[1]*M[1]);
}

///////////////////////////////////////////////////////////////////
// Errase cluster fo forward propagation
///////////////////////////////////////////////////////////////////

void InDet::SiTrajectoryElement_xk::eraseClusterForwardPropagation()
{
  m_cluster=nullptr; 
  --m_nclustersForward; 
  m_xi2totalForward-=m_xi2Forward; 
  m_ndfForward-=m_ndf;
}

///////////////////////////////////////////////////////////////////
// Quality of the trajectory element
///////////////////////////////////////////////////////////////////

double InDet::SiTrajectoryElement_xk::quality(int& holes) const
{

  if(!m_cluster && !m_clusterNoAdd) {
    
    if(m_detstatus < 0) return 0.;
    
    if     (m_inside <  0) {
      double w = 2.-m_xi2max; if(++holes > 1) w*=2.; return w;
    }
    else if(m_inside == 0) return -1.;
    else                   return  0.;
  }

  double w,X,Xc = m_xi2max+2.;
  m_status == 1 ? X = m_xi2Forward        : X = m_xi2Backward;
  m_ndf    == 2 ? w = 1.2*(Xc-X*.5) : w = Xc-X  ; if(w < -1.) w = -1.;
  holes    = 0;

  return w;
}

/////////////////////////////////////////////////////////////////////////////////
// Main function for pattern track parameters and covariance matrix propagation
// to PlaneSurface.
/// Start from 'startingParameters', propagate to current surface. 
/// Write into outputParameters 
/// and wrote step to surface into StepLength
/////////////////////////////////////////////////////////////////////////////////
bool 
InDet::SiTrajectoryElement_xk::propagate(Trk::PatternTrackParameters  & startingParameters,
                                         Trk::PatternTrackParameters  & outputParameters,
                                         double & StepLength,
                                         const EventContext& ctx ) {
  if (Trk::SurfaceType::Plane == m_surface->type() and
    Trk::SurfaceType::Plane == startingParameters.associatedSurface().type()) {
    bool useJac = (startingParameters.iscovariance());
    double globalParameters[64];    /// helper field to store global frame parameters
                                    /// Convention:
                                    /// 0-2: Position
                                    /// 3-5: Direction
                                    /// 6: qoverp
                                    /// 7 - 41: jacobian
                                    /// 42-44: extension of jacobian for dA/ds
                                    /// 45: step length

    /// transform starting parameters to global, write into globalParameters
    if(!transformPlaneToGlobal(useJac,startingParameters,globalParameters)) return false;
    /// if running with field, use Runge Kutta. Will update globalParameters to the result in global coordinates
    if( m_fieldMode) {
      if(!rungeKuttaToPlane      (useJac,globalParameters)) return false;
    }
    /// otherwise, we can use the straight line
    else {
      if(!straightLineStepToPlane(useJac,globalParameters)) return false;
    }
    /// Update step length
    StepLength = globalParameters[45];
    /// and finally transform from global back to local, and write into the output parameters.
    return transformGlobalToPlane(useJac,globalParameters,startingParameters,outputParameters);
  } else {
    if (!m_proptool->propagate (ctx,
                                startingParameters, *m_surface, outputParameters,
                                Trk::anyDirection, m_tools->fieldTool(), StepLength, Trk::pion))
      return false;

    double sinPhi,cosPhi,sinTheta,cosTheta;
    sincos(outputParameters.parameters()[2],&sinPhi,&cosPhi);
    sincos(outputParameters.parameters()[3],&sinTheta,&cosTheta);
    m_localDir[0] = cosPhi*sinTheta;
    m_localDir[1] = sinPhi*sinTheta;
    m_localDir[2] =    cosTheta;
    return true;
  }
}

/////////////////////////////////////////////////////////////////////////////////
// Main function for pattern track parameters propagation without covariance. 
/// Start from 'startingParameters', propagate to current surface. 
/// Write into outputParameters 
/// and wrote step to surface into StepLength
/////////////////////////////////////////////////////////////////////////////////

bool 
InDet::SiTrajectoryElement_xk::propagateParameters(Trk::PatternTrackParameters  & startingParameters,
                                                   Trk::PatternTrackParameters  & outputParameters, 
                                                   double  & StepLength ) {
  bool useJac = false;
  double globalParameters[64];    /// helper field to store global frame parameters
                                  /// Convention: 
                                  /// 0-2: Position
                                  /// 3-5: Direction 
                                  /// 6: qoverp
                                  /// 7 - 41: jacobian 
                                  /// 42-44: extension of jacobian for dA/ds
                                  /// 45: step length 
  if(!transformPlaneToGlobal(useJac,startingParameters,globalParameters)) return false;
  /// if running with field, use Runge Kutta. 
  /// Will update globalParameters to the result in global coordinates
  if( m_fieldMode) {
    if(!rungeKuttaToPlane (useJac,globalParameters)) return false;
  } 
  /// otherwise, we can use the straight line 
  else {
    if(!straightLineStepToPlane(useJac,globalParameters)) return false;
  } 
  /// Update step length 
  StepLength = globalParameters[45]; 
  /// and finally transform from global back to local, and write into the output parameters.
  return transformGlobalToPlane(useJac,globalParameters,startingParameters,outputParameters);
}

/////////////////////////////////////////////////////////////////////////////////
/// Tramsform from plane to global
/// Will take the surface and parameters from localParameters
/// and populate globalPars with the result. 
/// globalPars needs to be at least 46 elements long. 
/// Convention: 
///                       /dL0    /dL1    /dPhi   /dThe   /dCM
///    X  ->globalPars[0]  dX /   globalPars[ 7]   globalPars[14]   globalPars[21]   globalPars[28]   globalPars[35]  
///    Y  ->globalPars[1]  dY /   globalPars[ 8]   globalPars[15]   globalPars[22]   globalPars[29]   globalPars[36]  
///    Z  ->globalPars[2]  dZ /   globalPars[ 9]   globalPars[16]   globalPars[23]   globalPars[30]   globalPars[37]   
///    Ax ->globalPars[3]  dAx/   globalPars[10]   globalPars[17]   globalPars[24]   globalPars[31]   globalPars[38]  
///    Ay ->globalPars[4]  dAy/   globalPars[11]   globalPars[18]   globalPars[25]   globalPars[32]   globalPars[39]  
///    Az ->globalPars[5]  dAz/   globalPars[12]   globalPars[19]   globalPars[26]   globalPars[33]   globalPars[40]  
///    CM ->globalPars[6]  dCM/   globalPars[13]   globalPars[20]   globalPars[27]   globalPars[34]   globalPars[41] 
// /////////////////////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::transformPlaneToGlobal(bool useJac,
							   Trk::PatternTrackParameters& localParameters,
							   double* globalPars) {
  /// obtain trigonometric functions required for transform                                   
  double sinPhi,cosPhi,cosTheta,sintheta;   
  sincos(localParameters.parameters()[2],&sinPhi,&cosPhi);  
  sincos(localParameters.parameters()[3],&sintheta,&cosTheta);
  if (m_tools->isITkGeometry()) {
    if(std::abs(sintheta) < std::abs(localParameters.parameters()[4])*50.) return false;
  }
  /// get the surface corresponding to the local parameters
  const Trk::Surface* pSurface=&localParameters.associatedSurface();
  if (!pSurface){
    throw(std::runtime_error("TrackParameters associated surface is null pointer in InDet::SiTrajectoryElement_xk::transformPlaneToGlobal"));
  }
  /// get the associated transform from the surface 
  const Amg::Transform3D& T  = pSurface->transform();
 
  /// Get the local x and y axis in the global frame 
  double Ax[3] = {T(0,0),T(1,0),T(2,0)};
  double Ay[3] = {T(0,1),T(1,1),T(2,1)};

  /// position 
  globalPars[ 0] = localParameters.parameters()[0]*Ax[0]+localParameters.parameters()[1]*Ay[0]+T(0,3);                    // X
  globalPars[ 1] = localParameters.parameters()[0]*Ax[1]+localParameters.parameters()[1]*Ay[1]+T(1,3);                    // Y
  globalPars[ 2] = localParameters.parameters()[0]*Ax[2]+localParameters.parameters()[1]*Ay[2]+T(2,3);                    // Z
  /// direction vectors
  globalPars[ 3] = cosPhi*sintheta;                                                         // Ax
  globalPars[ 4] = sinPhi*sintheta;                                                         // Ay
  globalPars[ 5] = cosTheta;          
  /// qoeverp                                                   // Az
  globalPars[ 6] = localParameters.parameters()[4];                                                   // CM
  /// for very high momenta, truncate to avoid zero
  if(std::abs(globalPars[6])<1.e-20) {
    if (globalPars[6] < 0){ 
      globalPars[6]=-1.e-20;
    }
    else globalPars[6]= 1.e-20;
  }    

  /// Update jacobian if requested 
  if(useJac) {

    //     /dL1   |      /dL2    |    /dPhi    |    /dThe     |    /dCM     |
    globalPars[ 7]  = Ax[0]; globalPars[14] = Ay[0]; globalPars[21] =   0.; globalPars[28] =    0.; globalPars[35] =   0.; // dX /
    globalPars[ 8]  = Ax[1]; globalPars[15] = Ay[1]; globalPars[22] =   0.; globalPars[29] =    0.; globalPars[36] =   0.; // dY /
    globalPars[ 9]  = Ax[2]; globalPars[16] = Ay[2]; globalPars[23] =   0.; globalPars[30] =    0.; globalPars[37] =   0.; // dZ /
    globalPars[10]  =    0.; globalPars[17] =    0.; globalPars[24] =-globalPars[4]; globalPars[31] = cosPhi*cosTheta; globalPars[38] =   0.; // dAx/
    globalPars[11]  =    0.; globalPars[18] =    0.; globalPars[25] = globalPars[3]; globalPars[32] = sinPhi*cosTheta; globalPars[39] =   0.; // dAy/
    globalPars[12]  =    0.; globalPars[19] =    0.; globalPars[26] =   0.; globalPars[33] =   -sintheta; globalPars[40] =   0.; // dAz/
   
    /// d(Ax,Ay,Az)/ds 
    globalPars[42]  =    0.; 
    globalPars[43] =    0.; 
    globalPars[44] =   0.;
  }
  /// Step length is initialised to zero - no propagation done yet
  globalPars[45] =   0.;
  return true;
}

/////////////////////////////////////////////////////////////////////////////////
/// Tramsform from global to plane surface
/// Will take the global parameters in globalPars, the surface from this trajectory
/// element and populate outputParameters with the result. 
/// Convention for globalPars: 
///                       /dL0    /dL1    /dPhi   /dThe   /dCM
///    X  ->globalPars[0]  dX /   globalPars[ 7]   globalPars[14]   globalPars[21]   globalPars[28]   globalPars[35]  
///    Y  ->globalPars[1]  dY /   globalPars[ 8]   globalPars[15]   globalPars[22]   globalPars[29]   globalPars[36]  
///    Z  ->globalPars[2]  dZ /   globalPars[ 9]   globalPars[16]   globalPars[23]   globalPars[30]   globalPars[37]   
///    Ax ->globalPars[3]  dAx/   globalPars[10]   globalPars[17]   globalPars[24]   globalPars[31]   globalPars[38]  
///    Ay ->globalPars[4]  dAy/   globalPars[11]   globalPars[18]   globalPars[25]   globalPars[32]   globalPars[39]  
///    Az ->globalPars[5]  dAz/   globalPars[12]   globalPars[19]   globalPars[26]   globalPars[33]   globalPars[40]  
///    CM ->globalPars[6]  dCM/   globalPars[13]   globalPars[20]   globalPars[27]   globalPars[34]   globalPars[41] 
// /////////////////////////////////////////////////////////////////////////////////
bool InDet::SiTrajectoryElement_xk::transformGlobalToPlane
(bool useJac,double* globalPars,Trk::PatternTrackParameters& startingParameters,Trk::PatternTrackParameters& outputParameters) 
{
  /// local x,y,z axes in global frame 
  double Ax[3] = {m_localTransform[0],m_localTransform[1],m_localTransform[2]};
  double Ay[3] = {m_localTransform[3],m_localTransform[4],m_localTransform[5]};
  double Az[3] = {m_localTransform[6],m_localTransform[7],m_localTransform[8]};
  /// distance of global position to origin of local frame
  double d [3] = {globalPars[0]-m_localTransform[ 9],
                  globalPars[1]-m_localTransform[10],
                  globalPars[2]-m_localTransform[11]};

  /// local parameters obtained by transforming from global
  double p[5] = {
     d[0]*Ax[0]+d[1]*Ax[1]+d[2]*Ax[2],            /// local X
		 d[0]*Ay[0]+d[1]*Ay[1]+d[2]*Ay[2],            /// local Y
		 atan2(globalPars[4],globalPars[3]),          /// phi (from global direction x,y) 
		 acos(globalPars[5]),                         /// theta (from global cosTheta)
		 globalPars[6]                                /// qoverp 
  };

  /// update local direction vector
  m_localDir[0] = globalPars[3];
  m_localDir[1] = globalPars[4];
  m_localDir[2] = globalPars[5];

  if (useJac) {
    /// Condition trajectory on surface
    double A  = Az[0]*globalPars[3]+Az[1]*globalPars[4]+Az[2]*globalPars[5];
    if(A!=0.) A=1./A;
    double s0 = Az[0]*globalPars[ 7]+Az[1]*globalPars[ 8]+Az[2]*globalPars[ 9];
    double s1 = Az[0]*globalPars[14]+Az[1]*globalPars[15]+Az[2]*globalPars[16];
    double s2 = Az[0]*globalPars[21]+Az[1]*globalPars[22]+Az[2]*globalPars[23];
    double s3 = Az[0]*globalPars[28]+Az[1]*globalPars[29]+Az[2]*globalPars[30];
    double s4 = Az[0]*globalPars[35]+Az[1]*globalPars[36]+Az[2]*globalPars[37];
    double T0 =(Ax[0]*globalPars[ 3]+Ax[1]*globalPars[ 4]+Ax[2]*globalPars[ 5])*A;
    double T1 =(Ay[0]*globalPars[ 3]+Ay[1]*globalPars[ 4]+Ay[2]*globalPars[ 5])*A;
    double n  = 1./globalPars[6];

    double Jac[21];

    // Jacobian production
    //
    Jac[ 0] = (Ax[0]*globalPars[ 7]+Ax[1]*globalPars[ 8])+(Ax[2]*globalPars[ 9]-s0*T0);    // dL0/dL0
    Jac[ 1] = (Ax[0]*globalPars[14]+Ax[1]*globalPars[15])+(Ax[2]*globalPars[16]-s1*T0);    // dL0/dL1
    Jac[ 2] = (Ax[0]*globalPars[21]+Ax[1]*globalPars[22])+(Ax[2]*globalPars[23]-s2*T0);    // dL0/dPhi
    Jac[ 3] = (Ax[0]*globalPars[28]+Ax[1]*globalPars[29])+(Ax[2]*globalPars[30]-s3*T0);    // dL0/dThe
    Jac[ 4] =((Ax[0]*globalPars[35]+Ax[1]*globalPars[36])+(Ax[2]*globalPars[37]-s4*T0))*n; // dL0/dCM

    Jac[ 5] = (Ay[0]*globalPars[ 7]+Ay[1]*globalPars[ 8])+(Ay[2]*globalPars[ 9]-s0*T1);    // dL1/dL0
    Jac[ 6] = (Ay[0]*globalPars[14]+Ay[1]*globalPars[15])+(Ay[2]*globalPars[16]-s1*T1);    // dL1/dL1
    Jac[ 7] = (Ay[0]*globalPars[21]+Ay[1]*globalPars[22])+(Ay[2]*globalPars[23]-s2*T1);    // dL1/dPhi
    Jac[ 8] = (Ay[0]*globalPars[28]+Ay[1]*globalPars[29])+(Ay[2]*globalPars[30]-s3*T1);    // dL1/dThe
    Jac[ 9] =((Ay[0]*globalPars[35]+Ay[1]*globalPars[36])+(Ay[2]*globalPars[37]-s4*T1))*n; // dL1/dCM

    double P3=0;
    double P4=0;
    /// transverse direction component
    double C = globalPars[3]*globalPars[3]+globalPars[4]*globalPars[4];
    if(C > 1.e-20) {
      C= 1./C ;
      /// unit direction vector in the X,Y plane
      P3 = globalPars[3]*C;
      P4 =globalPars[4]*C;
      C =-sqrt(C);
    }
    else{
      C=-1.e10;
      P3 = 1.;
      P4 =0.;
    }

    double T2  =(P3*globalPars[43]-P4*globalPars[42])*A;
    double C44 = C*globalPars[44]           *A;

    Jac[10] = P3*globalPars[11]-P4*globalPars[10]-s0*T2;    // dPhi/dL0
    Jac[11] = P3*globalPars[18]-P4*globalPars[17]-s1*T2;    // dPhi/dL1
    Jac[12] = P3*globalPars[25]-P4*globalPars[24]-s2*T2;    // dPhi/dPhi
    Jac[13] = P3*globalPars[32]-P4*globalPars[31]-s3*T2;    // dPhi/dThe
    Jac[14] =(P3*globalPars[39]-P4*globalPars[38]-s4*T2)*n; // dPhi/dCM

    Jac[15] = C*globalPars[12]-s0*C44;             // dThe/dL0
    Jac[16] = C*globalPars[19]-s1*C44;             // dThe/dL1
    Jac[17] = C*globalPars[26]-s2*C44;             // dThe/dPhi
    Jac[18] = C*globalPars[33]-s3*C44;             // dThe/dThe
    Jac[19] =(C*globalPars[40]-s4*C44)*n;          // dThe/dCM
    Jac[20] = 1.;                         // dCM /dCM

    /// covariance matrix production using jacobian - CovNEW = J*CovOLD*Jt
    AmgSymMatrix(5) newCov = Trk::RungeKuttaUtils::newCovarianceMatrix(Jac, *startingParameters.covariance());
    outputParameters.setParametersWithCovariance(m_surface, p, newCov);

    /// check for negative diagonals in the cov
    const AmgSymMatrix(5) & t = *outputParameters.covariance();
    if(t(0, 0)<=0. || t(1, 1)<=0. || t(2, 2)<=0. || t(3, 3)<=0. || t(4, 4)<=0.) return false;
  } else {
    /// write into output parameters. Assign our surface to them
    outputParameters.setParameters(m_surface,p);
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////
/// Runge Kutta step to plane
/// Updates the "globalPars" array, which is also used to 
/// pass the input.  
/////////////////////////////////////////////////////////////////////////////////

bool  InDet::SiTrajectoryElement_xk::rungeKuttaToPlane
(bool Jac,double* globalPars)
{
  /// minimum step length
  const double Smin = .1        ;
  /// step length below which we are allowed to use 
  /// a helical approximation
  const double Shel = 5.        ;
  /// tolerance before reducing step length
  const double dlt  = .001      ;

  /// Minimum momentum check 
  if(std::abs(globalPars[6]) > .05) return false;


  int    it    =               0;
  double* R    =          &globalPars[ 0];            // Coordinates 

  double* A    =          &globalPars[ 3];            // Directions

  double* sA   =          &globalPars[42];            /// dA/ds
  double  Pi   =  149.89626*globalPars[6];            /// This is a conversion from p to half-curvature (1/2R), when multiplied with the orthogonal field component
                                                      /// Curvature = 2 * Pi x B x sin(angle)
  double  Pa   = std::abs      (globalPars[6]);           /// abs(qoverp)

  /// projection of global direction onto local Z-axis                 
  double  a    = A[0]*m_localTransform[6]+A[1]*m_localTransform[7]+A[2]*m_localTransform[8]; 
  if(a==0.) return false; 
  /// distance orthogonal to surface in units of direction z-component 
  double  S    = ((m_localTransform[12]-R[0]*m_localTransform[6])-(R[1]*m_localTransform[7]+R[2]*m_localTransform[8]))/a; 
  double  S0   = std::abs(S)                                                ;

  /// if we are sufficiently close in Z already, add a 
  /// final straight line step corresponding to the remaining difference 
  /// to get the estimate on the surface, update step length, and return the result  
  if(S0 <= Smin) {
    R[0]+=(A[0]*S); 
    R[1]+=(A[1]*S); 
    R[2]+=(A[2]*S); 
    globalPars[45]+=S; 
    return true;
  }
  /// if we are too far away, 
  /// Limit the step size to 0.3 * p
  else  if( (Pa*S0) > .3) {
    if (S >0) S = 0.3 / Pa; 
    else S = -0.3/Pa; 
  }

  bool   ste   = false;

  /// holders for the B-field
  double f0[3];
  double f[3];

  /// get the field at our reference point
  m_fieldCache.getFieldZR(R,f0);


  while(true) {

    bool Helix = false; 
    /// if the step is sifficiently small, we are allowed to use a helical model
    if(std::abs(S) < Shel) Helix = true;
    double S3=(1./3.)*S;      /// S/3 
    double S4=.25*S;          /// S/4 
    double PS2=Pi*S;          /// 2 S/curvature radius, when multiplied with the appropriate field 
 
    /// First point
    ///   
    double H0[3] = {f0[0]*PS2,      /// equip field with conversion factor to 2S/curvature
                    f0[1]*PS2, 
                    f0[2]*PS2};

    double A0    = A[1]*H0[2]-A[2]*H0[1]            ;     /// v cross B 
    double B0    = A[2]*H0[0]-A[0]*H0[2]            ;
    double C0    = A[0]*H0[1]-A[1]*H0[0]            ;

    double A2    = A0+A[0]                          ;
    double B2    = B0+A[1]                          ;
    double C2    = C0+A[2]                          ;

    double A1    = A2+A[0]                          ;
    double B1    = B2+A[1]                          ;
    double C1    = C2+A[2]                          ;

    // Second point
    //
    if(!Helix) {
      double gP[3]={R[0]+A1*S4,       /// updated position for field eval
                    R[1]+B1*S4, 
                    R[2]+C1*S4};

      m_fieldCache.getFieldZR(gP,f);  /// field at updated position  

    }
    /// if assuming helix, homogenous field 
    else  {
      f[0]=f0[0]; 
      f[1]=f0[1]; 
      f[2]=f0[2];
    }

    double H1[3] = {f[0]*PS2,
                    f[1]*PS2,
                    f[2]*PS2}; 
    double A3    = (A[0]+B2*H1[2])-C2*H1[1]    ; 
    double B3    = (A[1]+C2*H1[0])-A2*H1[2]    ; 
    double C3    = (A[2]+A2*H1[1])-B2*H1[0]    ;

    double A4    = (A[0]+B3*H1[2])-C3*H1[1]    ; 
    double B4    = (A[1]+C3*H1[0])-A3*H1[2]    ; 
    double C4    = (A[2]+A3*H1[1])-B3*H1[0]    ;

    double A5    = 2.*A4-A[0]                  ; 
    double B5    = 2.*B4-A[1]                  ; 
    double C5    = 2.*C4-A[2]                  ;    

    // Last point
    //
    if(!Helix) {
      double gP[3]={R[0]+S*A4, 
                    R[1]+S*B4, 
                    R[2]+S*C4};

      m_fieldCache.getFieldZR(gP,f);

    }
    else{
      f[0]=f0[0]; 
      f[1]=f0[1];
      f[2]=f0[2];
    } 
    
    double H2[3] = {f[0]*PS2,
                    f[1]*PS2,
                    f[2]*PS2}; 

    double A6    = B5*H2[2]-C5*H2[1]           ;
    double B6    = C5*H2[0]-A5*H2[2]           ;
    double C6    = A5*H2[1]-B5*H2[0]           ;

    // Test approximation quality on give step and possible step reduction
    //
    if(!ste) {
      double EST = std::abs((A1+A6)-(A3+A4))+std::abs((B1+B6)-(B3+B4))+std::abs((C1+C6)-(C3+C4)); 
      if(EST>dlt) {
        S*=.6; 
        continue;
      } 
    }

    /// Parameters calculation
    /// if the step grew too small, or we traveled more than 2 meters, abort, this went badly wrong...   
    if((!ste && S0 > std::abs(S)*100.) || std::abs(globalPars[45]+=S) > 2000.) return false;
    ste = true;
	
    double A0arr[3]{A0,B0,C0}; 
    double A3arr[3]{A3,B3,C3}; 
    double A4arr[3]{A4,B4,C4}; 
    double A6arr[3]{A6,B6,C6}; 
    
    if(Jac) {
      Trk::propJacobian(globalPars,H0,H1,H2,A,A0arr,A3arr,A4arr,A6arr,S3); 
    }

    R[0]+=(A2+A3+A4)*S3; 
    A[0] = ((A0+2.*A3)+(A5+A6));
    R[1]+=(B2+B3+B4)*S3; 
    A[1] = ((B0+2.*B3)+(B5+B6));
    R[2]+=(C2+C3+C4)*S3; 
    A[2] = ((C0+2.*C3)+(C5+C6));
    if(!m_tools->isITkGeometry()){
      A[0] *= 1./3.;
      A[1] *= 1./3.;
      A[2] *= 1./3.;
    }

    double D   = 1./sqrt(A[0]*A[0]+A[1]*A[1]+A[2]*A[2]);
    A[0]*=D; A[1]*=D; A[2]*=D;

    /// New step estimation
    ///
    double  a    = A[0]*m_localTransform[6]+A[1]*m_localTransform[7]+A[2]*m_localTransform[8]; 
    if(a==0.) return false;
    double  Sn   = ((m_localTransform[12]-R[0]*m_localTransform[6])-(R[1]*m_localTransform[7]+R[2]*m_localTransform[8]))/a;
    double aSn = std::abs(Sn);

    /// if the remaining step is below threshold,
    /// we can finish with a small linear step 
    if(aSn <= Smin) {
      double Sl = 2./S; 
      sA[0] = A6*Sl; 
      sA[1] = B6*Sl; 
      sA[2] = C6*Sl;

      R[0]+=(A[0]*Sn); 
      R[1]+=(A[1]*Sn); 
      R[2]+=(A[2]*Sn); 
      globalPars[45]+=Sn; 
      
      return true;  
    }

    double aS = std::abs(S);
    
    /// if we ran past the surface and changed signs more than twice, abort 
    if     (  S*Sn < 0. ) {
      if(++it > 2) return false;
      /// if absolute step size is decreasing, we keep going 
      if (aSn < aS) S = Sn; 
      /// if step increased, try the other direction
      else S =-S;
    }
    /// otherwise refine step length if shorter than before 
    else if( aSn  < aS  ) S = Sn;

    /// update field for next iteration
    f0[0]=f[0]; 
    f0[1]=f[1]; 
    f0[2]=f[2];
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////////
// Straight line step to plane
/////////////////////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::straightLineStepToPlane
(bool Jac,double* globalPars) 
{
  double*  R   = &globalPars[ 0];             // Start coordinates
  double*  A   = &globalPars[ 3];             // Start directions
  /// 
  double  a    = A[0]*m_localTransform[6]+A[1]*m_localTransform[7]+A[2]*m_localTransform[8]; 
  if(a==0.) return false; 
  double  S    = ((m_localTransform[12]-R[0]*m_localTransform[6])-(R[1]*m_localTransform[7]+R[2]*m_localTransform[8]))/a;
  globalPars[45]        = S;

  // Track parameters in last point
  //
  R[0]+=(A[0]*S); R[1]+=(A[1]*S); R[2]+=(A[2]*S); if(!Jac) return true;
  
  // Derivatives of track parameters in last point
  //
  for(int i=7; i<42; i+=7) {

    double* dR = &globalPars[i  ]; 
    double* dA = &globalPars[i+3];
    dR[0]+=(dA[0]*S); dR[1]+=(dA[1]*S); dR[2]+=(dA[2]*S);
  }
  return true;
}

InDet::SiTrajectoryElement_xk::SiTrajectoryElement_xk()
{
  m_detstatus         =-1  ;
  m_status            = 0  ;
  m_nlinksForward     = 0  ;
  m_nlinksBackward    = 0  ;
  m_nMissing          = 0  ;
  m_radlength         = .03;
  m_radlengthN        = .03;
  m_energylose        = .4 ;
  m_tools             = nullptr  ;
  m_noisemodel        = 0  ; 
  m_covariance.resize(2,2);
  m_covariance<<0.,0.,0.,0.;
  m_ndf               = 0 ;
  m_ndfBackward       = 0 ;
  m_ndfForward        = 0 ;
  m_ntsos             = 0 ;
  m_maxholes          = 0 ;
  m_maxdholes         = 0 ;
  m_xi2Forward        = 0.;
  m_xi2Backward       = 0.;
  m_xi2totalForward   = 0.;
  m_xi2totalBackward  = 0.;
  m_halflength        = 0.;
  m_step              = 0.;
  m_xi2max            = 0.;
  m_dist              = 0.;
  m_xi2maxNoAdd       = 0.;
  m_xi2maxlink        = 0.;
  m_xi2multi          = 0.;
  m_invMoment         = 0.;
  m_detelement        = nullptr ; 
  m_detlink           = nullptr ;
  m_surface           = nullptr ;
  m_cluster           = nullptr ;
  m_clusterOld        = nullptr ;
  m_clusterNoAdd      = nullptr ;
  m_updatorTool       = nullptr ;
  m_proptool          = nullptr ;
  m_riotool           = nullptr ;
  m_inside            = 0 ;
  m_nholesForward     = 0 ;
  m_nholesBackward    = 0 ;
  m_dholesForward     = 0 ;
  m_dholesBackward    = 0 ;
  m_nclustersForward  = 0 ;
  m_nclustersBackward = 0 ;
  m_npixelsBackward   = 0 ;
  m_stereo      = false   ;
  m_fieldMode   = false   ;

  m_tsos[0]=m_tsos[1]=m_tsos[2]=nullptr; 
}

InDet::SiTrajectoryElement_xk::SiTrajectoryElement_xk(const SiTrajectoryElement_xk& E)
{
  *this          = E;
}

// deliberately not assigning all variables?
// cppcheck-suppress operatorEqVarError
InDet::SiTrajectoryElement_xk& InDet::SiTrajectoryElement_xk::operator = 
(const InDet::SiTrajectoryElement_xk& E) 
{
  if(&E==this) return(*this);

  m_fieldMode                 = E.m_fieldMode   ;
  m_status                    = E.m_status      ;
  m_detstatus                 = E.m_detstatus   ;
  m_inside                    = E.m_inside      ;
  m_nMissing                  = E.m_nMissing    ;
  m_stereo                    = E.m_stereo      ;
  m_detelement                = E.m_detelement  ;
  m_detlink                   = E.m_detlink     ;
  m_surface                   = E.m_surface     ;
  m_sibegin                   = E.m_sibegin     ;
  m_siend                     = E.m_siend       ; 
  m_cluster                   = E.m_cluster     ;
  m_clusterOld                = E.m_clusterOld  ;
  m_clusterNoAdd              = E.m_clusterNoAdd;
  m_parametersPredForward     = E.m_parametersPredForward;
  m_parametersUpdatedForward  = E.m_parametersUpdatedForward;
  m_parametersPredBackward    = E.m_parametersPredBackward;
  m_parametersUpdatedBackward = E.m_parametersUpdatedBackward;
  m_parametersSM              = E.m_parametersSM;
  m_dist                      = E.m_dist            ;
  m_xi2Forward                = E.m_xi2Forward      ;
  m_xi2Backward               = E.m_xi2Backward     ;
  m_xi2totalForward           = E.m_xi2totalForward ;
  m_xi2totalBackward          = E.m_xi2totalBackward;
  m_radlength                 = E.m_radlength       ;
  m_radlengthN                = E.m_radlengthN      ;
  m_energylose                = E.m_energylose      ;
  m_halflength                = E.m_halflength      ;
  m_step                      = E.m_step            ;
  m_nlinksForward             = E.m_nlinksForward   ;
  m_nlinksBackward            = E.m_nlinksBackward  ;
  m_nholesForward             = E.m_nholesForward   ;
  m_nholesBackward            = E.m_nholesBackward  ;
  m_dholesForward             = E.m_dholesForward   ;
  m_dholesBackward            = E.m_dholesBackward  ;
  m_noisemodel                = E.m_noisemodel      ;
  m_ndf                       = E.m_ndf             ;
  m_ndfForward                = E.m_ndfForward      ;
  m_ndfBackward               = E.m_ndfBackward     ;
  m_ntsos                     = E.m_ntsos           ;
  m_nclustersForward          = E.m_nclustersForward   ; 
  m_nclustersBackward         = E.m_nclustersBackward  ;
  m_npixelsBackward           = E.m_npixelsBackward    ;
  m_noise                     = E.m_noise       ;
  m_tools                     = E.m_tools       ;
  m_covariance                = E.m_covariance  ;
  m_position                  = E.m_position    ;
  m_invMoment                 = E.m_invMoment   ;
  for(int i=0; i!=m_nlinksForward; ++i) {m_linkForward[i]=E.m_linkForward[i];}
  for(int i=0; i!=m_nlinksBackward; ++i) {m_linkBackward[i]=E.m_linkBackward[i];}
  for(int i=0; i!=m_ntsos  ; ++i) {m_tsos [i]=E.m_tsos [i];}
  for(int i=0; i!=m_ntsos  ; ++i) {m_utsos[i]=E.m_utsos [i];}
  return(*this);
}

int InDet::SiTrajectoryElement_xk::numberClusters() const
{
  int n = 0;
  if (m_detstatus<=0) return n;

  if (m_itType==PixelClusterColl) {
    const InDet::PixelClusterCollection::const_iterator* sibegin
      = std::any_cast<const InDet::PixelClusterCollection::const_iterator>(&m_sibegin);
    const InDet::PixelClusterCollection::const_iterator* siend
      = std::any_cast<const InDet::PixelClusterCollection::const_iterator>(&m_siend);
    if (sibegin==nullptr or siend==nullptr) return 0;
    for (InDet::PixelClusterCollection::const_iterator p = *sibegin; p!=*siend; ++p) {
      ++n;
    }
  } else if (m_itType==SCT_ClusterColl) {
    const InDet::SCT_ClusterCollection::const_iterator* sibegin
      = std::any_cast<const InDet::SCT_ClusterCollection::const_iterator>(&m_sibegin);
    const InDet::SCT_ClusterCollection::const_iterator* siend
      = std::any_cast<const InDet::SCT_ClusterCollection::const_iterator>(&m_siend);
    if (sibegin==nullptr or siend==nullptr) return 0;
    for (InDet::SCT_ClusterCollection::const_iterator p = *sibegin; p!=*siend; ++p) {
      ++n;
    }
  } else {
    const InDet::SiClusterCollection::const_iterator* sibegin
      = std::any_cast<const InDet::SiClusterCollection::const_iterator>(&m_sibegin);
    const InDet::SiClusterCollection::const_iterator* siend
      = std::any_cast<const InDet::SiClusterCollection::const_iterator>(&m_siend);
    if (sibegin==nullptr or siend==nullptr) return 0;
    for (InDet::SiClusterCollection::const_iterator p = *sibegin; p!=*siend; ++p) {
      ++n;
    }
  }
  return n;
}
  
bool InDet::SiTrajectoryElement_xk::difference() const
{
  return m_cluster != m_clusterOld || m_status != 3;
}

/////////////////////////////////////////////////////////////////////////////////
// Test for next compatible cluster
/////////////////////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::isNextClusterHoleB(bool& cl,double& X)
{
  cl = false             ;
  X  = m_xi2totalBackward-m_xi2Backward;

  if(m_nlinksBackward >  1 && m_linkBackward[1].xi2() <= m_xi2max) {
    X+=m_linkBackward[1].xi2();
    cl = true; return true;
  }

  if(m_inside < 0) {
    return m_nholesBackward < m_maxholes && m_dholesBackward < m_maxdholes;
  }
  return true;
}

/// checks if removing this cluster from the forward propagation would 
/// result in a critical number of holes or double holes.
/// Fills "cl" with true if we would still find another cluster after removing 
/// the preferred one. Fills X with the chi square we would have on removal. 
/// Return true if we would *not* get a problematic number of holes
bool InDet::SiTrajectoryElement_xk::isNextClusterHoleF(bool& cl,double& X)
{
  cl = false             ;
  /// chi2 if we drop this cluster
  X  = m_xi2totalForward-m_xi2Forward;
  /// not used in SiTrajectory? 
  if(m_detstatus == 2) return false;
  /// if we have another good cluster on this element which could replace
  /// the current one were it dropped: 
  if(m_nlinksForward >  1 && m_linkForward[1].xi2() <= m_xi2max) {
    /// add the chi2 of the second forward link to the second arg
    X+=m_linkForward[1].xi2();
    /// set the return-arg signifying we would still see a cluster and return
    cl = true; 
    return true;
  }
  /// if we expect a hit: 
  if(m_inside < 0) {
    /// return true if we are below the max allowed holes (so we can add one)
    if(m_nholesForward < m_maxholes && m_dholesForward < m_maxdholes) return true;
    /// otherwise return false 
    return false;
  }
  /// if we do not expect a hit return true 
  return true;
}

void InDet::SiTrajectoryElement_xk::setCluster(const InDet::SiCluster* Cl)
{
  m_cluster = Cl;
}

void InDet::SiTrajectoryElement_xk::setClusterB(const InDet::SiCluster* Cl,double Xi2)
{
  m_cluster = Cl ;
  m_status  = 2  ;
  m_xi2Backward = Xi2;
}
  

void InDet::SiTrajectoryElement_xk::setParametersB(Trk::PatternTrackParameters& P)
{
  m_parametersUpdatedBackward = P;
} 

void InDet::SiTrajectoryElement_xk::setParametersF(Trk::PatternTrackParameters& P)
{
  m_parametersUpdatedForward = P;
}

void InDet::SiTrajectoryElement_xk::setNdist(int n)
{
  m_nMissing = n;
}
 
/////////////////////////////////////////////////////////////////////////////////
// Add pixel or SCT cluster to pattern track parameters with Xi2 calculation
/////////////////////////////////////////////////////////////////////////////////
  
bool InDet::SiTrajectoryElement_xk::addCluster
(Trk::PatternTrackParameters& Ta,Trk::PatternTrackParameters& Tb,double& Xi2)
{
  int N; 
  /// non-stereo measurements
  if(!m_stereo) {
    /// set m_covariance to the cluster errors 
    patternCovariances
      (m_cluster,m_covariance(0,0),m_covariance(1,0),m_covariance(1,1));	
    /// update state
    if(m_detelement->isSCT()) {
      return m_updatorTool->addToStateOneDimension
        (Ta,m_cluster->localPosition(),m_covariance,Tb,Xi2,N);
    } 
    return m_updatorTool->addToState
      (Ta,m_cluster->localPosition(),m_covariance,Tb,Xi2,N);
  }
  /// for stereo, use the cluster local covariance directly
  return m_updatorTool->addToStateOneDimension
    (Ta,m_cluster->localPosition(),m_cluster->localCovariance(),Tb,Xi2,N);
}

/////////////////////////////////////////////////////////////////////////////////
// Add pixel or SCT cluster to pattern track parameters without Xi2 calculation
/////////////////////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::addCluster
(Trk::PatternTrackParameters& Ta,Trk::PatternTrackParameters& Tb)
{
  if(!m_stereo) {

    patternCovariances
      (m_cluster,m_covariance(0,0),m_covariance(1,0),m_covariance(1,1));	
    if(m_detelement->isSCT()) {
      return m_updatorTool->addToStateOneDimension
        (Ta,m_cluster->localPosition(),m_covariance,Tb);
    }
    return m_updatorTool->addToState
      (Ta,m_cluster->localPosition(),m_covariance,Tb);
  }
  return m_updatorTool->addToStateOneDimension
    (Ta,m_cluster->localPosition(),m_cluster->localCovariance(),Tb);
}

/////////////////////////////////////////////////////////////////////////////////
// Add pixel or SCT cluster to pattern track parameters with Xi2 calculation
// using precise error
/////////////////////////////////////////////////////////////////////////////////
  
bool InDet::SiTrajectoryElement_xk::addClusterPrecise(Trk::PatternTrackParameters& Ta,
					       Trk::PatternTrackParameters& Tb,
					       double& Xi2) {
  int N;
  if(m_ndf==1) {
    return m_updatorTool->addToStateOneDimension(Ta,m_position,m_covariance,Tb,Xi2,N);
  }
  else {
    return m_updatorTool->addToState           (Ta,m_position,m_covariance,Tb,Xi2,N);
  }
}

/////////////////////////////////////////////////////////////////////////////////
// Add two pattern track parameters without Xi2 calculation
/////////////////////////////////////////////////////////////////////////////////
 
bool InDet::SiTrajectoryElement_xk::combineStates 
(Trk::PatternTrackParameters& Ta,
 Trk::PatternTrackParameters& Tb,
 Trk::PatternTrackParameters& Tc)
{
  return m_updatorTool->combineStates(Ta,Tb,Tc);
}

/////////////////////////////////////////////////////////////////////////////////
// Propagate pattern track parameters to surface
/////////////////////////////////////////////////////////////////////////////////

void InDet::SiTrajectoryElement_xk::noiseInitiate()
{
  m_noisemodel = 1; m_noise.initiate();
}

///////////////////////////////////////////////////////////////////
// Initiate state
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::initiateState
(Trk::PatternTrackParameters& inputPars,Trk::PatternTrackParameters& outputPars)
{
  /// Gives Tb the x and (for pix) the y of the cluster, and the corresponding cov elements, 
  /// and copies everything else from inputPars
  if (m_tools->isITkGeometry()) {
    // using pattern covariance for all clusters for ITk
    Amg::MatrixX cov(2,2);
    patternCovariances(m_cluster,cov(0,0),cov(1,0),cov(1,1));
    return outputPars.initiate(inputPars,m_cluster->localPosition(),cov);
  }
  return outputPars.initiate(inputPars,m_cluster->localPosition(),m_cluster->localCovariance());
}

///////////////////////////////////////////////////////////////////
// Initiate state with cluster correction
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::initiateStatePrecise
(Trk::PatternTrackParameters& inputPars,Trk::PatternTrackParameters& outputPars)
{
  return outputPars.initiate(inputPars,m_position,m_covariance);
}

///////////////////////////////////////////////////////////////////
// Pattern covariances
///////////////////////////////////////////////////////////////////

void InDet::SiTrajectoryElement_xk::patternCovariances
(const InDet::SiCluster* c,double& covX,double& covXY,double& covY) const
{
  const Amg::MatrixX& v = c->localCovariance();
  if (m_tools->useFastTracking() and m_stereo) {
    // in fast tracking mode, endcap strip clusters use cluster covariance terms
    covX=v(0,0);
    covY=v(1,1);
    covXY=v(1,0);
    return;
  }
  covX  = c->width().phiR(); 
  covX*=(covX*s_oneOverTwelve);  /// sigma ~pitch / sqrt(12)
  covXY = c->localCovariance()(1,0);

  if(!m_tools->useFastTracking()){
    if(covX < v(0,0)) covX=v(0,0);  /// if larger error in cluster covariance, replace covx by it
    covXY = 0.;
  }

  if(m_ndf==1) {    /// strips: take y error from cov 
    covY=v(1,1);
  }
  else         {
    /// pixels: again take either pitch/sqrt(12) or the cluster cov, whichever is larger
    covY=c->width().z(); 
    covY*=(covY*s_oneOverTwelve);
    if(!m_tools->useFastTracking()){
      if(covY < v(1,1)) covY=v(1,1);
    }
  }
}

///////////////////////////////////////////////////////////////////
// Last detector elements with clusters
///////////////////////////////////////////////////////////////////

void InDet::SiTrajectoryElement_xk::lastActive()
{
  m_detstatus = 2;
}

Trk::TrackStateOnSurface* InDet::SiTrajectoryElement_xk::tsos (int i) 
{
  if(i<0 || i>2) return nullptr;

  bool us = m_utsos[i]; 
  m_utsos[i] = true;

  if(us) return new Trk::TrackStateOnSurface(*m_tsos[i]);

  return m_tsos[i];  
}

///////////////////////////////////////////////////////////////////
// Set electron noise model
///////////////////////////////////////////////////////////////////
  
void InDet::SiTrajectoryElement_xk::bremNoiseModel()
{
  m_noisemodel = 2;
}

///////////////////////////////////////////////////////////////////
// Initiate state with cluster correction
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::initiateStateWithCorrection(Trk::PatternTrackParameters& Tc,
								Trk::PatternTrackParameters& Ta,
								Trk::PatternTrackParameters& Tb)
{
  precisePosCov(Tc);
  return Tb.initiate(Ta,m_position,m_covariance);
}

/////////////////////////////////////////////////////////////////////////////////
// Add pixel or SCT cluster to pattern track parameters with Xi2 calculation
// using precise error
/////////////////////////////////////////////////////////////////////////////////

bool InDet::SiTrajectoryElement_xk::addClusterPreciseWithCorrection(Trk::PatternTrackParameters& Tc,
								    Trk::PatternTrackParameters& Ta,
								    Trk::PatternTrackParameters& Tb,
								    double& Xi2)
{
  int N;
  precisePosCov(Tc);

  if(m_ndf==1) {
    return m_updatorTool->addToStateOneDimension(Ta,m_position,m_covariance,Tb,Xi2,N);
  }
  else         {
    return m_updatorTool->addToState(Ta,m_position,m_covariance,Tb,Xi2,N);
  }
}

/////////////////////////////////////////////////////////////////////////////////
// Precise cluster position and covariance calculation
/////////////////////////////////////////////////////////////////////////////////

void  InDet::SiTrajectoryElement_xk::precisePosCov(Trk::PatternTrackParameters& Tc)
{

  m_position   = m_cluster->localPosition  ();
  m_covariance = m_cluster->localCovariance();

  if(m_ndf==1) return;

  const Amg::Vector2D& colRow = m_cluster->width().colRow();
  if(colRow.x()==1. && colRow.y()==1.) return;

  std::unique_ptr<Trk::TrackParameters> tr = Tc.convert(true);
  std::unique_ptr<const Trk::RIO_OnTrack> ri(m_riotool->correct(*m_cluster, *tr));

  m_position   = ri->localParameters();
  m_covariance = ri->localCovariance();

}

///////////////////////////////////////////////////////////////////
// Search clusters compatible with track
///////////////////////////////////////////////////////////////////

int InDet::SiTrajectoryElement_xk::searchClusters(Trk::PatternTrackParameters& Tp, SiClusterLink_xk* L) {
  /// delegate work to subsystem-specific template implementation
  if (m_itType==PixelClusterColl) return searchClustersSub<InDet::PixelClusterCollection::const_iterator>(Tp, L);
  if (m_itType==SCT_ClusterColl) return searchClustersSub<InDet::SCT_ClusterCollection::const_iterator>(Tp, L);
  return searchClustersSub<InDet::SiClusterCollection::const_iterator>(Tp, L);
}
void InDet::SiTrajectoryElement_xk::checkBoundaries(const Trk::PatternTrackParameters & pars){

    m_inside = m_detlink->intersect(pars, m_dist); 
  
}
