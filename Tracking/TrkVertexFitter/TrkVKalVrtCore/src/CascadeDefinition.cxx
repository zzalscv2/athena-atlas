/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkVKalVrtCore/CascadeDefinition.h"
#include "TrkVKalVrtCore/CFitCascade.h"
#include "TrkVKalVrtCore/VKvFast.h"
#include "TrkVKalVrtCore/Matrix.h"
#include "TrkVKalVrtCore/Derivt.h"
#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"
#include "TrkVKalVrtCore/VKalVrtBMag.h"
#include <cmath>
#include <iostream>

namespace Trk {

VKVertex* startCascade( std::unique_ptr<VKVertex> vk)
{
   auto *ptr = vk.get();
   ptr->vk_fitterControl->getCascadeEvent()->cascadeNV=1;
   ptr->vk_fitterControl->getCascadeEvent()->nearPrmVertex=0;
   ptr->vk_fitterControl->getCascadeEvent()->cascadeVertexList.clear();
   ptr->vk_fitterControl->getCascadeEvent()->cascadeVertexList.push_back(std::move(vk));
   return ptr->vk_fitterControl->getCascadeEvent()->cascadeVertexList.back().get();
}

VKVertex* addCascadeEntry( std::unique_ptr<VKVertex> vk)
{
   auto *ptr = vk.get();
   ptr->vk_fitterControl->getCascadeEvent()->cascadeNV++;
   ptr->vk_fitterControl->getCascadeEvent()->cascadeVertexList.push_back(std::move(vk));
   return ptr->vk_fitterControl->getCascadeEvent()->cascadeVertexList.back().get();
}

VKVertex* addCascadeEntry( std::unique_ptr<VKVertex> vk, const std::vector<int> &index)
{
   for(int i=0; i<(int)index.size(); i++){
     VKVertex * predecessor =  vk->vk_fitterControl->getCascadeEvent()->cascadeVertexList.at(index[i]).get();
     if(predecessor->nextCascadeVrt) std::cout <<"VKalVrtCore: ERROR 1 in CASCADE creation !!!"<<'\n';
     predecessor->nextCascadeVrt = vk.get();
     vk->includedVrt.push_back(predecessor);
   }
//
   auto *ptr = vk.get();
   ptr->vk_fitterControl->getCascadeEvent()->cascadeNV++;
   ptr->vk_fitterControl->getCascadeEvent()->cascadeVertexList.push_back(std::move(vk));
   return ptr->vk_fitterControl->getCascadeEvent()->cascadeVertexList.back().get();
}

//=================================================================================================
//   Main cascade definition entry
//
//  Reference and iteration reference points are set to (0,0,0). Track weight matrices are computed.
//
//  If predessor vertices point to current one, container tracks are created for combined particles
//     from them.
//
//  vertexDefinition[iv][it]   - list of real track to vertex associacion
//  cascadeDefinition[iv][ipv] - for given vertex IV the list of previous vertices pointing to it.
//
int makeCascade(VKalVrtControl & FitCONTROL, long int NTRK, const long int *ich, double *wm, double *inp_Trk5, double *inp_CovTrk5,
                   const std::vector< std::vector<int> > &vertexDefinition,
                   const std::vector< std::vector<int> > &cascadeDefinition,
                   double definedCnstAccuracy/*=1.e-4*/)
{
    long int  tk;
    int iv,it;
    if( vertexDefinition.size() != cascadeDefinition.size() ){
       std::cout<<" WRONG INPUT!!!"<<'\n';
       return  -1;
    }

    long int IERR;
    int NV=vertexDefinition.size();
    double xyz[3]={0.};
    double tmp[15]={0.};
    double tmpWgt[15],out[3];
    double vBx,vBy,vBz;
    VKTrack  * trk;
    int vEstimDone=0;

    for(iv=0; iv<NV; iv++){
      auto VRT = std::make_unique< VKVertex >(FitCONTROL);
      VRT->setRefV(xyz);                               //ref point
      VRT->setRefIterV(xyz);                           //iteration ref. point
      VRT->setIniV(xyz); VRT->setCnstV(xyz);           // initial guess. 0 of course.
      auto arr = std::make_unique<double[]>(vertexDefinition[iv].size()*5);
      int NTv=vertexDefinition[iv].size();
      for (it=0; it<NTv ; it++) {
        tk=vertexDefinition[iv][it];
        if( tk >= NTRK ) {
          return -1;
        }
        VRT->TrackList.emplace_back(new VKTrack(tk, &inp_Trk5[tk*5], &inp_CovTrk5[tk*15] , VRT.get(), wm[tk]));
        VRT->tmpArr.emplace_back(new TWRK());
        trk = VRT->TrackList[it].get();
        trk->Charge = ich[tk];
        trk->iniP[0]=trk->cnstP[0]=trk->fitP[0]=inp_Trk5[tk*5+2];   //initial guess
        trk->iniP[1]=trk->cnstP[1]=trk->fitP[1]=inp_Trk5[tk*5+3];
        trk->iniP[2]=trk->cnstP[2]=trk->fitP[2]=inp_Trk5[tk*5+4];
        IERR=cfInv5(&inp_CovTrk5[tk*15], tmpWgt);
        if (IERR) IERR=cfdinv(&inp_CovTrk5[tk*15], tmpWgt, 5);
        if (IERR) { return  -1; }
        trk->setCurrent(&inp_Trk5[tk*5],tmpWgt);
        arr[it*5]=inp_Trk5[tk*5];arr[it*5+1]=inp_Trk5[tk*5+1];arr[it*5+2]=inp_Trk5[tk*5+2];
        arr[it*5+3]=inp_Trk5[tk*5+3];arr[it*5+4]=inp_Trk5[tk*5+4];
      }
      if(NTv>1){   //First estimation of vertex position if vertex has more than 2 tracks
        vEstimDone=1;
        Trk::vkalMagFld::getMagFld(VRT->refIterV[0],VRT->refIterV[1],VRT->refIterV[2],vBx,vBy,vBz,(VRT->vk_fitterControl).get());
        double aVrt[3]={0.,0.,0.};
        for(int i=0; i<NTv-1;i++) for(int j=i+1; j<NTv; j++){
           vkvFastV(&arr[i*5],&arr[j*5],xyz,vBz,out);
           aVrt[0]+=out[0]; aVrt[1]+=out[1]; aVrt[2]+=out[2];
        }
        aVrt[0]/=(NTv*(NTv-1)/2); aVrt[1]/=(NTv*(NTv-1)/2); aVrt[2]/=(NTv*(NTv-1)/2);
        VRT->setRefIterV(aVrt);                       //iteration ref. point
      }
      if(iv==0){                           // start cascade creation
         startCascade( std::move(VRT) );
         continue;
      }
      int includeNV=cascadeDefinition[iv].size();
      if(!includeNV) {                     // no predecessors
        addCascadeEntry( std::move(VRT) );
      }else{
        auto *vrttemp = addCascadeEntry( std::move(VRT), cascadeDefinition[iv]);
        for (it=0; it<includeNV ; it++) {             // tracks created out of predecessing vertices
          vrttemp->TrackList.emplace_back(new VKTrack(-999, tmp, tmp , vrttemp, 0.));
          vrttemp->tmpArr.emplace_back(new TWRK());
        }
      }
    }
//
// ----------------  If some vertex positions are different from (0,0,0) - move tracks there
    if(vEstimDone){
      IERR = translateToFittedPos(*(FitCONTROL.getCascadeEvent()),1.); if(IERR)return IERR;
      for( iv=0; iv<FitCONTROL.getCascadeEvent()->cascadeNV; iv++){
        auto *VRT=FitCONTROL.getCascadeEvent()->cascadeVertexList[iv].get();
        int NTv = VRT->TrackList.size();            // Number of tracks at vertex
        for(it=0; it<NTv; it++){
          trk=VRT->TrackList[it].get();
          if(trk->Id < 0) continue;               // pseudo-track from cascade vertex
          trk->cnstP[0] = trk->iniP[0] = trk->fitP[0] = trk->Perig[2];
          trk->cnstP[1] = trk->iniP[1] = trk->fitP[1] = trk->Perig[3];
          trk->cnstP[2] = trk->iniP[2] = trk->fitP[2] = trk->Perig[4];
        }
      }
    }
// ----------------  Init engine

    IERR=initCascadeEngine(*FitCONTROL.getCascadeEvent());
    FitCONTROL.getCascadeEvent()->setAccuracyConstraint(definedCnstAccuracy);

    return IERR;
}

//==================================================================================
//  Creation of VKalVrtCore arrays and structures needed for fit and first fit
//   without any constraints to get approximate vertices and track parameters
//
int initCascadeEngine(CascadeEvent & cascadeEvent_)
{
  VKVertex * VRT;
  long int IERR=0, iv, i;
//---------------------Some check-----------
//  VKMassConstraint * tmpc0=0; VKMassConstraint * tmpc1=0;
//  if(cascadeEvent_.cascadeVertexList[0]->ConstraintList.size()>0){
//     tmpc0=dynamic_cast< VKMassConstraint*>(cascadeEvent_.cascadeVertexList[0]->ConstraintList[0]);
//     //std::cout<<(*tmpc0)<<" init status"<<'\n';
//     if(cascadeEvent_.cascadeVertexList[0]->ConstraintList.size()>1){
//       tmpc1=dynamic_cast< VKMassConstraint*>(cascadeEvent_.cascadeVertexList[0]->ConstraintList[1]);
//       //std::cout<<(*tmpc1)<<" init status"<<'\n';
//  }}
//  if( cascadeEvent_.cascadeVertexList.size()>1){
//    if( cascadeEvent_.cascadeVertexList[1]->ConstraintList.size()>0){
//      tmpc1=dynamic_cast< VKMassConstraint*>(cascadeEvent_.cascadeVertexList[1]->ConstraintList[0]);
//      //std::cout<<(*tmpc1)<<" init status"<<'\n';
//  }}
//
// ----------------  Fisrt fit without any constraints at all, just vertices
  for( i=0; i<4; i++){
    for(iv=0; iv<cascadeEvent_.cascadeNV; iv++){
       VRT = cascadeEvent_.cascadeVertexList[iv].get();
       IERR = fitVertexCascade( VRT, 0);     if(IERR)return IERR;   //fit
       IERR = setVTrackMass(VRT);            if(IERR)return IERR;   //mass of combined particle
       if(!VRT->includedVrt.empty()){  // Save fitted vertex as target for "pass near" constraint in previous vertex
         for(int pseu=0; pseu<(int)VRT->includedVrt.size(); pseu++){
	    VRT->includedVrt[pseu]->FVC.vrt[0] = VRT->refIterV[0] + VRT->fitV[0];
	    VRT->includedVrt[pseu]->FVC.vrt[1] = VRT->refIterV[1] + VRT->fitV[1];
	    VRT->includedVrt[pseu]->FVC.vrt[2] = VRT->refIterV[2] + VRT->fitV[2];
           std::copy(VRT->fitVcov, VRT->fitVcov + 6, VRT->includedVrt[pseu]->FVC.covvrt);
         }
       }
    }
    IERR = translateToFittedPos(cascadeEvent_);           if(IERR)return IERR;
  }
  return IERR;
}

//==================================================================================
//  Mass constraints for DEFINED cascade. So may be called only after makeCascade!!!
//
//          For complete vertex
int setCascadeMassConstraint(CascadeEvent & cascadeEvent_, long int IV, double Mass)
{
    if(IV>cascadeEvent_.cascadeNV-1) return -1;  //error in format
    if(IV<0)                         return -1;
    VKVertex * vk =  cascadeEvent_.cascadeVertexList[IV].get();  //target vertex
    int NTRK = vk->TrackList.size();
    vk->ConstraintList.emplace_back(new VKMassConstraint( NTRK, Mass, vk));
    return 0;
}
//-----------------------------------------
//          For subset of tracks in vertex.
//  trkInVrt,pseudoInVrt - list of indexes of participating tracks/pseudos in given vertex
//  Tracks and pseudotracks are in the common list - tracks first, pseudotracks after
//
int setCascadeMassConstraint(CascadeEvent & cascadeEvent_, long int IV, std::vector<int> &trkInVrt, std::vector<int> &pseudoInVrt, double Mass)
{
    std::vector<int> tmpIndex;
    int it;
    if(IV>cascadeEvent_.cascadeNV-1) return -1;  //error in format
    if(IV<0)                         return -1;
    VKVertex * vk =  cascadeEvent_.cascadeVertexList[IV].get();  //target vertex
    int NTRK = vk->TrackList.size();             //tracks+pseudotracks
    int nRealTrk=0;                              //number of real tracks
    for(it=0; it<(int)trkInVrt.size(); it++) if(vk->TrackList[it]->Id >= 0) nRealTrk++;
//
//-- Real tracks
//
    if( (int)trkInVrt.size() > NTRK) return -1;            //checks...
    for(it=0; it<(int)trkInVrt.size(); it++){
       if(trkInVrt[it] <   0  ) return -1;
       if(trkInVrt[it] >= NTRK) return -1;
       tmpIndex.push_back(trkInVrt[it]);
    }
//
//-- Pseudo tracks
//
    if( (int)pseudoInVrt.size() > NTRK) return -1;            //checks...
    for(int it=0; it<(int)pseudoInVrt.size(); it++){
       if(pseudoInVrt[it] <   0  ) return -1;
       if(pseudoInVrt[it] >= NTRK) return -1;
       tmpIndex.push_back(pseudoInVrt[it]+nRealTrk);
    }

    vk->ConstraintList.emplace_back(new VKMassConstraint( NTRK, Mass, std::move(tmpIndex), vk));
    return 0;
}

} /* End of VKalVrtCore namespace */
