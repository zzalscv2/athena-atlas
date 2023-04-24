/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 *
 * @class InDet::InDetIterativeSecVtxFinderTool
 * @author  Lianyou SHAN ( IHEP )
 *
 * developed upon the InDetPriVxFinderTool by :
 * @author Giacinto Piacquadio (Freiburg University)
 *
 *
 * This class provides an implementation for a secondary
 * vertex finding tool, which uses the Adaptive Vertex 
 * Fitter to reject outliers not belonging to the vertex interaction.
 *
 ***************************************************************************/

//implemented using as template the InDetPriVxFinderTool class of A. Wildauer and F. Akesson

#ifndef INDETPRIVXFINDERTOOL_INDETITERATIVESECVXFINDERTOOL_H
#define INDETPRIVXFINDERTOOL_INDETITERATIVESECVXFINDERTOOL_H

#include "InDetRecToolInterfaces/IInDetIterativeSecVtxFinderTool.h"
#include "InDetRecToolInterfaces/IVertexFinder.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "TrkVertexFitterInterfaces/IImpactPoint3dEstimator.h"
#include "TrkVertexFitterInterfaces/IVertexSeedFinder.h"
#include "TrkVertexFitters/AdaptiveVertexFitter.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TrkTrack/TrackCollection.h" // type def ...
#include "TrkParticleBase/TrackParticleBaseCollection.h" // type def ...
#include "TrkParameters/TrackParameters.h"
// we may save out some private stuff
#include "xAODTracking/VertexFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODTracking/VertexContainerFwd.h"
#include "xAODTracking/TrackParticleContainerFwd.h"
#include <vector>


class TTree;

namespace Trk
{

 class Track;
 class TrackParticleBase;
 class ITrackLink;
 class IVertexSeedFinder;
 class IImpactPoint3dEstimator;
 class IVertexLinearizedTrackFactory;
 class IVxCandidateXAODVertex;
}
//
//
namespace InDet
{
 class IInDetTrackSelectionTool;
  
//  InnerDetector/InDetValidation/InDetVertexSplitter/InDetVertexSplitterHist
//  InnerDetector/InDetDigitization/FastSiDigitization/SiSmearedDigitizationTool
//  Tracking/TrkVertexFitter/TrkVertexSeedFinderUtils/share/ImagingSeedTuningAlg_jobOptions.py
 class ATLAS_NOT_THREAD_SAFE InDetIterativeSecVtxFinderTool : public extends<AthAlgTool, ISecVertexFinder>
 {

public:

   /**
   * Constructor
   */
   using extends::extends;
   InDetIterativeSecVtxFinderTool(const std::string& t, const std::string& n, const IInterface*  p);
   
   /**
   * Destructor
   */
   
   virtual ~InDetIterativeSecVtxFinderTool() = default;
    
   StatusCode initialize() override;
   StatusCode finalize() override;

   std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> 
         findVertex(const TrackCollection* trackTES) override;
   std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> 
         findVertex(const Trk::TrackParticleBaseCollection* trackTES) override;

   /** 
   * Finding method.
   * Has as input a track collection and as output 
   * a VxContainer.
   */

   std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> 
         findVertex(const xAOD::TrackParticleContainer* trackParticles) override;
   std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*>
         findVertex( const std::vector<const xAOD::IParticle*> & inputTracks) override;

   void setPriVtxPosition( double , double , double ) override;

   int getModes1d( std::vector<int> *, std::vector<int> *, std::vector<int> * ) const ;

 private:


   std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> 
     findVertex( const std::vector<Trk::ITrackLink*> & trackVector) ;

   bool passHitsFilter( const Trk::TrackParameters* , float vtxR, float absvz ) const;

   bool V0kine( const std::vector<Amg::Vector3D>&, const Amg::Vector3D &position , float & , float & ) const;


   const std::vector<Amg::Vector3D> getVertexMomenta( xAOD::Vertex * myxAODVertex ) const ;
//   double MomentumDirection( int, const Amg::Vector3D & ) const ;


   static double VrtVrtDist( xAOD::Vertex * v1, xAOD::Vertex * v2 ) ;

   float removeTracksInBadSeed( xAOD::Vertex * myxAODVertex, std::vector<const Trk::TrackParameters*> & ) const ;

   void FillXcheckdefauls() ;


   Amg::Vector3D m_privtx ;
   mutable std::vector< Amg::Vector3D > m_TrkAtVtxMomenta ;

// watch below for possible update from InDetIterativePriVxFinderTool 

   void removeCompatibleTracks(xAOD::Vertex * myxAODVertex,
                               std::vector<const Trk::TrackParameters*> & perigeesToFit,
                               std::vector<Trk::ITrackLink*> & seedTracks) const;

   void removeAllFrom(std::vector<const Trk::TrackParameters*> & perigeesToFit,
                      std::vector<Trk::ITrackLink*> & seedTracks) const;

//   double compatibility(const Trk::TrackParameters& measPerigee,
//                        const xAOD::Vertex & vertex  ) const;
   double compatibility(const Trk::TrackParameters& measPerigee,
                  const AmgSymMatrix(3) &covariancePosition , const Amg::Vector3D &position ) const ;
   void countTracksAndNdf(xAOD::Vertex * myxAODVertex,
                          float & ndf, int & ntracks) const;



   ToolHandle< Trk::AdaptiveVertexFitter > m_iVertexFitter{this, "VertexFitterTool", "Trk::AdaptiveVertexFitter","Vertex Fitter"};
   ToolHandle< InDet::IInDetTrackSelectionTool > m_trkFilter{this, "BaseTrackSelector", "InDet::InDetTrackSelection", "base track selector"};
   ToolHandle< InDet::IInDetTrackSelectionTool > m_SVtrkFilter{this, "SecVtxTrackSelector", "InDet::InDetSecVtxTrackSelection", "SV track selector"};
   ToolHandle< Trk::IVertexSeedFinder > m_SeedFinder{this, "SeedFinder", "Trk::IndexedCrossDistancesSeedFinder", "seed finder"};
   ToolHandle< Trk::IImpactPoint3dEstimator > m_ImpactPoint3dEstimator{this, "ImpactPoint3dEstimator", "Trk::ImpactPoint3dEstimator", "impact point estimator"};
   ToolHandle< Trk::IVertexLinearizedTrackFactory > m_LinearizedTrackFactory{this, "LinearizedTrackFactory", "Trk::FullLinearizedTrackFactory", "linearized track factory"};
   

   Gaudi::Property<int> m_filterLevel{this,"VertexFilterLevel",0,""} ;
   Gaudi::Property<double> m_significanceCutSeeding{this, "significanceCutSeeding", 9., ""};
   Gaudi::Property<double> m_maximumChi2cutForSeeding{this, "maxCompatibilityCutSeeding",18., ""};
   Gaudi::Property<double> m_minWghtAtVtx{this, "minTrackWeightAtVtx",0.02, ""};
   Gaudi::Property<double> m_maxVertices{this, "maxVertices",20, ""};
   Gaudi::Property<double> m_CutHitsFilter{this, "TrackInnerOuterFraction",0.95, ""};
   Gaudi::Property<float> m_privtxRef{this, "MomentumProjectionOnDirection",-999.9,""};
   Gaudi::Property<float> m_minVtxDist{this, "SeedsMinimumDistance",0.1,""};
   Gaudi::Property<bool> m_createSplitVertices{this, "createSplitVertices", false, ""};
   Gaudi::Property<int> m_splitVerticesTrkInvFraction{this, "splitVerticesTrkInvFraction",2,""}; ///< Integer: 1./fraction of tracks to be assigned to the tag split vertex 

   Gaudi::Property<bool> m_reassignTracksAfterFirstFit{this, "reassignTracksAfterFirstFit", true, ""};

   Gaudi::Property<bool> m_doMaxTracksCut{this, "doMaxTracksCut", false, ""}; 
   Gaudi::Property<unsigned int> m_maxTracks{this, "MaxTracks", 5000,""};

   void SGError(const std::string& errService);

   virtual void printParameterSettings();
 
   TTree*              m_OTree{} ;

   long int m_evtNum{} ;
   int m_iterations{} ;

   std::vector<int> *m_leastmodes{} ;
   std::vector< std::vector < float > > * m_sdFsmwX{};
   std::vector< std::vector < float > > * m_sdFsmwY{};
   std::vector< std::vector < float > > * m_sdFsmwZ{};
   std::vector< std::vector < float > > * m_sdcrsWght{};

   std::vector<int> * m_nperiseed{} ;
   std::vector < float > *m_seedX{} ;
   std::vector < float > *m_seedY{} ;
   std::vector < float > *m_seedZ{} ;
   std::vector < float > *m_seedXYdist{} ;
   std::vector < float > *m_seedZdist{} ;
   std::vector < int > *m_seedac{} ;

   mutable float m_v0mass{}, m_v0ee{}, m_dir{}, m_ndf{}, m_hif{} ;
   mutable int m_ntracks{} ;
   mutable std::vector< Amg::VectorX > m_trkdefiPars ;

 };//end of class definitions
}//end of namespace definitions
#endif
                                                                                                             
