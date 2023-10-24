/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetMultipleVertexSeedFinder/SlidingWindowMultiSeedFinder.h"
#include "TrkTrack/Track.h"
#include "VxVertex/Vertex.h"

#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "InDetMultipleVertexSeedFinderUtils/InDetTrackZ0SortingTool.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkVertexFitterInterfaces/IVertexSeedFinder.h"
#include "xAODTracking/Vertex.h"

namespace InDet
{

 StatusCode SlidingWindowMultiSeedFinder::initialize()
 {
  if(m_trkFilter.retrieve().isFailure())
  {
   ATH_MSG_ERROR(" Unable to retrieve "<<m_trkFilter);
   return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Track selector tool retrieved");

  if(m_sortingTool.retrieve().isFailure()){
   ATH_MSG_ERROR(" Unable to retrieve "<<m_sortingTool);
   return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Track sorting tool retrieved");
   
  ATH_CHECK(m_beamSpotKey.initialize());

  if(m_vtxSeedFinder.retrieve().isFailure())
  {
   ATH_MSG_ERROR("Unable to retrieve " << m_vtxSeedFinder);
   return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Vertex seed finder retriever");
    
  if(m_extrapolator.retrieve().isFailure())
  {
   ATH_MSG_ERROR("Failed to retrieve tool " << m_extrapolator);
   return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Retrieved tool " << m_extrapolator);

  return StatusCode::SUCCESS;
 }//end of initialize mtehod
 

 SlidingWindowMultiSeedFinder::SlidingWindowMultiSeedFinder(const std::string& t, const std::string& n, const
                                                            IInterface*p):AthAlgTool(t,n,p), 
                                                                          m_clusterLength(5.),
                                                                          m_breakingDistance(3.), 
                                                                          m_addingDistance(0.1), 
                                                                          m_useMaxInCluster(false),
                                                                          m_ignoreLevel(0),
                                                                          m_ignoreBeamSpot(false),
                                                                          m_vtxSeedFinder("Trk::CrossDistancesSeedFinder"),
                                                                          m_extrapolator("Trk::Extrapolator") 
 {
  declareInterface<IMultiPVSeedFinder>(this); 
  declareProperty("clusterLength",      m_clusterLength);
  declareProperty("BreakingDistance",   m_breakingDistance);
  declareProperty("AddingDistance",     m_addingDistance);
  declareProperty("UseMaxInCluster",    m_useMaxInCluster);
  declareProperty("IgnoreLevel",	m_ignoreLevel);
  declareProperty("IgnoreBeamSpot",	m_ignoreBeamSpot);
  
  //track filter
  declareProperty("TrackSelector", m_trkFilter);

  //sorting tool
  declareProperty("SortingTool", m_sortingTool);

  //vertex finder tool (needed when no beam spot is available)
  declareProperty("VertexSeedFinder",m_vtxSeedFinder);
  
  //extrapolator
  declareProperty("Extrapolator", m_extrapolator);  
  
 }
 
 SlidingWindowMultiSeedFinder::~SlidingWindowMultiSeedFinder()
 = default;

 std::vector< std::vector<const Trk::Track *> > SlidingWindowMultiSeedFinder::seeds(const std::vector<const Trk::Track*>& tracks )const
 {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  std::vector<const Trk::Track*> preselectedTracks(0);
  std::vector<const Trk::Track*>::const_iterator tr = tracks.begin();
  std::vector<const Trk::Track*>::const_iterator tre = tracks.end(); 
  
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
  Trk::RecVertex  beamRecVertex(beamSpotHandle->beamVtx());
  for(;tr!=tre;++tr) if(m_trkFilter->decision(**tr,&beamRecVertex)) preselectedTracks.push_back(*tr);
  ATH_MSG_DEBUG("Beam spot position is: "<< beamRecVertex.position());

  Trk::Vertex* beamVertex=&beamRecVertex;
  
  Trk::Vertex myVertex;
  if (m_ignoreBeamSpot)
  {
   myVertex = Trk::Vertex(m_vtxSeedFinder->findSeed(tracks));
   ATH_MSG_DEBUG(" vtx seed x: " << myVertex.position().x() <<
		 " vtx seed y: " << myVertex.position().y() <<
		 " vtx seed z: " << myVertex.position().z());
   beamVertex = &myVertex;
  }

  //output container
  std::vector< std::vector<const Trk::Track *> > result(0);

  //sorting the tracks on their z_0 basis.
  if(!preselectedTracks.empty())
  {
   std::vector<int> indexOfSorted = m_sortingTool->sortedIndex(preselectedTracks,beamVertex);  

   std::vector<const Trk::Track *> tmp_cluster(0); 

   Trk::PerigeeSurface perigeeSurface(beamVertex->position());

   const Trk::TrackParameters * exPerigee = nullptr;
   if(!indexOfSorted.empty()){
    exPerigee =
      m_extrapolator
      ->extrapolateTrack(ctx, *preselectedTracks[indexOfSorted[0]], perigeeSurface, Trk::anyDirection, true, Trk::pion)
      .release();
   }

   float lastTrackZ0  = -999.;
   if(exPerigee) { lastTrackZ0 = exPerigee->parameters()[Trk::z0]; delete exPerigee; }
   else {
    ATH_MSG_WARNING("Impossible to extrapolate the first track; returning 0 container for this event");
    return result;
   }          	 	 
	 	   	   
   //looping over sorted container:
   //storing the Z0 of last iteration...
   float prevTrackZ0 = 0.;

   float addingDistance = m_addingDistance;
   if(m_useMaxInCluster) addingDistance = 0.;
   for(unsigned int i=0;i<indexOfSorted.size();++i)
   {
    const  Trk::TrackParameters * lexPerigee = m_extrapolator->extrapolateTrack(ctx, *preselectedTracks[indexOfSorted[i]],
									   perigeeSurface,Trk::anyDirection,true, Trk::pion).release(); 
    float currentTrackZ0 = lexPerigee->parameters()[Trk::z0];
    delete lexPerigee;
    
    if(!i) prevTrackZ0 = currentTrackZ0; 
     
    ATH_MSG_DEBUG("Z0 of current  track"<< currentTrackZ0);
    
    if((std::abs(currentTrackZ0 - lastTrackZ0)>m_clusterLength &&
	std::abs(currentTrackZ0 - prevTrackZ0)>addingDistance) ||
       std::abs(currentTrackZ0 - prevTrackZ0)>m_breakingDistance)
    {
     //this track is outside the current cluster
     if(int(tmp_cluster.size())>m_ignoreLevel) result.push_back(tmp_cluster);
     tmp_cluster.clear();
     tmp_cluster.push_back(preselectedTracks[indexOfSorted[i]]);
     lastTrackZ0 = currentTrackZ0;
    }else{
     //this track is within the current cluster
     tmp_cluster.push_back(preselectedTracks[indexOfSorted[i]]);
     //checking whethet this is the last track in list
     if(i==indexOfSorted.size()-1 && int(tmp_cluster.size())>m_ignoreLevel) result.push_back(tmp_cluster);
    }//end of check for the cluster size
        
    //making adding distance max in cluster
    if(m_useMaxInCluster){
      float diff = std::abs(currentTrackZ0-prevTrackZ0);
      if(addingDistance < diff) addingDistance = diff; 
    }//end of  using max in cluster
    prevTrackZ0 = currentTrackZ0;
   }//end of loop over the sorted container, using sorted index
  
  }//end of check for successfull preselection
   
  return result;
 }//end of seeding method

 
std::vector< std::vector<const Trk::TrackParameters *> > SlidingWindowMultiSeedFinder::seeds(const std::vector<const xAOD::TrackParticle*>& tracks )const
  {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    //step 1: preselection 
    std::vector<const xAOD::TrackParticle*> preselectedTracks(0);
    
    //selecting with respect to the beam spot
    xAOD::Vertex beamposition;
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey };
    beamposition.setPosition(beamSpotHandle->beamVtx().position());
    beamposition.setCovariancePosition(beamSpotHandle->beamVtx().covariancePosition());

    for (const auto *track : tracks) {
      if (m_trkFilter->decision(*track,&beamposition)) preselectedTracks.push_back(track);
    }
    
    std::vector<const Trk::TrackParameters*> perigeeList;
    std::vector<const xAOD::TrackParticle*>::const_iterator trackBegin=tracks.begin();
    std::vector<const xAOD::TrackParticle*>::const_iterator trackEnd=tracks.end();
    for (std::vector<const xAOD::TrackParticle*>::const_iterator trackIter=trackBegin;trackIter!=trackEnd;++trackIter)
      {
	perigeeList.push_back(&((*trackIter)->perigeeParameters()));
      }
 
    Trk::RecVertex myVertex (m_vtxSeedFinder->findSeed(perigeeList));
   
    if (m_ignoreBeamSpot)
      {
	ATH_MSG_DEBUG(" vtx seed x: " << myVertex.position().x() <<
		      " vtx seed y: " << myVertex.position().y() <<
		      " vtx seed z: " << myVertex.position().z());
	beamposition.setPosition(myVertex.position());
	beamposition.setCovariancePosition(myVertex.covariancePosition());
      }
    
    //step 2: sorting in z0
    //output container  
    std::vector< std::vector<const Trk::TrackParameters *> > result(0);
    if(!preselectedTracks.empty())
      {
	std::vector<int> indexOfSorted = m_sortingTool->sortedIndex(preselectedTracks, &beamposition);
	
	std::vector<const Trk::TrackParameters *> tmp_cluster(0); 
	
	//extrapolating the tracks to the actual beam spot   
	const Trk::TrackParameters * exPerigee(nullptr);
	Trk::PerigeeSurface perigeeSurface(beamposition.position());
	
	exPerigee = m_extrapolator->extrapolate(ctx, preselectedTracks[indexOfSorted[0]]->perigeeParameters(),
						perigeeSurface,Trk::anyDirection,true, Trk::pion).release();
	
	float lastTrackZ0  = -999.;
	if(exPerigee) { lastTrackZ0 = exPerigee->parameters()[Trk::z0]; delete exPerigee; }
	else
	  {
	    ATH_MSG_WARNING("Impossible to extrapolate the first track; returning 0 container for this event");
	    return result;
	  }          	   
	
	ATH_MSG_DEBUG("Z0 of last track"<< lastTrackZ0);

	//looping over sorted container:  storing the Z0 of last iteration...
	float prevTrackZ0 =0.;

        float addingDistance = m_addingDistance;
	if(m_useMaxInCluster) addingDistance = 0.;
	
	for(unsigned int i=0;i<indexOfSorted.size();++i){

          const Trk::TrackParameters* lexPerigee =
            m_extrapolator
              ->extrapolate(
                ctx, 
                preselectedTracks[indexOfSorted[i]]->perigeeParameters(), 
                perigeeSurface, 
                Trk::anyDirection, 
                true, Trk::pion).release();
          float currentTrackZ0 = lexPerigee->parameters()[Trk::z0];
          delete lexPerigee;

          if(!i) prevTrackZ0 = currentTrackZ0;
          ATH_MSG_DEBUG("Z0 of current  track" << currentTrackZ0);

          if ((std::abs(currentTrackZ0 - lastTrackZ0) > m_clusterLength &&
               std::abs(currentTrackZ0 - prevTrackZ0) > addingDistance) ||
              std::abs(currentTrackZ0 - prevTrackZ0) > m_breakingDistance) {
            // this track is outside the current cluster or the distance is too big. breaking the cluster here
            if (int(tmp_cluster.size()) > m_ignoreLevel) result.push_back(tmp_cluster);
            tmp_cluster.clear();
            tmp_cluster.push_back(&(preselectedTracks[indexOfSorted[i]]->perigeeParameters()));
            lastTrackZ0 = currentTrackZ0;
	    ATH_MSG_DEBUG("Finishing a cluster, starting the new one");
	  }else{
	    //this track is within the current cluster  or the distance to next is too short. Keeping populating it.
	    tmp_cluster.push_back(&(preselectedTracks[indexOfSorted[i]]->perigeeParameters()));
	    ATH_MSG_DEBUG("Pushing a track to a cluster");

	    if(i==indexOfSorted.size()-1 && int(tmp_cluster.size())>m_ignoreLevel) result.push_back(tmp_cluster);

	  }//end of check for the cluster size
	    
	  //making adding distance max in cluster
	  if(m_useMaxInCluster)
	    {
	      float diff = std::abs(currentTrackZ0-prevTrackZ0);
	      if(addingDistance < diff) addingDistance = diff;
	    }//end of  using max in cluster
	  prevTrackZ0 = currentTrackZ0;
	}//end of loop
      }//end of preselection size check
    
    ATH_MSG_DEBUG("Returning number of clusters: "<< result.size());
    return result;

  }
  
}//end of namespace definitions
