/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkVKalVrtCore/CascadeUtils.h"
#include "TrkVKalVrtCore/Utilities.h"
#include "TrkVKalVrtCore/Derivt.h"
#include "TrkVKalVrtCore/cfMomentum.h"
#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"
#include "TrkVKalVrtCore/VKalVrtBMag.h"
#include <array>
#include <cmath>
#include <iostream>

namespace Trk {

//  Add to system matrix the derivatives due to pseudotrack constraints
int fixPseudoTrackPt(long int NPar, double * fullMtx, double * LSide, CascadeEvent & cascadeEvent_)
{

   int iv,it,ivnext;
   //Deliberately not make_unique to bypass inititalization
   std::unique_ptr<double[]> DerivC( new double[NPar] );
   std::unique_ptr<double[]> DerivP( new double[NPar] );
   std::unique_ptr<double[]> DerivT( new double[NPar] );
//
   std::vector<double> vMagFld; double vBx,vBy,vBz;
   for( iv=0; iv<cascadeEvent_.cascadeNV; iv++){
      VKVertex * vk = cascadeEvent_.cascadeVertexList[iv].get();
      Trk::vkalMagFld::getMagFld(vk->refIterV[0]+vk->iniV[0], vk->refIterV[1]+vk->iniV[1], vk->refIterV[2]+vk->iniV[2],vBx,vBy,vBz,(vk->vk_fitterControl).get());
      vMagFld.push_back(vBz);  // fill mag.fields for all vertices
   }
//
   for( iv=0; iv<cascadeEvent_.cascadeNV; iv++){
      int indCombTrk=-1;
      int iniPosTrk=0;                              /* Start of track part of vertex in global matrix */
      int posCombTrk=0;                             /* Conbined track position in global matrix */
      VKVertex* vk = cascadeEvent_.cascadeVertexList[iv].get();
      if(vk->nextCascadeVrt){                           //next vertex exists
        ivnext=-1;                                      //index of next vertex in common structure
        for(int ivt=0;ivt<cascadeEvent_.cascadeNV;ivt++)if(vk->nextCascadeVrt==cascadeEvent_.cascadeVertexList[ivt].get())ivnext=ivt;
        if(ivnext<0){return -1;};  //error in cascade
//
        int NV=vk->nextCascadeVrt->includedVrt.size();
        if(NV>0){
          for(it=0; it<NV; it++)
	    if(vk->nextCascadeVrt->includedVrt[it] == vk)
              indCombTrk=vk->nextCascadeVrt->TrackList.size() - NV + it;   // index of combined track in next vertex track list
        }
        if(indCombTrk>=0){
          iniPosTrk  =cascadeEvent_.matrixPnt[iv]+3;  /*Start of track part of vertex in global matrix */
          posCombTrk =cascadeEvent_.matrixPnt[ivnext]+3+3*indCombTrk;  /*Get position in global matrix */
        }
        if(posCombTrk==0 || iniPosTrk==0) {return -1;}  //ERROR  in cascade structure somewhere....
//
// Momentum of pseudo track in next vertex
        for(int ivt=0; ivt<NPar; ivt++)DerivC[ivt]=DerivP[ivt]=DerivT[ivt]=0.;
        DerivC[posCombTrk+2]=-1.;
        DerivT[posCombTrk+0]=-1.;
        DerivP[posCombTrk+1]=-1.;
        std::array<double, 4> ppsum = getIniParticleMom( vk->nextCascadeVrt->TrackList[indCombTrk].get(), vMagFld[ivnext] ); // INI for pseudo
        double csum=vk->nextCascadeVrt->TrackList[indCombTrk]->iniP[2];                                              // INI for pseudo
        double ptsum=sqrt(ppsum[0]*ppsum[0] + ppsum[1]*ppsum[1]);
        double sinth2sum=(ppsum[0]*ppsum[0] + ppsum[1]*ppsum[1])/(ppsum[0]*ppsum[0] + ppsum[1]*ppsum[1] + ppsum[2]*ppsum[2]);

//
// Momenta+Derivatives of tracks in vertex itself
  for(it=0; it<(int)vk->TrackList.size(); it++){
    std::array<double, 4> pp =    getIniParticleMom( vk->TrackList[it].get(), vMagFld[iv]);
	  double curv=vk->TrackList[it]->iniP[2];
    double pt=sqrt(pp[0]*pp[0] + pp[1]*pp[1]);
    double cth=pp[2]/pt;
    double sinth2=(pp[0]*pp[0] + pp[1]*pp[1])/(pp[0]*pp[0] + pp[1]*pp[1] + pp[2]*pp[2]);
    DerivC[iniPosTrk+it*3+1] =  csum/ptsum/ptsum*(ppsum[0]*pp[1]-ppsum[1]*pp[0]);         //  dC/dPhi_i
    DerivC[iniPosTrk+it*3+2] =  csum/ptsum/ptsum*(ppsum[0]*pp[0]+ppsum[1]*pp[1])/curv;    //  dC/dC_i
    DerivP[iniPosTrk+it*3+1] =  (ppsum[0]*pp[0]+ppsum[1]*pp[1])/ptsum/ptsum;           //  dPhi/dPhi_i
    DerivP[iniPosTrk+it*3+2] =  (ppsum[1]*pp[0]-ppsum[0]*pp[1])/ptsum/ptsum/curv;      //  dPhi/dC_i
    DerivT[iniPosTrk+it*3+0] =  (sinth2sum*pt)/(sinth2*ptsum);                   //  dTheta/dTheta_i
    DerivT[iniPosTrk+it*3+2] =  (sinth2sum*pt*cth)/(curv*ptsum);                 //  dTheta/dC_i
  }
//        double iniV0Curv=myMagFld.getCnvCst()*vMagFld[iv]/sqrt(tpx*tpx+tpy*tpy);    //initial PseudoTrack Curvature
//        if(csum<0)iniV0Curv *= -1.;
//        iniV0Curv *= vMagFld[ivnext]/vMagFld[iv];  //magnetic field correction
//
//fill Full Matrix and left side vector
//
//---- Momentum only
//        for(it=0; it<NPar; it++) fullMtx[ (NPar-1*(cascadeEvent_.cascadeNV-1)+1*iv  )*NPar + it] = DerivC[it];
//        for(it=0; it<NPar; it++) fullMtx[ (NPar-1*(cascadeEvent_.cascadeNV-1)+1*iv  ) + it*NPar] = DerivC[it];
//        LSide[ NPar-1*(cascadeEvent_.cascadeNV-1)+1*iv] = -iniV0Curv+csum;
//---- Momentum+phi+theta //VK seems overshooting because direction is fixed by vertex-vertex pointing. Returns wrong error matrix
        for(it=0; it<NPar; it++) fullMtx[ (NPar-3*(cascadeEvent_.cascadeNV-1)+3*iv  )*NPar + it] = DerivT[it];
        for(it=0; it<NPar; it++) fullMtx[ (NPar-3*(cascadeEvent_.cascadeNV-1)+3*iv+1)*NPar + it] = DerivP[it];
        for(it=0; it<NPar; it++) fullMtx[ (NPar-3*(cascadeEvent_.cascadeNV-1)+3*iv+2)*NPar + it] = DerivC[it];
        for(it=0; it<NPar; it++) fullMtx[ (NPar-3*(cascadeEvent_.cascadeNV-1)+3*iv  ) + it*NPar] = DerivT[it];
        for(it=0; it<NPar; it++) fullMtx[ (NPar-3*(cascadeEvent_.cascadeNV-1)+3*iv+1) + it*NPar] = DerivP[it];
        for(it=0; it<NPar; it++) fullMtx[ (NPar-3*(cascadeEvent_.cascadeNV-1)+3*iv+2) + it*NPar] = DerivC[it];
        VKTrack* cmbt=vk->nextCascadeVrt->TrackList[indCombTrk].get();
        LSide[ NPar-3*(cascadeEvent_.cascadeNV-1)+3*iv  ] = cmbt->iniP[0]-cmbt->Perig[2];
        LSide[ NPar-3*(cascadeEvent_.cascadeNV-1)+3*iv+1] = cmbt->iniP[1]-cmbt->Perig[3];
        LSide[ NPar-3*(cascadeEvent_.cascadeNV-1)+3*iv+2] = cmbt->iniP[2]-cmbt->Perig[4];
      }

   } //end of vertex cycle
   return 0;  //All ok
}
//---------------------------------------------------------------------------
//  Returns address of  VTrack of combined track for given vertex
//
VKTrack * getCombinedVTrack(VKVertex * vk)
{
    if(!vk->nextCascadeVrt) return nullptr;                 //nonpointing vertex
    int NV=vk->nextCascadeVrt->includedVrt.size();
    if(NV==0) return nullptr;                               //Error in structure

    int itv=-1;
    for(int it=0; it<NV; it++) if(vk->nextCascadeVrt->includedVrt[it] == vk) {itv=it; break;};
    if(itv<0) return nullptr;                               // Not found in list

    int totNT = vk->nextCascadeVrt->TrackList.size();
    return vk->nextCascadeVrt->TrackList[totNT - NV + itv].get(); // pointer to combined track in next vertex
}


//---------------------------------------------------------------------------
//  Returns dimension of full matrix for cascade fit
//  At the end the place for pseudotrack(all!) momenta(3) constraints
//  By default (Type=0) return full cascade NPar.
//  For Type=1 returns amount of physics parameters only (without all constraints)
//
//   MUST BE CONSISTENT WITH fixPseudoTrackPt(...)!!!
//
int getCascadeNPar(CascadeEvent & cascadeEvent_, int Type/*=0*/)
{
  int NV=cascadeEvent_.cascadeNV;
  int NTrk=0;
  int NCnst=0;
  for( int iv=0; iv<cascadeEvent_.cascadeNV; iv++){
     VKVertex *vk = cascadeEvent_.cascadeVertexList[iv].get();
     NTrk += vk->TrackList.size();
     for(int ic=0; ic<(int)vk->ConstraintList.size();ic++) NCnst += vk->ConstraintList[ic]->NCDim;
  }
  if(Type==1) return 3*(NV+NTrk);                              // Return amount of physics parameters
  return 3*(NV+NTrk)+NCnst + 3*(cascadeEvent_.cascadeNV-1);  //additional 3 momentum constraints
  //return 3*(NV+NTrk)+NCnst + 1*(cascadeEvent_.cascadeNV-1);    //additional 1 momentum constraints
}

//
//  Track parameters are translated at each iteration so iniV==(0,0,0)
//
void setFittedParameters(const double * result, std::vector<int> & matrixPnt, CascadeEvent & cascadeEvent_)
{
   int iv,it,Pnt;
   for( iv=0; iv<cascadeEvent_.cascadeNV; iv++){
     VKVertex *vk = cascadeEvent_.cascadeVertexList[iv].get();
     Pnt=matrixPnt[iv];    // start of vertex parameters
     vk->fitV[0]=result[Pnt]; vk->fitV[1]=result[Pnt+1]; vk->fitV[2]=result[Pnt+2];
     for( it=0; it<(int)vk->TrackList.size(); it++){
        VKTrack * trk=vk->TrackList[it].get();
        trk->fitP[0]=trk->iniP[0]+result[Pnt+3+it*3 + 0];
        trk->fitP[1]=trk->iniP[1]+result[Pnt+3+it*3 + 1];
        trk->fitP[2]=trk->iniP[2]+result[Pnt+3+it*3 + 2];
        trk->Chi2 = cfchi2(vk->fitV, trk->fitP, trk );
     }
   }
}

void setFittedMatrices(const double * COVFIT, long int MATRIXSIZE,
                       std::vector<int> & matrixPnt,
                       std::vector< std::vector<double> > & covarCascade,
                       CascadeEvent & cascadeEvent_)
{
   int iv, Pnt, ic, ir, vrtMtxSize, count;
   for( iv=0; iv<cascadeEvent_.cascadeNV; iv++){
     VKVertex *vk = cascadeEvent_.cascadeVertexList[iv].get();
     Pnt=matrixPnt[iv];    // start of vertex parameters
     vrtMtxSize=3+vk->TrackList.size()*3;         //size of matrix for given vertex
     std::vector<double> Res(vrtMtxSize*(vrtMtxSize+1)/2);
     count=0;
     for (int col=0; col<vrtMtxSize; col++){
       for (int row=0; row<=col; row++){
          ic=Pnt+col; ir=Pnt+row;
          Res[count]=COVFIT[ic*MATRIXSIZE + ir]; count++;
       }
     }
     covarCascade.emplace_back(std::move(Res));
   }
}
//
// Symmetrical indexing (I*(I+1)/2+J) is valid ONLY if I>=J
std::vector<double> transformCovar(int NPar, double **Deriv, const std::vector<double> &covarI )
{
      std::vector<double> covarO(NPar*(NPar+1)/2, 0.);
      int ii,ij,oi,oj, indexO, indexI;
      for(oi=0; oi<NPar; oi++){
        for(oj=0; oj<=oi; oj++){
            indexO = oi*(oi+1)/2+oj;
            for(ii=0; ii<NPar; ii++){
              for(ij=0; ij<NPar; ij++){
                indexI = ii>=ij ? ii*(ii+1)/2+ij : ij*(ij+1)/2+ii;
                covarO[indexO] += Deriv[oi][ii]*covarI[indexI]*Deriv[oj][ij];
      } }   } }
      return covarO;
}


void addCrossVertexDeriv(CascadeEvent & cascadeEvent_, double * ader, long int MATRIXSIZE, const std::vector<int> & matrixPnt)
{
   int iv,ivn;
   //for( iv=0; iv<cascadeEvent_.cascadeNV; iv++)std::cout<<matrixPnt[iv]<<", ";std::cout<<'\n';

   for( iv=0; iv<cascadeEvent_.cascadeNV; iv++){
     VKVertex *vk = cascadeEvent_.cascadeVertexList[iv].get();
     if(!vk->nextCascadeVrt)continue;                        //no pointing
     for( ivn=iv; ivn<cascadeEvent_.cascadeNV; ivn++){
        if(vk->nextCascadeVrt == cascadeEvent_.cascadeVertexList[ivn].get()) break;    //vertex found
     }
     if( ivn == cascadeEvent_.cascadeNV ) continue;  // no found vertex
//
// Now we have vertex pair "from"(iv) "to"(ivn)
     int From=matrixPnt[iv];    // start of "from" information
     int Next=matrixPnt[iv+1];  // start of "next vertex" information. Pnt.constraints are 2 prevous param.!!!
     int Into=matrixPnt[ivn];   // start of "to" information
//
//  The same constraints, but derivatives are with respect to other ("to") vertex
     ader[(0+Into)   + (Next-1)*MATRIXSIZE] = - ader[(0+From)   + (Next-1)*MATRIXSIZE];
     ader[(1+Into)   + (Next-1)*MATRIXSIZE] = - ader[(1+From)   + (Next-1)*MATRIXSIZE];
     ader[(2+Into)   + (Next-1)*MATRIXSIZE] = - ader[(2+From)   + (Next-1)*MATRIXSIZE];
     ader[(0+Into)   + (Next-2)*MATRIXSIZE] = - ader[(0+From)   + (Next-2)*MATRIXSIZE];
     ader[(1+Into)   + (Next-2)*MATRIXSIZE] = - ader[(1+From)   + (Next-2)*MATRIXSIZE];
     ader[(2+Into)   + (Next-2)*MATRIXSIZE] = - ader[(2+From)   + (Next-2)*MATRIXSIZE];
     ader[(0+Into)*MATRIXSIZE   + (Next-1)] = - ader[(0+From)   + (Next-1)*MATRIXSIZE];
     ader[(1+Into)*MATRIXSIZE   + (Next-1)] = - ader[(1+From)   + (Next-1)*MATRIXSIZE];
     ader[(2+Into)*MATRIXSIZE   + (Next-1)] = - ader[(2+From)   + (Next-1)*MATRIXSIZE];
     ader[(0+Into)*MATRIXSIZE   + (Next-2)] = - ader[(0+From)   + (Next-2)*MATRIXSIZE];
     ader[(1+Into)*MATRIXSIZE   + (Next-2)] = - ader[(1+From)   + (Next-2)*MATRIXSIZE];
     ader[(2+Into)*MATRIXSIZE   + (Next-2)] = - ader[(2+From)   + (Next-2)*MATRIXSIZE];
   }
}


//--------------------------------------------------------------------
//  Copy matrix Input with dimension IDIM to predefined place(TStart)
//   into matrix Target with dimension TDIM
//
void copyFullMtx(const double *Input, long int IPar, long int IDIM,
                 double *Target, long int TStart, long int TDIM)
{
   int i,j,it,jt;
   for( i=0; i<IPar; i++){
     for( j=0; j<IPar; j++){
        it=i+TStart; jt=j+TStart;
        Target[it*TDIM+jt] = Input[i*IDIM+j];
     }
   }
}

//--------------------------------------------------------------------
//  Make the convolution Cov=D*OldCov*Dt
//
void getNewCov(const double *OldCov, const double* Der, double* Cov, long int DIM) noexcept
{
   int i,j,it,jt;
   for( i=0; i<DIM; i++){
     for( j=0; j<DIM; j++){
       Cov[i*DIM+j]=0.;
       for( it=0; it<DIM; it++){
         for( jt=0; jt<DIM; jt++){
           Cov[i*DIM+j] += Der[i*DIM+it]*OldCov[it*DIM+jt]*Der[j*DIM+jt];
         }
       }
     }
   }
}



} /* End of namespace Trk*/
