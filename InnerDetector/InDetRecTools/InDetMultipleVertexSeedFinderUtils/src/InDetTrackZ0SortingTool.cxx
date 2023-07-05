/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetMultipleVertexSeedFinderUtils/InDetTrackZ0SortingTool.h"
#include "TrkTrack/Track.h"
#include "VxVertex/Vertex.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkExInterfaces/IExtrapolator.h"

namespace InDet
{

 StatusCode InDetTrackZ0SortingTool::initialize()
 {
  if ( m_extrapolator.retrieve().isFailure() ) 
  {
   ATH_MSG_ERROR("Failed to retrieve tool " << m_extrapolator);
   return StatusCode::FAILURE;                                                  
  }
  ATH_MSG_DEBUG("Retrieved tool " << m_extrapolator);
 
  return StatusCode::SUCCESS;
 }//end of initialize method
    

 InDetTrackZ0SortingTool::InDetTrackZ0SortingTool(const std::string& t, const std::string& n, const IInterface*  p):
 AthAlgTool(t,n,p), m_extrapolator("Trk::Extrapolator")
 {

  //interface
  declareInterface<InDetTrackZ0SortingTool>(this);
  
  //extrapolator
  declareProperty("Extrapolator",m_extrapolator);  
  
 }//end of constructor 
 

 InDetTrackZ0SortingTool::~InDetTrackZ0SortingTool()
 = default;
 

 std::vector<int> InDetTrackZ0SortingTool::sortedIndex(const std::vector<const Trk::Track*>& tracks, const Trk::Vertex * reference )const
 {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  std::map<double, int> mapOfZ0;
  std::vector<const Trk::Track*>::const_iterator tb = tracks.begin();
  std::vector<const Trk::Track*>::const_iterator te = tracks.end();
  unsigned int j=0;
  for(;tb != te ;++tb)
  {
   const Trk::TrackParameters * perigee(nullptr);
   if(!reference) perigee=(*tb)->perigeeParameters();
   else
   {
 
     //here we want to make an extrapolation
     Trk::PerigeeSurface perigeeSurface(reference->position());
     perigee = m_extrapolator->extrapolateTrack(ctx,
						**tb,perigeeSurface,
						Trk::anyDirection, true,
						Trk::pion).release();
   }//end of extrapolation block
   
   if(perigee)
   {
    double trkZ0 =perigee->parameters()[Trk::z0];
    mapOfZ0.insert(std::map<double, int>::value_type(trkZ0,j));
    if (reference)
    {
      delete perigee;
      perigee=nullptr;
    }
   }
   else ATH_MSG_WARNING("This track particle has no perigee state. Not egligible for sorting. Will NOT be written to the sorted vector");
   ++j;
  }//end of loop over all the tracks
 
  //creating an output vector, filling it and returning
  std::vector<int> result(0);
  std::map<double, int>::const_iterator mb = mapOfZ0.begin();
  std::map<double, int>::const_iterator me = mapOfZ0.end(); 
  for(;mb!=me;++mb) 
  {
   result.push_back((*mb).second);
  }
  return result;
 }//end of sorting method

  std::vector<int> InDetTrackZ0SortingTool::sortedIndex(const std::vector<const xAOD::TrackParticle*>& tracks,const xAOD::Vertex * reference) const
  {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    std::map<double, int> mapOfZ0; 
    std::vector<const xAOD::TrackParticle*>::const_iterator tb = tracks.begin();
    std::vector<const xAOD::TrackParticle*>::const_iterator te = tracks.end();
    unsigned int j=0;
    
    for(;tb != te ;++tb)
      {
	const Trk::TrackParameters * perigee = nullptr;

	//here we want to make an extrapolation
	Trk::PerigeeSurface perigeeSurface(reference->position());
	perigee = m_extrapolator->extrapolate(ctx,
					      (*tb)->perigeeParameters(),
					      perigeeSurface,
					      Trk::anyDirection, true,
					      Trk::pion).release();
	
	if(perigee)
	  {
	    double trkZ0 = perigee->parameters()[Trk::z0];
	    mapOfZ0.insert(std::map<double, int>::value_type(trkZ0,j)); 
	    delete perigee;
	    perigee = nullptr;
	  }else{
	  ATH_MSG_WARNING("This track particle has no perigee state. Not egligible for sorting. Will NOT be written to the sorted vector");
	}//end of perigee existance check
	++j;
      }//end of loop over all track particle base's
    
    //creating an output vector, filling it and returning
    std::vector<int> result(0);
    
    //sorted part  
    std::map<double, int>::const_iterator mb = mapOfZ0.begin();
    std::map<double, int>::const_iterator me = mapOfZ0.end(); 
    for(;mb!=me;++mb) result.push_back((*mb).second);

    return result;
  }

}
