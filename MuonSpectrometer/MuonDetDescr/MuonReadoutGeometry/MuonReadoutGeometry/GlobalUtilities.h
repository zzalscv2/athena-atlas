/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 global functions and stuff ...
 -----------------------------------------
 ***************************************************************************/

#ifndef MUONREADOUTGEOMETRY_GLOBALUTILITIES_H
#define MUONREADOUTGEOMETRY_GLOBALUTILITIES_H

#include <string>
#include <optional>
#include <GeoPrimitives/GeoPrimitives.h>
#include <EventPrimitives/EventPrimitives.h>

namespace MuonGM {
    std::string buildString(int i, int ncha);
    int strtoint(std::string_view str, unsigned int istart, unsigned int length);
    int stationPhiTGC(std::string_view stName, int fi, int zi_input, std::string_view geometry_version);

    /// Calculates the closest approach of two lines. 
    ///    posA: offset point of line A
    ///    dirA: orientation of line A
    ///    posB: offset point of line B
    ///    dirB: orientation of line B
    /// Returns the length to be travelled along line B
    template <int N>
    std::optional<double> intersect(const AmgVector(N)& posA, const  AmgVector(N)& dirA, const AmgVector(N)& posB, const  AmgVector(N)& dirB) {
        //// Use the formula
        ///    A + lambda dirA  = B + mu dirB
        ///    (A-B) + lambda dirA = mu dirB
        ///    <A-B, dirB> + lambda <dirA,dirB> = mu
        ///     A + lambda dirA = B + (<A-B, dirB> + lambda <dirA,dirB>)dirB
        ///     <A-B,dirA> + lambda <dirA, dirA> = <A-B, dirB><dirA,dirB> + lamda<dirA,dirB><dirA,dirB>
        ///   -> lambda = (<A-B, dirA> - <A-B, dirB> * <dirA, dirB>) / (1- <dirA,dirB>^2)
        ///   --> mu   =  (<A-B, dirB> - <A-B, dirA> * <dirA, dirB>) / (1- <dirA,dirB>^2)
        const double dirDots = dirA.dot(dirB);
        const double divisor = (1. - std::pow(dirDots,2));
        /// If the two directions are paralellt to each other there's no way of intersection
        if (std::abs(divisor) < std::numeric_limits<double>::epsilon()) return std::nullopt;
        const AmgVector(N) AminusB = posA - posB;
        return (AminusB.dot(dirB) - AminusB.dot(dirA) * dirDots) / divisor;

    }
}  // namespace MuonGM

#endif  // MUONREADOUTGEOMETRY_GLOBALUTILITIES_H
