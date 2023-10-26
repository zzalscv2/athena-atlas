/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVKalVrtCore/TrkVKalVrtCore.h"
#include "TrkVKalVrtCore/cfImp.h"
//-------------------------------------------------
//
#include<iostream>


//
//__________________________________________________________________________
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


namespace Trk{

  double TrkVKalVrtFitter::VKalGetImpact(const Trk::Perigee* InpPerigee,
                                         const Amg::Vector3D& Vertex,
                                         const long int Charge,
                                         std::vector<double>& Impact,
                                         std::vector<double>& ImpactError) const
  {
    State state;
    initState (state);
    return VKalGetImpact (InpPerigee, Vertex, Charge, Impact, ImpactError, state);
  }

  double TrkVKalVrtFitter::VKalGetImpact(const Trk::Perigee* InpPerigee,
                                         const Amg::Vector3D& Vertex,
                                         const long int Charge,
                                         std::vector<double>& Impact,
                                         std::vector<double>& ImpactError,
                                         IVKalState& istate) const
  {
    assert(dynamic_cast<State*> (&istate)!=nullptr);
    State& state = static_cast<State&> (istate);

    //
    //------ Variables and arrays needed for fitting kernel
    //
    double SIGNIF=0.;
    std::vector<const Trk::Perigee*> InpPerigeeList;
    InpPerigeeList.push_back(InpPerigee);

    //
    //------  extract information about selected tracks
    //
    int ntrk=0;
    StatusCode sc = CvtPerigee(InpPerigeeList, ntrk, state);
    if(sc.isFailure() || ntrk != 1) {    //Something is wrong in conversion
        Impact.assign(5,1.e10);
        ImpactError.assign(3,1.e20);
        return 1.e10;
     }
    long int vkCharge = state.m_ich[0];
    if(Charge==0) vkCharge=0;

    //
    // Target vertex in ref.frame defined by track themself
    //
    double VrtInp[3]={Vertex.x()-state.m_refFrameX,
		      Vertex.y()-state.m_refFrameY,
		      Vertex.z()-state.m_refFrameZ};
    double VrtCov[6]={0.,0.,0.,0.,0.,0.};

    Impact.resize(5);
    ImpactError.resize(3);
    Trk::cfimp(0, vkCharge, 0,
	       &state.m_apar[0][0], &state.m_awgt[0][0],
	       &VrtInp[0], &VrtCov[0],
	       Impact.data(), ImpactError.data(),
	       &SIGNIF, &state.m_vkalFitControl);

    return SIGNIF;
  }


  double TrkVKalVrtFitter::VKalGetImpact(const xAOD::TrackParticle* InpTrk,const Amg::Vector3D& Vertex,const long int Charge,
                                         std::vector<double>& Impact, std::vector<double>& ImpactError) const
  {
    State state;
    initState (state);
    return VKalGetImpact (InpTrk, Vertex, Charge, Impact, ImpactError, state);
  }


  double TrkVKalVrtFitter::VKalGetImpact(const xAOD::TrackParticle* InpTrk,const Amg::Vector3D& Vertex,const long int Charge,
                                         std::vector<double>& Impact, std::vector<double>& ImpactError,
                                         IVKalState& istate) const
  {
    assert(dynamic_cast<State*> (&istate)!=nullptr);
    State& state = static_cast<State&> (istate);
//
//------ Variables and arrays needed for fitting kernel
//
    double SIGNIF=0.;

    std::vector<const xAOD::TrackParticle*> InpTrkList(1,InpTrk);
//

//
//------  extract information about selected tracks
//
    int ntrk=0;
    StatusCode sc = CvtTrackParticle(InpTrkList,ntrk,state);
    if(sc.isFailure() ||  ntrk != 1   )  {       //Something is wrong in conversion
        Impact.assign(5,1.e10);
        ImpactError.assign(3,1.e20);
        return 1.e10;
    }
    if(std::abs(Vertex.z())>m_IDsizeZ || Vertex.perp()>m_IDsizeR){  // Crazy user request
        Impact.assign(5,1.e10);
        ImpactError.assign(3,1.e20);
        return 1.e10;
    }
    long int vkCharge=state.m_ich[0];
    if(Charge==0)vkCharge=0;
//
// Target vertex in ref.frame defined by track itself
//
    double VrtInp[3]={Vertex.x() -state.m_refFrameX, Vertex.y() -state.m_refFrameY, Vertex.z() -state.m_refFrameZ};
    double VrtCov[6]={0.,0.,0.,0.,0.,0.};
//
//
    Impact.resize(5); ImpactError.resize(3);
    Trk::cfimp( 0, vkCharge, 0, &state.m_apar[0][0], &state.m_awgt[0][0], &VrtInp[0], &VrtCov[0], Impact.data(), ImpactError.data(), &SIGNIF, &state.m_vkalFitControl);

    return SIGNIF;

  }


}

