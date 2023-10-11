/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//Author: Lianyou Shan <lianyou.shan@cern.ch>

/***************************************************************************
                         InDetIterativeSecVtxFinderTool.cxx  -  Description
                             -------------------
    begin   : 07-11-2016
    authors : Lianyou SHAN ( IHEP@BEIJING )
 ***************************************************************************/

#define MONITORTUNES  // UNcomment this line to get more output, unfortunately in an ugly way

#include "src/InDetIterativeSecVtxFinderTool.h"

#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Matrix/Vector.h"
#include "TrkTrack/Track.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkParticleBase/TrackParticleBase.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "TrkVertexSeedFinderUtils/IMode3dFinder.h"

#include "TrkSurfaces/PlaneSurface.h"
#include "EventPrimitives/EventPrimitives.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitives.h"


#include "TrkVertexFitterInterfaces/IImpactPoint3dEstimator.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "TrkVertexFitterInterfaces/IVertexSeedFinder.h"

#include "AthContainers/DataVector.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "VxVertex/RecVertex.h"
#include "VxVertex/VxTrackAtVertex.h"

#include "TrkTrackLink/ITrackLink.h"
#include "TrkTrack/LinkToTrack.h"
#include "TrkParticleBase/LinkToTrackParticleBase.h"
#include "TrkLinks/LinkToXAODTrackParticle.h"
#include "TrkVertexFitterInterfaces/IVertexLinearizedTrackFactory.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include <map>
#include <utility>

// we may save out some private stuff
#ifdef MONITORTUNES
  #include "GaudiKernel/ITHistSvc.h"
  #include "TFile.h"
  #include "TTree.h"
#endif

namespace InDet
{

InDetIterativeSecVtxFinderTool::InDetIterativeSecVtxFinderTool(const std::string& t, const std::string& n, const IInterface*  p)
        : base_class(t,n,p){}


void InDetIterativeSecVtxFinderTool::setPriVtxPosition( double vx, double vy, double vz )
{ 
  m_privtx = Amg::Vector3D( vx, vy, vz ) ; 
}

std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> InDetIterativeSecVtxFinderTool::findVertex(const xAOD::TrackParticleContainer* trackParticles)
{
  ATH_MSG_DEBUG(" Number of input tracks before track selection: " << trackParticles->size());

  m_evtNum ++ ;

  std::vector<Trk::ITrackLink*> selectedTracks;


  using TrackParticleDataVecIter = DataVector<xAOD::TrackParticle>::const_iterator;

  bool selectionPassed;
  m_trkdefiPars.clear() ;
  xAOD::Vertex null;
  null.makePrivateStore();
  null.setPosition(Amg::Vector3D(0,0,0));
  AmgSymMatrix(3) vertexError;
  vertexError.setZero();
  null.setCovariancePosition(vertexError);
  for (TrackParticleDataVecIter itr  = trackParticles->begin(); itr != trackParticles->end(); ++itr) {
        
    selectionPassed=static_cast<bool>(m_trkFilter->accept(**itr,&null));
    if ( selectionPassed ) selectionPassed =static_cast<bool>(m_SVtrkFilter->accept(**itr,&null));
    if (selectionPassed)
    {
      Amg::VectorX par = (*itr)->definingParameters();
      par[0] = 1.0*(*itr)->hitPattern() ;
      m_trkdefiPars.push_back( par ) ;

      ElementLink<xAOD::TrackParticleContainer> link;
      link.setElement(*itr);
      Trk::LinkToXAODTrackParticle * linkTT = new Trk::LinkToXAODTrackParticle(link);

      linkTT->setStorableObject(*trackParticles);
      selectedTracks.push_back(linkTT);
    }
  }

  ATH_MSG_DEBUG("Of " << trackParticles->size() << " tracks " << selectedTracks.size() << " survived the preselection.");


  std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> returnContainers 
     =findVertex( selectedTracks    );

  return returnContainers;

}

std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*>
         InDetIterativeSecVtxFinderTool::findVertex( const std::vector<const xAOD::IParticle*> & inputTracks)  
{
  
  ATH_MSG_DEBUG(" Number of input tracks before track selection: " << inputTracks.size());

  m_evtNum ++ ;

  std::vector<Trk::ITrackLink*> selectedTracks;


  bool selectionPassed;
  m_trkdefiPars.clear() ;
  xAOD::Vertex null;
  null.makePrivateStore();
  null.setPosition(Amg::Vector3D(0,0,0));
  AmgSymMatrix(3) vertexError;
  vertexError.setZero();
  null.setCovariancePosition(vertexError);
  std::vector<const xAOD::IParticle*>::const_iterator   trk_iter;
  for (trk_iter= inputTracks.begin(); trk_iter != inputTracks.end(); ++trk_iter)
  {
    const xAOD::TrackParticle * tmp=dynamic_cast<const xAOD::TrackParticle *> ((*trk_iter));
    
    selectionPassed=static_cast<bool>(m_trkFilter->accept( *tmp, &null));
    if ( selectionPassed ) selectionPassed =static_cast<bool>(m_SVtrkFilter->accept(*tmp,&null));
 
    if (selectionPassed)
    {
      Amg::VectorX par = tmp->definingParameters();
      par[0] = 1.0*tmp->hitPattern() ;
      m_trkdefiPars.push_back( par ) ;

      ElementLink<xAOD::TrackParticleContainer> linkTP;
      linkTP.setElement(tmp);
      
      Trk::LinkToXAODTrackParticle* link= new Trk::LinkToXAODTrackParticle(linkTP);
      
      selectedTracks.push_back(link);
    }

  }

  std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> returnContainers
     =findVertex( selectedTracks    );

  return returnContainers;

}

// The workhorse ...
std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> 
InDetIterativeSecVtxFinderTool::findVertex(const std::vector<Trk::ITrackLink*> & trackVector) 
{
  
  //two things need to be added
  //1) the seeding mechanism
  //2) the iterative removal of tracks
  std::vector<Trk::ITrackLink*> origTracks=trackVector;
  std::vector<Trk::ITrackLink*> seedTracks=trackVector;
// in the iteration from the below do { ... } while () loop, 
// the container of seedTracks is updated/dynamic : some tracks are moved out 
// once a vertex is successfully found
//
  
  xAOD::VertexContainer* theVertexContainer = new xAOD::VertexContainer;
  xAOD::VertexAuxContainer* theVertexAuxContainer = new xAOD::VertexAuxContainer;
  theVertexContainer->setStore( theVertexAuxContainer );
  const auto invalidResponse = std::make_pair(theVertexContainer, theVertexAuxContainer);

  //bail out early with only Dummy vertex if multiplicity cut is applied and exceeded
  if (m_doMaxTracksCut && (trackVector.size() > m_maxTracks)){ 
    ATH_MSG_WARNING( trackVector.size() << " tracks - exceeds maximum (" << m_maxTracks << "), skipping vertexing and returning only dummy..." ); 
    return std::make_pair(theVertexContainer, theVertexAuxContainer);
  } 
 
  m_iterations = -1 ;
  unsigned int seedtracknumber=seedTracks.size();
  
  //used to store seed info
  Amg::Vector3D seedVertex;

  //prepare iterators for tracks only necessary for seeding
  std::vector<Trk::ITrackLink*>::iterator seedBegin;
  std::vector<Trk::ITrackLink*>::iterator seedEnd;

// fortunately the tool InDetIterativeSecVtxFinderTool::findVertex is called only once per event

#ifdef MONITORTUNES

  m_leastmodes->clear() ;
  m_sdFsmwX->clear() ;
  m_sdFsmwY->clear() ;
  m_sdFsmwZ->clear() ;
  m_sdcrsWght->clear() ;

  m_nperiseed->clear() ;
  m_seedX->clear() ;
  m_seedY->clear() ;
  m_seedZ->clear() ;
  m_seedXYdist->clear() ;
  m_seedZdist->clear() ;
  m_seedac->clear() ;

  SG::AuxElement::Decorator<std::vector<float> > mDecor_trkWght( "trkWeight" ) ;
  SG::AuxElement::Decorator<std::vector<float> > mDecor_trkDOE( "trkDistOverError" ) ;
  SG::AuxElement::Decorator<float> mDecor_direction( "MomentaDirection" ) ;
  SG::AuxElement::Decorator< float > mDecor_HitsFilter( "radiiPattern" );
  SG::AuxElement::Decorator< int > mDecor_NumTrk( "NumTrkAtVtx" );
#endif

  SG::AuxElement::Decorator<float> mDecor_sumPt2( "sumPt2" );
  SG::AuxElement::Decorator<float> mDecor_mass( "mass" );
  SG::AuxElement::Decorator<float> mDecor_energy( "ee" );
  SG::AuxElement::Decorator<int> mDecor_nrobbed( "nrobbed" );
  SG::AuxElement::Decorator<int> mDecor_intrk( "NumInputTrk" );

  do
  {
    seedBegin=seedTracks.begin();
    seedEnd=seedTracks.end();

    if (seedtracknumber ==0)
    {
      ATH_MSG_DEBUG( " New iteration. No tracks available after track selection for seeding. No finding done." );
      break;
    }
    
    m_iterations += 1;
    ATH_MSG_DEBUG( "ITERATION NUMBER " << m_iterations );
    
    //now find a new SEED
    
    std::vector<const Trk::TrackParameters*> perigeeList;
    for (std::vector<Trk::ITrackLink*>::iterator seedtrkAtVtxIter=seedBegin;
         seedtrkAtVtxIter!=seedEnd;++seedtrkAtVtxIter) {
      perigeeList.push_back( (*seedtrkAtVtxIter)->parameters()  );
    }

    int ncandi = 0 ;
    xAOD::Vertex theconstraint;
    
    
    ATH_MSG_DEBUG( " goto seed finder " );

    std::unique_ptr<Trk::IMode3dInfo> info;
    seedVertex = m_SeedFinder->findSeed(m_privtx.x(), m_privtx.y(),
                                        info, perigeeList);

    ATH_MSG_DEBUG( " seedFinder finished " );

#ifdef MONITORTUNES
    std::vector<float> FsmwX, FsmwY, FsmwZ, wght ;


    double cXY = -9.9, cZ = -9.9 ;
    info->getCorrelationDistance( cXY, cZ ) ;

    m_sdFsmwX->push_back( FsmwX ) ;
    m_sdFsmwY->push_back( FsmwY ) ;
    m_sdFsmwZ->push_back( FsmwZ ) ;
    m_sdcrsWght->push_back( wght ) ;

    m_seedX->push_back( seedVertex.x() ) ;
    m_seedY->push_back( seedVertex.y() ) ;
    m_seedZ->push_back( seedVertex.z() ) ;
    m_seedXYdist->push_back( cXY ) ;
    m_seedZdist->push_back( cZ ) ;
#endif



    Amg::MatrixX looseConstraintCovariance(3,3);
    looseConstraintCovariance.setIdentity();
    looseConstraintCovariance = looseConstraintCovariance * 1e+8;
    theconstraint = xAOD::Vertex();
    theconstraint.setPosition( seedVertex );
    theconstraint.setCovariancePosition( looseConstraintCovariance );
    theconstraint.setFitQuality( -99.9, 1.0 );
    
// on the other side VKalFitter  : Tracking/TrkVertexFitter/TrkVKalVrtFitter/src/VKalVrtFitSvc

    ATH_MSG_DEBUG( " seed at x: " << seedVertex.x() << 
         " at y: " <<      seedVertex.y() << 
         " at z: " <<      seedVertex.z() );
    
    
    if ( seedVertex.z()==0. ) {
      
      ATH_MSG_DEBUG( "No seed found: no further vertices in event" );
      ATH_MSG_DEBUG( "Number of input tracks: " << perigeeList.size() << " but no seed returned." );
      

#ifdef MONITORTUNES
      FillXcheckdefauls() ;
#endif

      break;
    }

#ifdef MONITORTUNES
    m_nperiseed->push_back( ncandi ) ;
    m_seedac->push_back( 1 ) ;
#endif

    //now question (remove tracks which are too far away??? I think so...)
    std::vector<const Trk::TrackParameters*> perigeesToFit;
    std::vector<const Trk::TrackParameters*> perigeesToFitSplitVertex;
    
    int numberOfTracks( perigeeList.size() );
    
    std::vector<const Trk::TrackParameters*>::const_iterator perigeeListBegin=perigeeList.begin();
    std::vector<const Trk::TrackParameters*>::const_iterator perigeeListEnd=perigeeList.end();
    
    int counter=0;
    for (std::vector<const Trk::TrackParameters*>::const_iterator perigeeListIter=perigeeListBegin;
         perigeeListIter!=perigeeListEnd;++perigeeListIter)
    {

      if (numberOfTracks<=2)
      {
        perigeesToFit.push_back(*perigeeListIter);
        counter+=1;
      }
      else if ( numberOfTracks <= 3 && !m_createSplitVertices )
      {
        perigeesToFit.push_back(*perigeeListIter);
        counter+=1;
      } 
      else if (numberOfTracks<=4*m_splitVerticesTrkInvFraction && m_createSplitVertices)
      {
  // few tracks are left, put them into the fit regardless their position!
        if (counter % m_splitVerticesTrkInvFraction == 0)
        {
          perigeesToFit.push_back(*perigeeListIter);
          counter+=1;
        }
        else
        {
          perigeesToFitSplitVertex.push_back(*perigeeListIter);
          counter+=1;
        }
      }
      else
      { //check first whether it is not too far away!
        
        double distance=0.;
        try
        {
          std::unique_ptr<Trk::PlaneSurface> mySurface=m_ImpactPoint3dEstimator->Estimate3dIP(*perigeeListIter,&seedVertex,distance);
          ATH_MSG_VERBOSE( " ImpactPoint3dEstimator done " );
        }
        catch (error::ImpactPoint3dEstimatorProblem err)
        {
          ATH_MSG_WARNING( " ImpactPoint3dEstimator failed to find minimum distance between track and vertex seed: " << err.p );
        }
        
        if (distance<0)
        {
          ATH_MSG_WARNING( " Distance between track and seed vtx is negative: " << distance );
        }
        
        const Trk::TrackParameters* myPerigee=(*perigeeListIter);
        
      //very approximate error
        double doe = 99999999.9 ;
        double error= 0.;
  
  if( myPerigee && myPerigee->covariance() )
  {
    error  =  std::sqrt((*myPerigee->covariance())(Trk::d0,Trk::d0)+
            (*myPerigee->covariance())(Trk::z0,Trk::z0));
        }//end of the security check
  
        if (error==0.)
        {
          ATH_MSG_ERROR( " Error is zero! " << distance );
          error=1.;
        }  
        doe = distance/error ;
        
        
        ATH_MSG_VERBOSE( " Distance between track and seed vtx: " << distance << " d/s(d) = " << 
        distance/error << " err " << error );
        
        
        if ( doe < m_significanceCutSeeding )
        {
          if (counter % m_splitVerticesTrkInvFraction == 0 || !m_createSplitVertices)
          {
            perigeesToFit.push_back(*perigeeListIter);
            counter+=1;
          }
          else
          {
            perigeesToFitSplitVertex.push_back(*perigeeListIter);
            counter+=1;
          }
        }

      }
    }    // end of loop for filling perigeeListIter into perigeesToFit

//  here is while-okay

    
    ATH_MSG_VERBOSE( " Considering n. " << perigeesToFit.size() << " tracks for the fit. " );
    if (m_createSplitVertices)
    {
      ATH_MSG_VERBOSE( " and n. " << perigeesToFitSplitVertex.size() << " tracks for split vertex fit. " );
    }
    
    
    m_ndf = -3. ;
    m_ntracks = 0;
    m_v0mass = -299.9 ;
    m_v0ee = -299.9 ;
    m_dir = -99999.9 ;
    m_hif = -1.0 ;

    if (perigeesToFit.empty())
    {
      
      ATH_MSG_DEBUG( " No good seed found. Exiting search for vertices..." );
      
#ifdef MONITORTUNES
      xAOD::Vertex * seededxAODVertex = new xAOD::Vertex;
      theVertexContainer->push_back( seededxAODVertex ); // 

      seededxAODVertex->setPosition( seedVertex );
      Amg::MatrixX dummyCovariance(3,3);
      dummyCovariance.setIdentity();
      seededxAODVertex->setCovariancePosition( dummyCovariance );
      seededxAODVertex->vxTrackAtVertex() = std::vector<Trk::VxTrackAtVertex>();
      seededxAODVertex->setFitQuality( 99.9, m_ndf ) ;
      seededxAODVertex->setVertexType( xAOD::VxType::NotSpecified );

      mDecor_mass( *seededxAODVertex ) = m_v0mass ;
      mDecor_energy( *seededxAODVertex ) = m_v0ee ;
      mDecor_NumTrk( *seededxAODVertex ) = m_ntracks ;
      mDecor_HitsFilter( *seededxAODVertex ) = m_hif ;
      mDecor_direction( *seededxAODVertex ) = m_dir ;
      mDecor_intrk( *seededxAODVertex ) = numberOfTracks ;
#endif
      
      break;
    }
    // one has to break off the loop if no seed is found 

    //now you have perigeeToFit and perigeeToFitSplitVertices
    //AND HERE YOU DO THE FIT
    //to reassign vertices you look ino what is already in myVxCandidate
    //you do it only ONCE!

    xAOD::Vertex * myxAODVertex = nullptr;


    if ( perigeesToFit.size()>1) // our main use case
    {
      myxAODVertex=m_iVertexFitter->fit( perigeesToFit, seedVertex );
    }

    // 
    // The fitter neither tell whether the vertex is good or not, but the chi2/dof 
    // 

    countTracksAndNdf( myxAODVertex, m_ndf, m_ntracks);

    m_v0mass = -199.9 ;

    bool goodVertex = myxAODVertex != nullptr && m_ndf >0 && m_ntracks >=2 ;

    
    ATH_MSG_DEBUG( " xAOD::Vertex : " << ( myxAODVertex != nullptr ? 1 : 0 ) 
         << ",  #dof = " << m_ndf << ",  #tracks (weight>0.01) = " << m_ntracks );
    

//  below is while-okay
    if (!goodVertex)
    {
      
      ATH_MSG_DEBUG( " Going to new iteration with: " << seedTracks.size() << " seed tracks after BAD VERTEX. " );
           

      if (myxAODVertex)
      {
// DUMMY !!  Only for validation study
        removeCompatibleTracks( myxAODVertex,  perigeesToFit,  seedTracks);
#ifdef MONITORTUNES
        myxAODVertex->setVertexType( xAOD::VxType::KinkVtx );
        theVertexContainer->push_back( myxAODVertex ); 
        mDecor_mass( *myxAODVertex ) = m_v0mass ;
        mDecor_energy( *myxAODVertex ) = m_v0ee ;
        mDecor_NumTrk( *myxAODVertex ) = m_ntracks ;
        mDecor_HitsFilter( *myxAODVertex ) = m_hif ;
        mDecor_direction( *myxAODVertex ) = m_dir ;
        mDecor_intrk( *myxAODVertex ) = numberOfTracks ;
#else
        delete myxAODVertex;
        myxAODVertex=0;
#endif
      }
      else 
      {
        removeAllFrom( perigeesToFit, seedTracks );
#ifdef MONITORTUNES
        xAOD::Vertex * seededxAODVertex = new xAOD::Vertex;
        theVertexContainer->push_back( seededxAODVertex ); // have to add vertex to container here first so it can use its aux store

        seededxAODVertex->setPosition( seedVertex );
        Amg::MatrixX dummyCovariance(3,3);
        dummyCovariance.setIdentity();
        seededxAODVertex->setCovariancePosition( dummyCovariance );
        seededxAODVertex->vxTrackAtVertex() = std::vector<Trk::VxTrackAtVertex>();
        seededxAODVertex->setFitQuality( 99.9, m_ndf ) ;
        seededxAODVertex->setVertexType( xAOD::VxType::NoVtx );

        mDecor_mass( *seededxAODVertex ) = m_v0mass ;
        mDecor_energy( *seededxAODVertex ) = m_v0ee ;
        mDecor_NumTrk( *seededxAODVertex ) = m_ntracks ;
        mDecor_HitsFilter( *seededxAODVertex ) = m_hif ;
        mDecor_direction( *seededxAODVertex ) = m_dir ;
        mDecor_intrk( *seededxAODVertex ) = numberOfTracks ;
#endif
     }

     continue ;  // try next seed 

    }

//  here is a boundary   XXXXXXXXXXXXXX, at least a "good" vertex is found
    // Now the goodVertex could be not so good ...
    mDecor_nrobbed( *myxAODVertex ) = 0 ;
    if ( m_reassignTracksAfterFirstFit && ( ! m_createSplitVertices ) )
    {

      
      ATH_MSG_VERBOSE( " N tracks used for fit before reallocating: "  << perigeesToFit.size() );
      
        //now you HAVE a good vertex
        //but you want to add the tracks which you missed...
        
      int numberOfAddedTracks=0;
      const AmgSymMatrix(3) covariance = (&(*myxAODVertex))->covariancePosition() ;
      const Amg::Vector3D position = (&(*myxAODVertex))->position() ;

        //iterate on remaining vertices and cross-check if tracks need to be attached 
        //to new vertex
      xAOD::VertexContainer::iterator vxBegin=theVertexContainer->begin();
      xAOD::VertexContainer::iterator vxEnd=theVertexContainer->end();
 
      for (xAOD::VertexContainer::iterator vxIter=vxBegin;vxIter!=vxEnd;++vxIter)
      {
          //  A vertex should not rob tracks from itself
#ifdef MONITORTUNES
        if ( (*vxIter)->vertexType() ==  xAOD::VxType::NoVtx ) continue ;
#endif
        std::vector<Trk::VxTrackAtVertex>* myVxTracksAtVtx=(&(*vxIter)->vxTrackAtVertex());
          
        if ( ! myVxTracksAtVtx ) continue;

        int nrobbed = 0 ;
    
        const AmgSymMatrix(3) oldcovariance = (*vxIter)->covariancePosition() ;
        const Amg::Vector3D oldposition = (*vxIter)->position() ;

          //now iterate on tracks at vertex
        std::vector<Trk::VxTrackAtVertex>::const_iterator tracksBegin=myVxTracksAtVtx->begin();
        std::vector<Trk::VxTrackAtVertex>::const_iterator tracksEnd=myVxTracksAtVtx->end();

        
        ATH_MSG_VERBOSE( " Iterating over new vertex to look for tracks to reallocate... "  );
        
          
        for (std::vector<Trk::VxTrackAtVertex>::const_iterator tracksIter=tracksBegin;
             tracksIter!=tracksEnd;)
        {
            
          //only try with tracks which are not too tightly assigned to another vertex
          if ((*tracksIter).weight() > m_minWghtAtVtx )
          {
            ++tracksIter;
            continue;
          }
            
          const Trk::TrackParameters* trackPerigee=(*tracksIter).initialPerigee();
                        
          if (trackPerigee==nullptr)
          {
            ATH_MSG_ERROR( " Cast to perigee gives 0 pointer, cannot continue " );
            return invalidResponse;
;
          }
            
          double chi2_newvtx=compatibility(*trackPerigee, covariance, position );
          double chi2_oldvtx=compatibility(*trackPerigee, oldcovariance, oldposition );

          double minDist = VrtVrtDist( myxAODVertex,  *vxIter ) ;

          
          ATH_MSG_VERBOSE( "Compatibility to old vtx is : " << chi2_oldvtx << 
           " to new vtx is: " << chi2_newvtx );
          

          bool remove=false;

          if (     chi2_newvtx < chi2_oldvtx 
               &&  (  m_filterLevel < 2 || minDist > m_minVtxDist ) 
             )
          {
              
            
            ATH_MSG_DEBUG( " Found track of old vertex (chi2= " << chi2_oldvtx << 
           ") more compatible to new one (chi2= " << chi2_newvtx << ")" );
            
              
            perigeesToFit.push_back(trackPerigee);
            //but you need to re-add it to the seedTracks too...

            bool isFound=false;

            std::vector<Trk::ITrackLink*>::iterator  origBegin=origTracks.begin();
            std::vector<Trk::ITrackLink*>::iterator  origEnd=origTracks.end();
            
            for (std::vector<Trk::ITrackLink*>::iterator origIter=origBegin;
                 origIter!=origEnd;++origIter)
            {
              if ( (*origIter)->parameters()==trackPerigee )
              {
                
                ATH_MSG_VERBOSE( " found the old perigee to be re-added to seedTracks in order to be deleted again!" );
                
                isFound=true;
                seedTracks.push_back(*origIter);
                break;
              }
            }

            if (!isFound)
            {
              ATH_MSG_WARNING( " Cannot find old perigee to re-add back to seed tracks... " );
            }             
              
            numberOfAddedTracks+=1;

            remove=true;
          }
            
          if (remove)
          {
            //now you have to delete the track from the old vertex...
            //easy???
            nrobbed ++ ;
            tracksIter=myVxTracksAtVtx->erase(tracksIter);
            tracksBegin=myVxTracksAtVtx->begin();
            tracksEnd=myVxTracksAtVtx->end();
          }
          else
          {
            ++tracksIter;
          }
        }//end of iterating on tracks at previous vertices

        if ( nrobbed > 0 )  mDecor_nrobbed( *(*vxIter) ) = (*vxIter)->auxdata<int>("nrobbed") + 1 ;

      }//end of iterating on already found vertices in event

      
      ATH_MSG_VERBOSE( " N tracks used for fit after reallocating: "  << perigeesToFit.size() );
      

        //now you have to delete the previous xAOD::Vertex, do a new fit, i
        // then check if you still have a good vertex

      if ( numberOfAddedTracks > 0 )
      {
          
        ATH_MSG_DEBUG( " refit with additional " << numberOfAddedTracks 
           << " from other vertices " );

        Amg::Vector3D fitposition = myxAODVertex->position() ;
        delete myxAODVertex;

        myxAODVertex=nullptr;

        if ( perigeesToFit.size()>1)
        {
          myxAODVertex=m_iVertexFitter->fit( perigeesToFit, fitposition ) ;
          mDecor_nrobbed( *myxAODVertex ) = 0 ; //  robbing but also refit
        }        
          
        countTracksAndNdf( myxAODVertex, m_ndf, m_ntracks);
          
        goodVertex = myxAODVertex != nullptr && m_ndf >0 && m_ntracks >=2 ;

        
        ATH_MSG_DEBUG( " Refitted xAODVertex is pointer: " << myxAODVertex << 
       " #dof = " << m_ndf << " #tracks (with weight>0.01) " << m_ntracks );
        

        if ( ! goodVertex )
        {
// it was ever good vertex at least, it become nothing after eating new a track ...         
          if (myxAODVertex)
          {
            removeCompatibleTracks( myxAODVertex,  perigeesToFit,  seedTracks);
#ifdef MONITORTUNES
            theVertexContainer->push_back(myxAODVertex);
            myxAODVertex->setVertexType( xAOD::VxType::KinkVtx );

            mDecor_mass( *myxAODVertex ) = m_v0mass ;
            mDecor_energy( *myxAODVertex ) = m_v0ee ;
            mDecor_HitsFilter( *myxAODVertex ) = m_hif ;
            mDecor_direction( *myxAODVertex ) = m_dir ;
            mDecor_NumTrk( *myxAODVertex ) = m_ntracks ;
            mDecor_intrk( *myxAODVertex ) = numberOfTracks ;
#else
            delete myxAODVertex;
            myxAODVertex=0;
#endif
          }  
          else
          { 
            removeAllFrom(perigeesToFit,seedTracks);
            
            ATH_MSG_DEBUG( " Adding tracks resulted in an invalid vertex. Should be rare... " );
            ATH_MSG_DEBUG( " Going to new iteration with " << seedTracks.size() 
         << " seed tracks after BAD VERTEX. " );

#ifdef MONITORTUNES

            xAOD::Vertex * seededxAODVertex = new xAOD::Vertex;
            theVertexContainer->push_back( seededxAODVertex ); 
            seededxAODVertex->setPosition( position );
            seededxAODVertex->setCovariancePosition( covariance  );
            seededxAODVertex->vxTrackAtVertex() = std::vector<Trk::VxTrackAtVertex>();

            seededxAODVertex->setVertexType( xAOD::VxType::NoVtx );
            seededxAODVertex->setFitQuality( 99.9, m_ndf ) ;
            mDecor_NumTrk( *seededxAODVertex ) = m_ntracks ;
            mDecor_mass( *seededxAODVertex ) = m_v0mass ;
            mDecor_energy( *seededxAODVertex ) = m_v0ee ;
            mDecor_HitsFilter( *seededxAODVertex ) = m_hif ;
            mDecor_direction( *seededxAODVertex ) = m_dir ;
            mDecor_intrk( *seededxAODVertex ) = numberOfTracks ;
#endif
          }
          continue ;

        }
      }// end if tracks were added...

    }//end reassign tracks from previous vertices and refitting if needed


//  here is a boundary   YYYYYYYYYYYYYYYYY,  a "good" vertex is anyway found
    m_v0mass = -99.9 ;

    myxAODVertex->setVertexType( xAOD::VxType::SecVtx );

    countTracksAndNdf( myxAODVertex, m_ndf, m_ntracks );
    bool isv0 = (   m_ntracks < 2 ? false  
                  : V0kine( getVertexMomenta( myxAODVertex ) , 
                                    (&(*myxAODVertex))->position(), m_v0mass, m_dir ) ) ;

#ifdef MONITORTUNES 
    mDecor_HitsFilter( *myxAODVertex ) = m_hif ;
    mDecor_direction( *myxAODVertex ) = m_dir ;
    mDecor_NumTrk( *myxAODVertex ) = m_ntracks ;
#else
    if (     (  m_filterLevel >= 3 && isv0 )
          || (  m_filterLevel >= 2 && m_dir < m_privtxRef ) 
       )
    {
        removeCompatibleTracks( myxAODVertex,  perigeesToFit,  seedTracks);
        delete myxAODVertex;
        myxAODVertex=0;
        continue ;   // fakes, just skip and have its tracks removed from seeds
    }
#endif

//  now we arrived at a V0 tagging :
    mDecor_mass( *myxAODVertex ) = m_v0mass ;
    mDecor_energy( *myxAODVertex ) = m_v0ee ;
    mDecor_intrk( *myxAODVertex ) = numberOfTracks ;
    if ( isv0 ) myxAODVertex->setVertexType(xAOD::VxType::V0Vtx);

// Now check the hits inner and outer the seed position 
    m_hif = 0. ; 
#ifdef MONITORTUNES
    if ( m_filterLevel > 4 && myxAODVertex->vertexType() != xAOD::VxType::V0Vtx )
#else
    if ( m_filterLevel > 4 && m_ntracks == 2 && myxAODVertex->vertexType() != xAOD::VxType::V0Vtx )
#endif
    {

      xAOD::Vertex * groomed = new xAOD::Vertex() ;
      * groomed = * myxAODVertex ;
      m_hif = removeTracksInBadSeed( groomed, perigeesToFit ) ;

      ATH_MSG_DEBUG( " radiiPattern Filter : " << m_hif <<" "<< m_ntracks );

#ifndef MONITORTUNES 
      if (  m_hif > m_CutHitsFilter ) 
      {
        removeCompatibleTracks( groomed,  perigeesToFit,  seedTracks);
        delete groomed ;
        groomed = 0 ;
        delete myxAODVertex ;
        myxAODVertex=0;
        continue ;
      }
#endif

      if ( m_hif > 0 ) 
      {
        bool goodgroom = false ;
        int ngroom = 0 ;
        if ( perigeesToFit.size() >= 2 ) 
        {
          groomed = nullptr;
          groomed=m_iVertexFitter->fit( perigeesToFit, myxAODVertex->position()  ) ;

          countTracksAndNdf( groomed, m_ndf, ngroom );

          goodgroom = ( groomed != nullptr && m_ndf > 0 && ngroom >= 2 ) ;
        }

        if ( perigeesToFit.size() < 2 ||  ! goodgroom  )
        {
#ifdef MONITORTUNES
          theVertexContainer->push_back(myxAODVertex);

          if ( perigeesToFit.size() < 2 || ngroom < 1 ) 
            myxAODVertex->setVertexType( xAOD::VxType::NoVtx );
          else 
            myxAODVertex->setVertexType( xAOD::VxType::KinkVtx );

          mDecor_HitsFilter( *myxAODVertex ) = m_hif ;
          mDecor_NumTrk( *myxAODVertex ) = m_ntracks ;
#endif
          if ( groomed )
          {
//              removeCompatibleTracks( groomed,  perigeesToFit,  seedTracks);
            delete groomed ;
            groomed=nullptr;
          }

          removeCompatibleTracks( myxAODVertex,  perigeesToFit,  seedTracks);
          continue ;  // a vertex went bad and skipped
        }

// found a good vertex after groom and refit, re-check V0 with reduced tracks
        * myxAODVertex = * groomed ;
//        myxAODVertex->setTrackParticleLinks( groomed->trackParticleLinks() ) ;

        delete groomed ;
        groomed=nullptr;

        ATH_MSG_DEBUG( " new vertex after grooming with reminded tracks : " << ngroom  
           << " with Chi2/dof " << myxAODVertex->chiSquared()/myxAODVertex->numberDoF() );

        isv0 = false ;
        if ( ngroom == 2 ) 
          isv0 = V0kine( getVertexMomenta( myxAODVertex ) , (&(*myxAODVertex))->position(), m_v0mass, m_dir ) ;

#ifndef MONITORTUNES 
        if (  isv0 ||  m_dir < m_privtxRef )
        {
          removeCompatibleTracks( myxAODVertex,  perigeesToFit,  seedTracks);
          delete myxAODVertex;
          myxAODVertex=0;
          continue ;   // fakes, just skip and have its tracks removed from seeds
        }
#endif
        if ( isv0 ) 
          myxAODVertex->setVertexType( xAOD::VxType::V0Vtx );
        else 
          myxAODVertex->setVertexType( xAOD::VxType::SecVtx );

        mDecor_mass( *myxAODVertex ) = m_v0mass ;
        mDecor_energy( *myxAODVertex ) = m_v0ee ;
        mDecor_intrk( *myxAODVertex ) = numberOfTracks ;
#ifdef MONITORTUNES 
        mDecor_HitsFilter( *myxAODVertex ) = m_hif ;
        mDecor_direction( *myxAODVertex ) = m_dir ;
        mDecor_NumTrk( *myxAODVertex ) = ngroom ;
#endif

        ATH_MSG_DEBUG( " radiiPattern filter worked with VxType : " << myxAODVertex->vertexType() );

      } else
      {
        ATH_MSG_DEBUG( " radiiPattern filter hasn't affect in tracks : " << m_ntracks 
           << " with VxType : " << myxAODVertex->vertexType() );
        delete groomed ;
        groomed = nullptr ;
#ifdef MONITORTUNES
        mDecor_HitsFilter( *myxAODVertex ) = m_hif ;
#endif

      }

    }
  
    removeCompatibleTracks( myxAODVertex,  perigeesToFit,  seedTracks);
    // SeedTracks are updated so that the remained could be seeded/fit in next iteration

    
    ATH_MSG_VERBOSE( "Number of seeds after removal of outliers: " << seedTracks.size() );
    
      
    if ( ! m_createSplitVertices )
    {
      theVertexContainer->push_back(myxAODVertex);
      ATH_MSG_DEBUG( " A GOOD vertex is stored in its container with tracks : " << m_ntracks );
    }
   //  there is no necessary to split for 2ndVtx... lets forget it.

  } while ( seedTracks.size() > 1 && m_iterations < m_maxVertices ) ;

  ATH_MSG_DEBUG( " Iterations done " ) ;

  //unfortunately you have still a problem with the track to links!!!
  
  xAOD::VertexContainer::iterator vxBegin=theVertexContainer->begin();
  xAOD::VertexContainer::iterator vxEnd=theVertexContainer->end();
  
  //prepare iterators for tracks only necessary for seeding
  std::vector<Trk::ITrackLink*>::iterator origtrkbegin=origTracks.begin();
  std::vector<Trk::ITrackLink*>::iterator origtrkend=origTracks.end();
  
  // refit a vertex if there were tracks eventualy ever removed/robbed from it
//  for (xAOD::VertexContainer::const_iterator vxIter=vxBegin;vxIter!=vxEnd;++vxIter)
  for (xAOD::VertexContainer::iterator vxIter=vxBegin;vxIter!=vxEnd;)
  {
#ifdef MONITORTUNES
    if ( (*vxIter)->vertexType() ==  xAOD::VxType::NoVtx || (*vxIter)->vertexType() ==  xAOD::VxType::KinkVtx  ) 
    {
      ++vxIter ;
      continue ;
    }
#endif
    std::vector<Trk::VxTrackAtVertex>* myVxTracksAtVtx = &((*vxIter)->vxTrackAtVertex());
    if ( !myVxTracksAtVtx ) 
    {
      ++vxIter ;
      continue ;
    }

    int nrobbed = (*vxIter)->auxdata<int>("nrobbed") ;
    if ( nrobbed < 1 ) 
    {
      ++vxIter ;
      continue ;
    }

    std::vector<Trk::VxTrackAtVertex>::const_iterator tracksBegin=myVxTracksAtVtx->begin();
    std::vector<Trk::VxTrackAtVertex>::const_iterator tracksEnd=myVxTracksAtVtx->end();

    std::vector<const Trk::TrackParameters*> perigeesToFit ; 
    std::vector<Trk::ITrackLink*> nullseedTracks ;
    int ntrk = 0 ;
    for (std::vector<Trk::VxTrackAtVertex>::const_iterator tracksIter=tracksBegin;
         tracksIter!=tracksEnd;++tracksIter)
    {
      for (std::vector<Trk::ITrackLink*>::const_iterator origtrkiter=origtrkbegin;
             origtrkiter!=origtrkend;++origtrkiter)
      {
        if ((*origtrkiter)->parameters()==(*tracksIter).initialPerigee())
        {
          if ( (*tracksIter).weight() > m_minWghtAtVtx ) ntrk ++ ;
          nullseedTracks.push_back( *origtrkiter ) ;
          perigeesToFit.push_back( (*origtrkiter)->parameters() ) ;
        }
      }
    }
// redo the fit
    xAOD::Vertex * QxAODVertex = new xAOD::Vertex() ;
    QxAODVertex = m_iVertexFitter->fit( perigeesToFit, (*vxIter)->position() );
    if ( QxAODVertex == nullptr ) 
    {
      ++vxIter ;
      continue ;
    }

    removeCompatibleTracks( QxAODVertex,  perigeesToFit,  nullseedTracks);

    float chi2dof1 = (*vxIter)->chiSquared()/(*vxIter)->numberDoF() ;
    float chi2dof2 = QxAODVertex->chiSquared()/QxAODVertex->numberDoF() ;

    float oldhf = 0. ;
    if ( (*vxIter)->isAvailable<float>( "radiiPattern" ) )
      oldhf = (*vxIter)->auxdata<float>( "radiiPattern" ) ;
    if ( chi2dof2 >=  chi2dof1 ) 
    {
      ++vxIter ;
      continue ;
    }

    int nit = (*vxIter)->auxdata<int>( "NumInputTrk" ) ;
    *(*vxIter) = *QxAODVertex ;

    bool isv0 = V0kine( getVertexMomenta( QxAODVertex ), (&(*QxAODVertex))->position(), m_v0mass, m_dir ) ;
    mDecor_mass( *(*vxIter) ) = m_v0mass ;
    mDecor_energy( *(*vxIter) ) = m_v0ee ;
    mDecor_intrk( *(*vxIter) ) = nit ;

    if ( isv0 &&  ntrk == 2 ) (*vxIter)->setVertexType(xAOD::VxType::V0Vtx);

#ifdef MONITORTUNES
    mDecor_HitsFilter( *(*vxIter) ) = oldhf ;
    mDecor_direction( *(*vxIter) ) = m_dir ;
    mDecor_NumTrk( *(*vxIter) ) = ntrk ;
#else
    if (     (  m_filterLevel >= 3 && m_ntracks == 2 && isv0 )
          || (  m_filterLevel > 2 && m_dir < m_privtxRef ) 
       )
    {
      vxIter = theVertexContainer->erase( vxIter ) ;
      vxBegin=theVertexContainer->begin();
      vxEnd=theVertexContainer->end();     
    } 
#endif

    ++ vxIter ; 
   

    delete QxAODVertex ;
  }

  // update vertices, while fix the TrackParticles linked to the vertex
  int nv = 0 ;

  for (xAOD::VertexContainer::iterator vxIter=vxBegin;vxIter!=vxEnd;++vxIter, nv++)
  {

    ATH_MSG_DEBUG( " filled " << nv << " 'th vertex : x= " << (*vxIter)->position().x() <<" , y= " 
                    << (*vxIter)->position().y() <<" , z= " << (*vxIter)->position().z() 
       << " vxType = " << (*vxIter)->vertexType() ) ;

    std::vector<float> trkWght ;

   std::vector<Trk::VxTrackAtVertex>* myVxTracksAtVtx = &((*vxIter)->vxTrackAtVertex());

    if ( ! myVxTracksAtVtx ) 
    {
#ifdef MONITORTUNES
      mDecor_trkDOE( *(*vxIter) ) = trkWght ;
      mDecor_trkWght( *(*vxIter)  ) = trkWght ;
      mDecor_sumPt2( *(*vxIter)  ) = -99.9 ;
      mDecor_nrobbed( *(*vxIter)  ) = 0 ;
#endif
      continue ;
    }

    float pt2 = 0.0 ;
    std::vector<float> xdoe ;

    std::vector<Trk::VxTrackAtVertex>::iterator tracksBegin=myVxTracksAtVtx->begin();
    std::vector<Trk::VxTrackAtVertex>::iterator tracksEnd=myVxTracksAtVtx->end();

    for (std::vector<Trk::VxTrackAtVertex>::iterator tracksIter=tracksBegin;
         tracksIter!=tracksEnd;++tracksIter)
    {
      bool found = false ;

      trkWght.push_back( (*tracksIter).weight() ) ;

      //now look for corresponding ITrackLink
      for (std::vector<Trk::ITrackLink*>::iterator origtrkiter=origtrkbegin;
           origtrkiter!=origtrkend;++origtrkiter)
      {

        if ((*origtrkiter)->parameters()==(*tracksIter).initialPerigee())
        {

          found=true;

          const Trk::TrackParameters * svperigee =  (*tracksIter).perigeeAtVertex()  ;

          if ( svperigee ) 
          {
            ATH_MSG_VERBOSE( " svperigee retrieved " );

            float pt = svperigee->parameters()[Trk::theta]/std::abs( svperigee->parameters()[Trk::qOverP] ) ;
            ATH_MSG_VERBOSE( " track perigee parameters retrieved " );
            pt2 += pt*pt*0.000001 ;

#ifdef MONITORTUNES
            double distance=0.;
            try
            {
              std::unique_ptr<Trk::PlaneSurface> mySurface= 
                   m_ImpactPoint3dEstimator->Estimate3dIP( (*tracksIter).initialPerigee(), &((*vxIter)->position()), distance );
            }
            catch (error::ImpactPoint3dEstimatorProblem err)
            {
              ATH_MSG_WARNING( " ImpactPoint3dEstimator failed  " << err.p );
            }
        
            double error= 1. ;
  
            if( svperigee->covariance() )
              error  =  std::sqrt(   (*svperigee->covariance())(Trk::d0,Trk::d0) 
                                   + (*svperigee->covariance())(Trk::z0,Trk::z0)
                                 ); 
            xdoe.push_back( std::abs( distance/error ) ) ;
#endif
          } 

          //assigning the input track to the fitted vertex through VxTrackAtVertices vector  
    (*tracksIter).setOrigTrack (*origtrkiter );
    
          // See if the trklink is to an xAOD::TrackParticle
          Trk::LinkToXAODTrackParticle* linkToXAODTP = dynamic_cast<Trk::LinkToXAODTrackParticle*>( *origtrkiter );

          // If track is an xAOD::TrackParticle, set directly in xAOD::Vertex
          if (linkToXAODTP)
          {     
      (*vxIter)->addTrackAtVertex(*linkToXAODTP, (*tracksIter).weight());

          } //TODO: else write in a warning? (if tracks were Trk::Tracks or Trk::TrackParticleBase)

          origTracks.erase(origtrkiter);
          origtrkbegin=origTracks.begin();
          origtrkend=origTracks.end();
          break;
        }  // end of matched 
      } // end of matching loop over original trakcs

      if (!found)
      {
        ATH_MSG_ERROR( " Cannot find vector element to fix links (step 4)! " );
      }

    }//end iterate on tracks at vtx

// update the decorations 

   const xAOD::Vertex * vtx = (*vxIter);

#ifdef MONITORTUNES
    mDecor_trkWght( *vtx ) = trkWght ;
    mDecor_trkDOE( *vtx ) = xdoe ;
#endif

    mDecor_sumPt2( *vtx ) = pt2 ;

  }//end iterate on vertices
  
  ATH_MSG_DEBUG(" #Vtx "<< theVertexContainer->size() <<" with track-vertex association fixed " );

  
  for (unsigned int i = 0 ; i < theVertexContainer->size() ; i++)
  {

    ATH_MSG_VERBOSE( " Vtx: " << i <<
        " x= " << (*theVertexContainer)[i]->position().x() <<
        " y= " << (*theVertexContainer)[i]->position().y() <<
        " z= " << (*theVertexContainer)[i]->position().z() <<
        " ntracks= " << (*theVertexContainer)[i]->vxTrackAtVertex().size() <<
        " chi2= " << (*theVertexContainer)[i]->chiSquared() << 
         " #dof = " << (*theVertexContainer)[i]->numberDoF() );
  }

  for (std::vector<Trk::ITrackLink*>::iterator origtrkiter=origtrkbegin;
       origtrkiter!=origtrkend;++origtrkiter)
  {   
    if ((*origtrkiter)!=0) {
      delete *origtrkiter;
      *origtrkiter=0;
    }
  }

#ifdef MONITORTUNES
  m_OTree->Fill() ;
#endif

  return std::make_pair(theVertexContainer, theVertexAuxContainer);
}

bool InDetIterativeSecVtxFinderTool::V0kine( const std::vector< Amg::Vector3D > &momenta, const Amg::Vector3D & posi, 
                                             float & mass, float &modir ) const
{
  mass = -99.9 ;
  modir = -99999.9 ;

  int ntrk = momenta.size() ;

  if ( ntrk < 2 ) 
  {
    ATH_MSG_DEBUG( " ntrk < 2 , Meaningless to test mass " );
    return false ;
  }

  std::vector<double> Pv (ntrk);
  double vx = 0., vy = 0., vz = 0., eK0 = 0. ;
  double pi2 = 139.57018*139.57018 ;   // Pion in MeV

  for ( int t = 0 ; t < ntrk ; t ++ )
  {
    Amg::Vector3D trk = momenta[t] ;

    vz += trk.z() ;
    vx += trk.x() ;
    vy += trk.y() ;
    Pv[t] = trk.x()*trk.x() + trk.y()*trk.y() + trk.z()*trk.z() ;
    eK0 += std::sqrt( Pv[t] + pi2 ) ; 
  }

  double mnt2 = vx*vx + vy*vy + vz*vz ;
  mass = eK0*eK0 - mnt2 ;
  mass = 0.001*( mass >= 0 ? std::sqrt( mass ) : std::sqrt( -mass ) ) ;

  Amg::Vector3D vdif = posi - m_privtx ;
  Amg::Vector3D vmoment =  Amg::Vector3D( vx, vy, vz ) ;
  
  modir = vmoment.dot( vdif )/std::sqrt( mnt2 )  ;

  // borrowed from InnerDetector/InDetRecAlgs/InDetV0Finder/InDetV0FinderTool
  double a0z = ( vdif + vmoment*vmoment.dot( vdif )/( mnt2 + 0.00001 ) ).z() ;
  double Rxy = vdif.perp() ;

  ATH_MSG_DEBUG( " V0kine : a0z = " << a0z << " Rxy = " << Rxy <<" direction  "<< modir );

  if ( ntrk != 2 )
  {
    ATH_MSG_VERBOSE( " ntrk != 2 , Meaningless to test V0 " );
    return false ;
  }

  if (  a0z > 15. || Rxy > 500. ) { return false ; }

  // 1 eV^(-1) of time = hbar / eV = 6.582173*10^(-16) second,  for energy-time in natural unit
//  double planck = 6.582173 ;      

  double eGam = std::sqrt( Pv[0] + 0.511*0.511 ) + std::sqrt( Pv[1] + 0.511*0.511 ) ;
  double mGam = eGam*eGam - mnt2 ;
 
  double prtn2 = 938.27205*938.27205 ;
  double eLam = Pv[0] > Pv[1] ?  std::sqrt( Pv[0] + prtn2 ) + std::sqrt( Pv[1] + pi2 ) : 
                          sqrt( Pv[0] + pi2 ) + std::sqrt( Pv[1] + prtn2 )  ;
  double mLam = eLam*eLam - mnt2 ;

  ATH_MSG_DEBUG( " V0 masses : " << mass 
                 <<" "<< ( mGam >= 0 ? std::sqrt( mGam ) : std::sqrt( -mGam ) )
                 <<" "<< ( mLam >= 0 ? std::sqrt( mLam ) : std::sqrt( -mLam ) ) );

  if (   ( std::abs( mass - 497.614 ) < 100. )   // K short 
      || ( mGam > 0 && sqrt( mGam ) < 40. )  // gamma conversion ;
      || ( mLam > 0 && std::abs( sqrt( mLam ) - 1115.683 ) < 200.  ) //  Lambda 
     )  return true ;

  return false ;
}



double InDetIterativeSecVtxFinderTool::VrtVrtDist( xAOD::Vertex * v1, xAOD::Vertex * v2 ) 
{
  double dist = 50 ;

  Amg::Vector3D vdif = v1->position() - v2->position() ;
  AmgSymMatrix(3) vErrs = v1->covariancePosition() + v2->covariancePosition() ;

  vErrs = vErrs.inverse().eval(); 
   
  dist = vdif.dot( vErrs * vdif ) ;

  return dist ;

}


bool InDetIterativeSecVtxFinderTool::passHitsFilter( 
      const Trk::TrackParameters* perigee, float rad, float absz ) const
{
  bool pass = true ;

  //  PrimaryVtx information is used, who was assumpted be always be there
  ATH_MSG_DEBUG( " seed input : " << rad <<" "<< absz );

/**   Borrowed from Reconstruction/VKalVrt/VrtSecInclusive ***/

      //
      // rough guesses for active layers:
      // BeamPipe: 23.5-24.3
      // IBL: 31.0-38.4
      // Pix0 (BLayer): 47.7-54.4, Pix1: 85.5-92.2, Pix2: 119.3-126.1
      // Sct0: 290-315, Sct1: 360-390, Sct2: 430-460, Sct3:500-530
      //

  enum vertexArea {
    insideBeamPipe,

    insidePixelBarrel0,
    aroundPixelBarrel0,
  
    outsidePixelBarrel0_and_insidePixelBarrel1,
    aroundPixelBarrel1,
  
    outsidePixelBarrel1_and_insidePixelBarrel2,
    aroundPixelBarrel2,
  
    outsidePixelBarrel2_and_insidePixelBarrel3,
    aroundPixelBarrel3,
  
    outsidePixelBarrel3_and_insideSctBarrel0,
    aroundSctBarrel0,
  
    outsideSctBarrel0_and_insideSctBarrel1,
    aroundSctBarrel1,

    insideSilicon
  };

  int vertex_pattern = 0;
  if( rad < 23.50 ) {
    vertex_pattern = insideBeamPipe;
  } else if( rad < 31.0 && absz < 331.5 ) {
    vertex_pattern = insidePixelBarrel0;
  } else if( rad < 38.4 && absz < 331.5 ) {
    vertex_pattern = aroundPixelBarrel0;
  } else if( rad < 47.7 && absz < 400.5 ) {
    vertex_pattern = outsidePixelBarrel0_and_insidePixelBarrel1;
  } else if( rad < 54.4 && absz < 400.5 ) {
    vertex_pattern = aroundPixelBarrel1;
  } else if( rad < 85.5 && absz < 400.5 ) {
    vertex_pattern = outsidePixelBarrel1_and_insidePixelBarrel2;
  } else if( rad < 92.2 && absz < 400.5 ) {
    vertex_pattern = aroundPixelBarrel2;
  } else if( rad < 119.3 && absz < 400.5 ) {
    vertex_pattern = outsidePixelBarrel2_and_insidePixelBarrel3;
  } else if( rad < 126.1 && absz < 400.5 ) {
    vertex_pattern = aroundPixelBarrel3;
  } else if( rad < 290 && absz < 749.0 ) {
    vertex_pattern = outsidePixelBarrel3_and_insideSctBarrel0;
  } else if( rad < 315 && absz < 749.0 ) {
    vertex_pattern = aroundSctBarrel0;
  } else if( rad < 360 && absz < 749.0 ) {
    vertex_pattern = outsideSctBarrel0_and_insideSctBarrel1;
  } else if( rad < 390 && absz < 749.0 ) {
    vertex_pattern = aroundSctBarrel1;
  } else {
    vertex_pattern = insideSilicon ;
  }
      


  for ( unsigned int p = 0 ; p < m_trkdefiPars.size() ; p++ )
  { 

    Amg::VectorX tpperigee = m_trkdefiPars.at(p) ;

    if (    tpperigee[4] != perigee->parameters()[Trk::qOverP]
            ||  tpperigee[3] != perigee->parameters()[Trk::theta]
            ||  tpperigee[2]  != perigee->parameters()[Trk::phi]
            ||  tpperigee[1] != perigee->parameters()[Trk::z0]
           
       ) continue ;
// for each perigee only single element in trkdefiPars is expected

    uint32_t HitPattern= tpperigee[0] ;
    ATH_MSG_VERBOSE( " Track HitPattern : " << HitPattern );



//  shift 1 left with bits of Trk::InnerDet_LayerX_DiscY to define a mask
//  int mask = 1<<Trk::InnerDet_LayerX_DiscY ;
//  check whether this bit is set in the pattern
//  bool set = HitPattern&mask ;

      if( vertex_pattern == insideBeamPipe ) {
  
  if( ! (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
  
      } else if( vertex_pattern == insidePixelBarrel0 ) {
  
  if( ! (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
      }
      
      else if( vertex_pattern == aroundPixelBarrel0 ) {
  // require nothing for PixelBarrel0 
  if( ! (HitPattern & (1<<Trk::pixelBarrel1)) ) return false;
      }      
      
      else if( vertex_pattern == outsidePixelBarrel0_and_insidePixelBarrel1 ) {
  
  if(   (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
  if( ! (HitPattern & (1<<Trk::pixelBarrel1)) ) return false;
      }
      
      else if( vertex_pattern == aroundPixelBarrel1 ) {
  
  if(   (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
        // require nothing for PixelBarrel1
  if( ! (HitPattern & (1<<Trk::pixelBarrel2)) ) return false;
      }
      
      else if( vertex_pattern == outsidePixelBarrel1_and_insidePixelBarrel2 ) {
  
  if(   (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel1)) ) return false;
  if( ! (HitPattern & (1<<Trk::pixelBarrel2)) ) return false;
      }
      
      else if( vertex_pattern == aroundPixelBarrel2 ) {
  
  if(   (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel1)) ) return false;
  // require nothing for PixelBarrel2
  if( ! (HitPattern & (1<<Trk::pixelBarrel3)) ) return false;
      }

      else if( vertex_pattern == outsidePixelBarrel2_and_insidePixelBarrel3 ) {
  
  if(   (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel1)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel2)) ) return false;
  if( ! (HitPattern & (1<<Trk::pixelBarrel3)) ) return false;
      }
  
      else if( vertex_pattern == aroundPixelBarrel3 ) {
  
  if(   (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel1)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel2)) ) return false;
  // require nothing for PixelBarrel3
  if( ! (HitPattern & (1<<Trk::sctBarrel0)) ) return false;
      }
      
      else if( vertex_pattern == outsidePixelBarrel3_and_insideSctBarrel0 ) {
  
  if(   (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel1)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel2)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel3)) ) return false;
  if( ! (HitPattern & (1<<Trk::sctBarrel0)) ) return false;
      }
      
      else if( vertex_pattern == aroundSctBarrel0 ) {
  
  if(   (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel1)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel2)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel3)) ) return false;
  // require nothing for SctBarrel0
  if( ! (HitPattern & (1<<Trk::sctBarrel1)) ) return false;
      }
      
      else if( vertex_pattern == outsideSctBarrel0_and_insideSctBarrel1 ) {
  
  if(   (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel1)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel2)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel3)) ) return false;
  if(   (HitPattern & (1<<Trk::sctBarrel0)) ) return false;
  if( ! (HitPattern & (1<<Trk::sctBarrel1)) ) return false;
      }

      else if( vertex_pattern == aroundSctBarrel1 ) {
  if(   (HitPattern & (1<<Trk::pixelBarrel0)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel1)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel2)) ) return false;
  if(   (HitPattern & (1<<Trk::pixelBarrel3)) ) return false;
  if(   (HitPattern & (1<<Trk::sctBarrel0)) ) return false;
  // require nothing for SctBarrel1
  if( ! (HitPattern & (1<<Trk::sctBarrel2)) ) return false;
      } else 
        pass = true ;
  }

  ATH_MSG_DEBUG( " passHitsFilter : " << pass );

  return pass ;
}        
  
void InDetIterativeSecVtxFinderTool::FillXcheckdefauls() 
{      
  m_seedac->push_back( 0 ) ;
  m_nperiseed->push_back( 0 ) ;
}

StatusCode InDetIterativeSecVtxFinderTool::initialize()
{
    StatusCode sc;

    m_evtNum = 0 ;
    ATH_CHECK(m_iVertexFitter.retrieve());
    ATH_CHECK(m_SeedFinder.retrieve());
    ATH_CHECK(m_LinearizedTrackFactory.retrieve());
    ATH_CHECK(m_ImpactPoint3dEstimator.retrieve());

    ATH_CHECK(m_trkFilter.retrieve());
    ATH_CHECK(m_SVtrkFilter.retrieve());

    // since some parameters special to an inherited class this method
    // will be overloaded by the inherited class
    printParameterSettings();

#ifdef MONITORTUNES
    ITHistSvc*     hist_root=nullptr;

    m_leastmodes = new std::vector<int>() ;
    m_sdFsmwX = new std::vector< std::vector < float > >() ;
    m_sdFsmwY = new std::vector< std::vector < float > >() ;
    m_sdFsmwZ = new std::vector< std::vector < float > >() ;
    m_sdcrsWght = new std::vector< std::vector < float > >() ;

    m_nperiseed = new std::vector<int>() ;
    m_seedX = new std::vector < float >() ;
    m_seedY = new std::vector < float >() ;
    m_seedZ = new std::vector < float >() ;
    m_seedXYdist = new std::vector< float >() ;
    m_seedZdist = new std::vector< float >() ;
    m_seedac = new std::vector < int >() ;

    sc = service( "THistSvc", hist_root); 
    if( sc.isFailure() ) 
    {
      ATH_MSG_ERROR( "Please switch off fillXcheck or provide THistSvc service" );
      return StatusCode::FAILURE;
    }

    m_OTree = new TTree( "IncSecVtxUnder", "TTree of underlying/upstream techinfo for InclusiveSecVtx" ) ;

    // the number of event processed ( calling of this tool )
    m_OTree->Branch("EvtNum", &m_evtNum, "EvtNum/I"  ) ;

    m_OTree->Branch("IteraNum", &m_iterations, "IteraNum/I"  ) ;

    // the minumal number of FsmwModes ( candidates ) for each seed
    // for each searching one has one or more modes, data type of vector<vecvtot<float>> :
    m_OTree->Branch("ModeNum",     &m_leastmodes  ) ;
    m_OTree->Branch("sdFsmwX",    &m_sdFsmwX ) ;
    m_OTree->Branch("sdFsmwY",    &m_sdFsmwY ) ;
    m_OTree->Branch("sdFsmwZ",    &m_sdFsmwZ ) ;
    m_OTree->Branch("crossingWght",    &m_sdcrsWght ) ;
    //  seeds as candidate ( number of trials ) of SecVtx
    m_OTree->Branch("nperigeeseed",    &m_nperiseed  ) ;
    m_OTree->Branch("seedX",    &m_seedX ) ;
    m_OTree->Branch("seedY",    &m_seedY ) ;
    m_OTree->Branch("seedZ",    &m_seedZ ) ;
    m_OTree->Branch("seedXYdist",    &m_seedXYdist ) ;
    m_OTree->Branch("seedZdist",    &m_seedZdist ) ;
    m_OTree->Branch("seedaccepted",    &m_seedac ) ;

    //  seeds filter

    //  seeds compatibility ( after fit )
      
    ATH_CHECK( hist_root->regTree("/AANT/SecVtxIncunder", m_OTree ) ) ;
#endif

    ATH_MSG_DEBUG( "Initialization successful" );
    return StatusCode::SUCCESS;
}

std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> InDetIterativeSecVtxFinderTool::findVertex(const TrackCollection* trackTES) 
{

  ATH_MSG_DEBUG( " Number of input tracks before track selection: " << trackTES->size() );

  std::vector<Trk::ITrackLink*> selectedTracks;

  typedef DataVector<Trk::Track>::const_iterator TrackDataVecIter;

  bool selectionPassed;
  for (TrackDataVecIter itr  = (*trackTES).begin(); itr != (*trackTES).end(); ++itr) {
    Trk::Vertex null(Amg::Vector3D(0,0,0));
    selectionPassed=static_cast<bool>(m_trkFilter->accept(**itr,&null));
    if ( selectionPassed ) selectionPassed=static_cast<bool>(m_SVtrkFilter->accept(**itr,&null));
    if (selectionPassed)
    {
      ElementLink<TrackCollection> link;
      link.setElement(*itr);
      Trk::LinkToTrack * linkTT = new Trk::LinkToTrack(link);
      linkTT->setStorableObject(*trackTES);
      selectedTracks.push_back(linkTT);
    }
  }

  ATH_MSG_DEBUG( "Of " << trackTES->size() << " tracks "
     << selectedTracks.size() << " survived the preselection." );

  
  std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> returnContainers 
     = findVertex( selectedTracks );

  return returnContainers;

}
// please do not run this TrackParticleBaseCollection and above TrackCollection utill fixed
std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> InDetIterativeSecVtxFinderTool::findVertex(const Trk::TrackParticleBaseCollection* trackTES) {
 
  ATH_MSG_DEBUG( " Number of input tracks before track selection: " << trackTES->size() );

  std::vector<Trk::ITrackLink*> selectedTracks;


  typedef DataVector<Trk::TrackParticleBase>::const_iterator TrackParticleDataVecIter;

  bool selectionPassed;
  for (TrackParticleDataVecIter itr  = (*trackTES).begin(); itr != (*trackTES).end(); ++itr) {
    
    Trk::Vertex null(Amg::Vector3D(0,0,0));
    selectionPassed=static_cast<bool>(m_trkFilter->accept( *((*itr)->originalTrack()), &null));
    if ( selectionPassed ) selectionPassed =static_cast<bool>( m_SVtrkFilter->accept( *((*itr)->originalTrack()), &null));
    
    if (selectionPassed)
    {
      ElementLink<Trk::TrackParticleBaseCollection> link;
      link.setElement(*itr);
      Trk::LinkToTrackParticleBase * linkTT = new Trk::LinkToTrackParticleBase(link);
      linkTT->setStorableObject(*trackTES);
      selectedTracks.push_back(linkTT);
    }
  }

  ATH_MSG_DEBUG( "Of " << trackTES->size() << " tracks "
     << selectedTracks.size() << " survived the preselection." );

  std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> returnContainers 
           = findVertex( selectedTracks );

  return returnContainers;
  
}

StatusCode InDetIterativeSecVtxFinderTool::finalize()
{
#ifdef MONITORTUNES
    m_OTree->Write();

    delete m_leastmodes ;
    delete m_sdFsmwX ;
    delete m_sdFsmwY ;
    delete m_sdFsmwZ ;
    delete m_sdcrsWght ;
    delete m_nperiseed ;
    delete m_seedX ;
    delete m_seedY ;
    delete m_seedZ ;
    delete m_seedXYdist ;
    delete m_seedZdist ;
    
    delete m_seedac ;

    m_leastmodes = nullptr ;
    m_sdFsmwX = nullptr ;
    m_sdFsmwY = nullptr ;
    m_sdFsmwZ = nullptr ;
    m_sdcrsWght = nullptr ;
    m_nperiseed = nullptr ;
    m_seedX = nullptr ;
    m_seedY = nullptr ;
    m_seedZ = nullptr ;
    m_seedXYdist = nullptr ;
    m_seedZdist = nullptr ;
    m_seedac = nullptr ;

#endif

    return StatusCode::SUCCESS;
}

void InDetIterativeSecVtxFinderTool::printParameterSettings()
{
  ATH_MSG_DEBUG( "InDetIterativeSecVtxFinderTool initialize(): Parametersettings " );
  ATH_MSG_DEBUG( "VertexFitter " << m_iVertexFitter );
}

void InDetIterativeSecVtxFinderTool::SGError(const std::string &errService)
{
  ATH_MSG_FATAL( errService << " not found. Exiting !" );
}

double InDetIterativeSecVtxFinderTool::compatibility(const Trk::TrackParameters & measPerigee,
                   const AmgSymMatrix(3) &covariancePosition , const Amg::Vector3D &position  ) const
{
  double returnValue = 0. ;
 
  Trk::LinearizedTrack* myLinearizedTrack=m_LinearizedTrackFactory->linearizedTrack( 
     &measPerigee, position );

//  Amg::Vector3D mapca = myLinearizedTrack->expectedMomentumAtPCA() ;

  AmgMatrix(2,2) weightReduced=myLinearizedTrack->expectedCovarianceAtPCA().block<2,2>(0,0);
  
  AmgMatrix(2,2) errorVertexReduced=(myLinearizedTrack->positionJacobian()*
   ( covariancePosition*myLinearizedTrack->positionJacobian().transpose())).block<2,2>(0,0);
  
  weightReduced+=errorVertexReduced;
  
  weightReduced = weightReduced.inverse().eval();
  Amg::Vector2D trackParameters2D = myLinearizedTrack->expectedParametersAtPCA().block<2,1>(0,0);
  returnValue += trackParameters2D.dot(weightReduced*trackParameters2D);
  
  delete myLinearizedTrack;
  myLinearizedTrack=nullptr;

  return returnValue;
}

void InDetIterativeSecVtxFinderTool::removeAllFrom(std::vector<const Trk::TrackParameters*> & perigeesToFit,
                                                  std::vector<Trk::ITrackLink*> & seedTracks) const
{
  //remove all perigeesToFit and go on...
  
  std::vector<Trk::ITrackLink*>::iterator  seedBegin=seedTracks.begin();
  std::vector<Trk::ITrackLink*>::iterator  seedEnd=seedTracks.end();
  
  std::vector<const Trk::TrackParameters*>::iterator perigeesToFitBegin=perigeesToFit.begin();
  std::vector<const Trk::TrackParameters*>::iterator perigeesToFitEnd=perigeesToFit.end();
  
  for (std::vector<const Trk::TrackParameters*>::iterator perigeesToFitIter=perigeesToFitBegin;
       perigeesToFitIter!=perigeesToFitEnd;++perigeesToFitIter)
  {
    
    ATH_MSG_VERBOSE( " Iterating on new track in original perigeesToFit list of BAD VERTEX..." );
    
    
    bool found=false;
    
    for (std::vector<Trk::ITrackLink*>::iterator seedIter=seedTracks.begin();
         seedIter!=seedEnd;++seedIter)
    {
      if ((*seedIter)->parameters()==*perigeesToFitIter)
      {
        
        ATH_MSG_VERBOSE( " found and deleted from seeds!" );
        
        found=true;
        seedTracks.erase(seedIter);
        seedBegin=seedTracks.begin();
        seedEnd=seedTracks.end();
        break;
      }
    }
    
    if (!found)
    {
      ATH_MSG_ERROR( " Cannot find vector element to delete when removing BAD vertex! " );
    }
    
  }//for cycle
  
}

void InDetIterativeSecVtxFinderTool::countTracksAndNdf(xAOD::Vertex * myxAODVertex,
                                                      float & ndf, int & ntrk) const
{
  ndf = -3.0 ;
  ntrk = 0 ;

  if ( myxAODVertex )
  {
    ndf = myxAODVertex->numberDoF();
    std::vector<Trk::VxTrackAtVertex> myVxTracksAtVtx = myxAODVertex->vxTrackAtVertex();
    std::vector<Trk::VxTrackAtVertex>::iterator tracksBegin=myVxTracksAtVtx.begin();
    std::vector<Trk::VxTrackAtVertex>::iterator tracksEnd=myVxTracksAtVtx.end();
    
    for (std::vector<Trk::VxTrackAtVertex>::iterator tracksIter=tracksBegin;
         tracksIter!=tracksEnd;++tracksIter)
    {
      
      if ( (*tracksIter).weight() > m_minWghtAtVtx  )
      {
        ntrk+=1;
      }
    }
  }
}

float InDetIterativeSecVtxFinderTool::removeTracksInBadSeed( xAOD::Vertex * myxAODVertex,
                                            std::vector<const Trk::TrackParameters*> & perigeesToFit ) const
{
  int removed = 0 ;

  std::vector<Trk::VxTrackAtVertex>* tracksAtVertex= &(myxAODVertex->vxTrackAtVertex());

  int tot = tracksAtVertex->size() ;
  ATH_MSG_DEBUG( " removeTracksInBadSeed ... #Tracks associated at vertex : " << tot );

  if ( tot == 0 ) return 0. ;

  std::vector<Trk::VxTrackAtVertex>::const_iterator tracksBegin=tracksAtVertex->begin();
  std::vector<Trk::VxTrackAtVertex>::const_iterator tracksEnd=tracksAtVertex->end();

  std::vector<const Trk::TrackParameters*>::iterator perigeesToFitBegin=perigeesToFit.begin();
  std::vector<const Trk::TrackParameters*>::iterator perigeesToFitEnd=perigeesToFit.end();

  const Amg::Vector3D position = myxAODVertex->position() ;

  tot = 0 ;
  float pt_tot = 0. , pt_hf = 0. ;

  std::vector<const Trk::TrackParameters*>  perigees_deleted ;
  for (std::vector<Trk::VxTrackAtVertex>::const_iterator tracksAtVertexIter=tracksBegin;
       tracksAtVertexIter!=tracksEnd;++tracksAtVertexIter)
  {
    if ( (*tracksAtVertexIter).weight() <= m_minWghtAtVtx ) continue ;

    const Trk::TrackParameters* measPerigee = (*tracksAtVertexIter).initialPerigee() ;

    if ( measPerigee == nullptr ) continue ;

    tot ++ ;
    bool hf = passHitsFilter( measPerigee, position.perp(), std::abs( position.z() ) ) ;

    float pt = 0. ;
    const Trk::TrackParameters * svperigee = (*tracksAtVertexIter).perigeeAtVertex() ;
    if ( svperigee ) 
      pt = svperigee->parameters()[Trk::theta]/std::abs( svperigee->parameters()[Trk::qOverP] ) ;

    pt_tot += pt ;

    if ( hf ) continue ; 

    ATH_MSG_DEBUG( " found and will delete from perigeesToFit !" );

    auto found=std::find(perigeesToFitBegin,perigeesToFitEnd,measPerigee);
    if(found!=perigeesToFitEnd){
        perigees_deleted.push_back(*found);
        perigeesToFit.erase(found);
        perigeesToFitBegin=perigeesToFit.begin();
        perigeesToFitEnd=perigeesToFit.end();
        pt_hf += pt ;
        removed++;
    }
  }

  if ( tot == 0 || pt_tot == 0. ) return 0. ;

//#ifndef MONITORTUNES
  for (std::vector<const Trk::TrackParameters*>::iterator perigeesToFitIter=perigees_deleted.begin();
        perigeesToFitIter!=perigees_deleted.end();++perigeesToFitIter)
  {
    for (std::vector<Trk::VxTrackAtVertex>::const_iterator tracksAtVertexIter=tracksBegin;
       tracksAtVertexIter!=tracksEnd;++tracksAtVertexIter)
    {
      const Trk::TrackParameters* measPerigee = (*tracksAtVertexIter).initialPerigee() ;
  
      if (*perigeesToFitIter==measPerigee)
      {
        tracksAtVertex->erase( tracksAtVertexIter );
        tracksBegin=tracksAtVertex->begin();
        tracksEnd=tracksAtVertex->end();
        ATH_MSG_DEBUG( " deleted tracksAtVertex !" );
        break ;
      } 
    }
  }
//#endif

  ATH_MSG_DEBUG( " Bad tracks removed from seed : " << removed );
  return 0.4*removed/( 1.0*tot ) + 0.6*pt_hf/pt_tot ;

}

const std::vector< Amg::Vector3D > InDetIterativeSecVtxFinderTool::getVertexMomenta( xAOD::Vertex * myxAODVertex ) const
{
  std::vector< Amg::Vector3D > TrkAtVtxMomenta ;

  std::vector<Trk::VxTrackAtVertex>* tracksAtVertex= &(myxAODVertex->vxTrackAtVertex());

  ATH_MSG_DEBUG( " getVertexMomenta ... #Tracks associated at vertex : " << tracksAtVertex->size() );

  std::vector<Trk::VxTrackAtVertex>::const_iterator tracksAtVertexBegin=tracksAtVertex->begin();
  std::vector<Trk::VxTrackAtVertex>::const_iterator tracksAtVertexEnd=tracksAtVertex->end();

  for (std::vector<Trk::VxTrackAtVertex>::const_iterator tracksAtVertexIter=tracksAtVertexBegin;
       tracksAtVertexIter!=tracksAtVertexEnd;++tracksAtVertexIter)
  {
    if ((*tracksAtVertexIter).weight() <= m_minWghtAtVtx ) continue ;
    {
      const Trk::TrackParameters* sv_perigee = (*tracksAtVertexIter).perigeeAtVertex() ;

      double qp  = 1/std::abs( sv_perigee->parameters()[Trk::qOverP] ) ;
      double theta = sv_perigee->parameters()[Trk::theta];
      double phi = sv_perigee->parameters()[Trk::phi];

      m_TrkAtVtxMomenta.emplace_back( qp*sin(theta)*cos(phi), qp*sin(theta)*sin(phi), qp*cos(theta) ) ;

    }

  }

  return m_TrkAtVtxMomenta ;
}

void InDetIterativeSecVtxFinderTool::removeCompatibleTracks(xAOD::Vertex * myxAODVertex,
                                                           std::vector<const Trk::TrackParameters*> & perigeesToFit,
                                                           std::vector<Trk::ITrackLink*> & seedTracks) const
{
  //now you have your new vertex with its new tracks
  //now you have to get the compatibility also of all tracks which DIDN'T BELONG to the vertex!
  std::vector<Trk::VxTrackAtVertex>* tracksAtVertex= &(myxAODVertex->vxTrackAtVertex());
  
  ATH_MSG_VERBOSE( " removeCompatibleTracks ... #Tracks associated at vertex : " << tracksAtVertex->size() );

  std::vector<Trk::VxTrackAtVertex>::const_iterator tracksAtVertexBegin=tracksAtVertex->begin();
  std::vector<Trk::VxTrackAtVertex>::const_iterator tracksAtVertexEnd=tracksAtVertex->end();
  
  std::vector<Trk::ITrackLink*>::iterator  seedBegin=seedTracks.begin();
  std::vector<Trk::ITrackLink*>::iterator  seedEnd=seedTracks.end();
  
  std::vector<const Trk::TrackParameters*>::iterator perigeesToFitBegin=perigeesToFit.begin();
  std::vector<const Trk::TrackParameters*>::iterator perigeesToFitEnd=perigeesToFit.end();
  
  const AmgSymMatrix(3) covariance = myxAODVertex->covariancePosition() ;
  const Amg::Vector3D position = myxAODVertex->position() ;

  for (std::vector<Trk::VxTrackAtVertex>::const_iterator tracksAtVertexIter=tracksAtVertexBegin;
       tracksAtVertexIter!=tracksAtVertexEnd;++tracksAtVertexIter)
  {
    
    
    ATH_MSG_VERBOSE( " new track..." );
    
    
    bool found=false;
    
    for (std::vector<Trk::ITrackLink*>::iterator seedIter=seedBegin;
         seedIter!=seedEnd;++seedIter)
    {
      if ((*seedIter)->parameters()==(*tracksAtVertexIter).initialPerigee() )
      {
        found=true;
        if ((*tracksAtVertexIter).weight()> m_minWghtAtVtx )
        {
          
          ATH_MSG_VERBOSE( " found and deleted from seeds!" );
          
          seedTracks.erase(seedIter);
          seedBegin=seedTracks.begin();
          seedEnd=seedTracks.end();
        }
        break;
      }
    }
    
    if (!found)
    {
      ATH_MSG_ERROR( " Cannot find vector element to delete (step 1)! " );
    }
    
    found=false;
    for (std::vector<const Trk::TrackParameters*>::iterator perigeesToFitIter=perigeesToFitBegin;
         perigeesToFitIter!=perigeesToFitEnd;++perigeesToFitIter)
    {
      if (*perigeesToFitIter==(*tracksAtVertexIter).initialPerigee())
      {
        found=true;
        
        if ((*tracksAtVertexIter).weight()> m_minWghtAtVtx )
        {
          
          ATH_MSG_VERBOSE( " found and deleted from perigeesToFit!" );
          
          perigeesToFit.erase(perigeesToFitIter);
          perigeesToFitBegin=perigeesToFit.begin();
          perigeesToFitEnd=perigeesToFit.end();

        }
        break;
      }
    }
    
#ifndef MONITORTUNES
    if (!found)
    {
      ATH_MSG_WARNING( " Cannot find vector element to delete (step 2)! " );
    }
#endif
    
  }//finishing iterating on tracks at vertex
  

  ATH_MSG_DEBUG( " Outliers still to be considered: " << perigeesToFit.size() );

  ATH_MSG_VERBOSE( "Number of seedtracks after removal of inliers: " << seedTracks.size() );



  std::vector<Trk::VxTrackAtVertex>* myVxTracksAtVertex= &(myxAODVertex->vxTrackAtVertex());
  
  std::vector<Trk::VxTrackAtVertex>::iterator tracksBegin=myVxTracksAtVertex->begin();
  std::vector<Trk::VxTrackAtVertex>::iterator tracksEnd=myVxTracksAtVertex->end();
  
  for (std::vector<const Trk::TrackParameters*>::iterator perigeesToFitIter=perigeesToFitBegin;
       perigeesToFitIter!=perigeesToFitEnd;++perigeesToFitIter)
  {
    
    bool found=false;
    
    //compute the chi2 with respect to the last fitted vertex!
    //(easy since track was NOT used in the last vertex fit...)
    
    const Trk::TrackParameters* myPerigee=(*perigeesToFitIter);
    
    if (myPerigee==nullptr)
    {
      ATH_MSG_ERROR( " Cast to perigee gives 0 pointer " );
      return;
      
    }
    
    double chi2=compatibility(*myPerigee, covariance, position );

    
    ATH_MSG_VERBOSE( "Compatibility is : " << chi2 );
    
    
    //check if still sufficiently compatible to previous vertex
    //(CAN BE VERY LOOSE TO BE CONSERVATIVE AGAINST FAR OUTLIERS)
    if (  chi2 < m_maximumChi2cutForSeeding )
    {
      
      ATH_MSG_VERBOSE( " Found track with compatibility: " << chi2 << 
       " to be removed from the seeds... " );
      
      
// These seed tracks did NOT participate the fitting of vertex, but they seem compatible...
      for (std::vector<Trk::ITrackLink*>::iterator seedIter=seedTracks.begin();
           seedIter!=seedEnd;++seedIter)
      {
        if ((*seedIter)->parameters()==*perigeesToFitIter)
        {
          
          ATH_MSG_VERBOSE( " found and deleted from seeds!" );
          
          found=true;
          seedTracks.erase(seedIter);
          seedBegin=seedTracks.begin();
          seedEnd=seedTracks.end();
          break;
        }
      }
      
      if (!found)
      {
        ATH_MSG_ERROR( " Cannot find vector element to delete (step 3)! " );
      }
    }
    else
    {
      //look if the track is attached to the vertex. If this is the case you should 
      //delete the track from the vertex!
      
      
      ATH_MSG_VERBOSE( " Found track with compatibility: " << chi2 << 
       " to be further considered and thus to be removed from previous vertex if it was there... " );
      
      
      bool found=false;
      
// some tracks seem INcompatible/INconsistet even though they succeed the fit
      for (std::vector<Trk::VxTrackAtVertex>::iterator tracksIter=tracksBegin;
           tracksIter!=tracksEnd;++tracksIter)
      {
        if ((*tracksIter).initialPerigee()==*perigeesToFitIter)
        {
          
          
          ATH_MSG_VERBOSE( " OK, removing track with compatibility:"  << (*tracksIter).trackQuality().chiSquared() << 
           " or vtx compatibility" << (*tracksIter).vtxCompatibility() << " which was found attached to the vertex... " );
          
          // this delete is no longer needed because objects in myVxTracksAtVertex are no longer pointers - memory deletion of this VxTrackAtVertex
          // was already taken care of inside fitter the moment the VxTrackAtVertex was added to the vector stored in xAOD::Vertex
          // -David S.
          //
          //delete *tracksIter; // delete has to happen BEFORE the erase (because the iterator will point to the next object in the vector AFTER the erase!)
          myVxTracksAtVertex->erase(tracksIter);
          tracksBegin=myVxTracksAtVertex->begin();
          tracksEnd=myVxTracksAtVertex->end();
          found=true;
          break;
        }
        
      }
      
      if (!found)
      {
        
        ATH_MSG_VERBOSE( "Track not found: probably it was already not attached to the vertex" );
        
      }
    }
  }//iterate on all perigeesToFit
  
      ATH_MSG_VERBOSE( " #CompatibleTracks associated at vertex : " << myVxTracksAtVertex->size() );
  
}

} // end namespace InDet
