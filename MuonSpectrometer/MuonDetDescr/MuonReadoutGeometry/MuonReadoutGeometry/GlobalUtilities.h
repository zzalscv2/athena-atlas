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
#include <string_view>
#include <optional>
#include <GeoPrimitives/GeoPrimitives.h>
#include <EventPrimitives/EventPrimitives.h>

namespace MuonGM {
    std::string buildString(int i, int ncha);
    int strtoint(std::string_view str, unsigned int istart, unsigned int length);

    /// Converts the AMDB phi index to the Identifier phi Index
    int stationPhiTGC(std::string_view stName, int fi, int zi_input);
    /// Converts the Identifier phi index to the AMDB phi index
    int amdbPhiTGC(std::string_view stName, int phiIndex, int eta_index);

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
        const double divisor = (1. - dirDots * dirDots);
        /// If the two directions are parallel to each other there's no way of intersection
        if (std::abs(divisor) < std::numeric_limits<double>::epsilon()) return std::nullopt;
        const AmgVector(N) AminusB = posA - posB;
        return (AminusB.dot(dirB) - AminusB.dot(dirA) * dirDots) / divisor;
    }
    /// Intersects a line parametrized as A + lambda * B with the (N-1) dimensional
    /// hyperplane that's given in the Jordan normal form <P, N> - C = 0 
    template <int N>
    std::optional<double> intersect(const AmgVector(N)& pos, const AmgVector(N)& dir, 
                                    const AmgVector(N)& planeNorm, const double offset) {
        ///  <P, N> - C = 0
        /// --> <A + lambda *B , N> - C = 0
        /// --> lambda = (C - <A,N> ) / <N, B>
        const double normDot = planeNorm.dot(dir); 
        if (std::abs(normDot) < std::numeric_limits<double>::epsilon()) return std::nullopt;
        return (offset - pos.dot(planeNorm)) / normDot;
    }
    /// Returns the position of the most left bit which is set to 1
    template <typename T> constexpr int maxBit(const T &number) {
        for (int bit = sizeof(number) * 8 - 1; bit >= 0; --bit) {
            if (number & (1 << bit)) return bit;
        }
        return -1;
    }
    /// Returns the position of the most right bit which is set to 1
    template <typename T> constexpr int minBit(const T &number) {
        for (unsigned int bit = 0; bit <= sizeof(number) * 8 - 1; ++bit) {
          if (number & (1 << bit)) return bit;
        }
        return -1;
    }

}  // namespace MuonGM

#endif  // MUONREADOUTGEOMETRY_GLOBALUTILITIES_H
