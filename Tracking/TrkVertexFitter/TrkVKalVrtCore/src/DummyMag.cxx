/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//  Create object vkalMagFld which containg magnetic field.
//  It accepts external handlers with magnetic field ( function or object)
//  and takes data from them. If they are absent - default constants 
//  ATLAS magnetic field (1.996 tesla) is used
//
//   Single object myMagFld of vkalMagFld type is created in CFit.cxx
//-------------------------------------------------------------------------
#include "TrkVKalVrtCore/VKalVrtBMag.h"
#include "TrkVKalVrtCore/TrkVKalVrtCore.h"
#include "TrkVKalVrtCore/CommonPars.h"
#include <iostream>

namespace Trk {

vkalMagFld::vkalMagFld()
  : m_cnstBMAG(1.997)
  , /* Const mag. field in Tesla */
  m_vkalCnvMagFld(vkalMagCnvCst)
{
//   vkalCnvMagFld = 0.0029979246;   /* For GeV and cm and Tesla*/
//   m_vkalCnvMagFld = 0.29979246;   /* For MeV and mm and Tesla*/
}


void vkalMagFld::getMagFld(const double X,const double Y,const double Z,
                           double& bx, double& by, double& bz, VKalVrtControlBase* FitControl=nullptr ) const
{
     bx=by=0.;
     //std::cout<<"In coreMag.field0 Control="<<FitControl<<'\n';
     //std::cout<<"In coreMag.field0 obj="<<FitControl->vk_objMagFld<<" func="<<FitControl->vk_funcMagFld<<'\n';
     if ( FitControl==nullptr  || (FitControl->vk_funcMagFld == nullptr && FitControl->vk_objMagFld==nullptr) ){
        bz=m_cnstBMAG;
        return;
     }
     if ( FitControl->vk_funcMagFld ){
        FitControl->vk_funcMagFld(X,Y,Z,bx,by,bz);
     }else if ( FitControl->vk_objMagFld ) {
        FitControl->vk_objMagFld->getMagFld(X,Y,Z,bx,by,bz);
     }
     //std::cout<<" coreMag.field="<<bx<<", "<<by<<", "<<bz<<" obj="<<FitControl->vk_objMagFld<<" func="<<FitControl->vk_funcMagFld<<'\n';
}

double vkalMagFld::getMagFld(const double xyz[3], const VKalVrtControlBase* FitControl=nullptr ) const
{
     //std::cout<<"In coreMag.field1 Control="<<FitControl<<'\n';
     //std::cout<<"In coreMag.field1 obj="<<FitControl->vk_objMagFld<<" func="<<FitControl->vk_funcMagFld<<'\n';
     double bx=0., by=0., bz=0.;
     if (  FitControl==nullptr  || (FitControl->vk_funcMagFld == nullptr && FitControl->vk_objMagFld==nullptr) ){
       bz=m_cnstBMAG;
       return bz;
     }
     if ( FitControl->vk_funcMagFld ){
        FitControl->vk_funcMagFld(xyz[0],xyz[1],xyz[2],bx,by,bz);
     }else if ( FitControl->vk_objMagFld ) {
        FitControl->vk_objMagFld->getMagFld(xyz[0],xyz[1],xyz[2],bx,by,bz);
     }
     return bz;
}



}

