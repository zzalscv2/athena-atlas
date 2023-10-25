/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkVKalVrtCore/cfMomentum.h"
#include "TrkVKalVrtCore/CommonPars.h"
#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"
#include "TrkVKalVrtCore/VKalVrtBMag.h"
#include <cmath>
#include <array>
#include <iostream>

namespace Trk {

void vkPerigeeToP( const double *perig3, double *pp, double BMAG)
{
    double constB =BMAG  * vkalMagCnvCst;
    double phiv = perig3[1];
    double pt   = constB / std::abs(perig3[2]);
    pp[0] = pt * cos(phiv);
    pp[1] = pt * sin(phiv);
    pp[2] = pt / tan(perig3[0]);
}

std::array<double, 4> getFitParticleMom( const VKTrack * trk, const VKVertex *vk)
{
    std::array<double, 4> p{};
    double fieldPos[3];
    fieldPos[0]=vk->refIterV[0]+vk->fitV[0];
    fieldPos[1]=vk->refIterV[1]+vk->fitV[1];
    fieldPos[2]=vk->refIterV[2]+vk->fitV[2];
    double magConst =Trk::vkalMagFld::getMagFld(fieldPos,(vk->vk_fitterControl).get())  * Trk::vkalMagFld::getCnvCst();

    double cth = 1. / tan( trk->fitP[0]);
    double phi      = trk->fitP[1];
    double pt       = magConst/ std::abs( trk->fitP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}
std::array<double, 4> getFitParticleMom(const VKTrack * trk, double BMAG)
{
    std::array<double, 4> p{};
    double magConst =BMAG  * vkalMagCnvCst;

    double cth = 1. / tan( trk->fitP[0]);
    double phi      = trk->fitP[1];
    double pt       = magConst/ std::abs( trk->fitP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}

std::array<double, 4> getIniParticleMom( const VKTrack * trk, const VKVertex *vk)
{
    std::array<double, 4> p{};
    double magConst = Trk::vkalMagFld::getMagFld(vk->refIterV,(vk->vk_fitterControl).get())  * Trk::vkalMagFld::getCnvCst();

    double cth = 1. / tan( trk->iniP[0]);
    double phi      =      trk->iniP[1];
    double pt       = magConst/ std::abs( trk->iniP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}
std::array<double, 4> getIniParticleMom(const VKTrack * trk, double BMAG)
{
    std::array<double, 4> p{};
    double magConst =BMAG  * vkalMagCnvCst;

    double cth = 1. / tan( trk->iniP[0]);
    double phi      =      trk->iniP[1];
    double pt       = magConst/ std::abs( trk->iniP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}


std::array<double, 4> getCnstParticleMom( const VKTrack * trk, const VKVertex *vk )
{
    std::array<double, 4> p{};
    double cnstPos[3];
    cnstPos[0]=vk->refIterV[0]+vk->cnstV[0];
    cnstPos[1]=vk->refIterV[1]+vk->cnstV[1];
    cnstPos[2]=vk->refIterV[2]+vk->cnstV[2];
    double magConst = Trk::vkalMagFld::getMagFld(cnstPos,(vk->vk_fitterControl).get())  * Trk::vkalMagFld::getCnvCst();

    double cth = 1. / tan( trk->cnstP[0]);
    double phi      =      trk->cnstP[1];
    double pt       = magConst/ std::abs( trk->cnstP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}
std::array<double, 4> getCnstParticleMom(const VKTrack * trk, double BMAG )
{
    std::array<double, 4> p{};
    double magConst =BMAG  * vkalMagCnvCst;

    double cth = 1. / tan( trk->cnstP[0]);
    double phi      =      trk->cnstP[1];
    double pt       = magConst/ std::abs( trk->cnstP[2] );
    double m   = trk->getMass();
    p[0] = pt * cos(phi);
    p[1] = pt * sin(phi);
    p[2] = pt * cth;
    p[3] = sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2] + m*m );
    return p;
}

}
