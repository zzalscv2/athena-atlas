/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file FPGATrackSimTrackPars.h
 * @author Riley Xu - riley.xu@cern.ch
 * @date March 27th, 2020
 * @brief Structs that store the 5 track parameters
 */

#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"

#include <stdexcept>

const double& FPGATrackSimTrackPars::operator[](unsigned i) const
{
    switch (i)
    {
    case 0: return phi;
    case 1: return qOverPt;
    case 2: return d0;
    case 3: return z0;
    case 4: return eta;
    default: throw std::out_of_range("FPGATrackSimTrackPars[] out of range");
    }
}


double& FPGATrackSimTrackPars::operator[](unsigned i)
{
    switch (i)
    {
    case 0: return phi;
    case 1: return qOverPt;
    case 2: return d0;
    case 3: return z0;
    case 4: return eta;
    default: throw std::out_of_range("FPGATrackSimTrackPars[] out of range");
    }
}


const int& FPGATrackSimTrackParsI::operator[](unsigned i) const
{
    switch (i)
    {
    case 0: return phi;
    case 1: return qOverPt;
    case 2: return d0;
    case 3: return z0;
    case 4: return eta;
    default: throw std::out_of_range("FPGATrackSimTrackPars[] out of range");
    }
}


int& FPGATrackSimTrackParsI::operator[](unsigned i)
{
    switch (i)
    {
    case 0: return phi;
    case 1: return qOverPt;
    case 2: return d0;
    case 3: return z0;
    case 4: return eta;
    default: throw std::out_of_range("FPGATrackSimTrackPars[] out of range");
    }
}


std::string FPGATrackSimTrackPars::parName(unsigned i)
{
    switch (i)
    {
    case 0: return "phi";
    case 1: return "qOverPt";
    case 2: return "d0";
    case 3: return "z0";
    case 4: return "eta";
    default: throw std::out_of_range("FPGATrackSimTrackPars::parName out of range");
    }
}

std::ostream& operator<<(std::ostream& os, const FPGATrackSimTrackPars& pars)
{
    os << "phi:" << pars.phi
        << " q/pt:" << pars.qOverPt
        << " d0:" << pars.d0
        << " z0:" << pars.z0
        << " eta:" << pars.eta;
    return os;
}

std::ostream& operator<<(std::ostream& os, const FPGATrackSimTrackParsI& pars)
{
    os << "phi:" << pars.phi
        << " q/pt:" << pars.qOverPt
        << " d0:" << pars.d0
        << " z0:" << pars.z0
        << " eta:" << pars.eta;
    return os;
}
